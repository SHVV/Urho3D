// SHW Spacecraft editor
//
// Unit for automatic Surface-Surface truss connection.

#include "SurfaceSurfaceAutoLinkUnit.h"

#include "../Core/MeshGeometry.h"
#include "../Core/MeshBuffer.h"
#include "../Core/MeshGenerator.h"
#include "../Core/MeshGenerationFunction.h"

#include "DynamicModel.h"

#include <Urho3D\Core\Context.h>
#include <Urho3D\Core\CoreEvents.h>
#include <Urho3D\Container\Sort.h>

using namespace Urho3D;

extern const char* EDITOR_CATEGORY;

/// Register object attributes.
void SurfaceSurfaceAutoLinkUnit::RegisterObject(Context* context)
{
  context->RegisterFactory<SurfaceSurfaceAutoLinkUnit>(EDITOR_CATEGORY);

  URHO3D_COPY_BASE_ATTRIBUTES(UnitModel);
}

/// Construct.
SurfaceSurfaceAutoLinkUnit::SurfaceSurfaceAutoLinkUnit(Context* context)
  : UnitModel(context)
{
}

/// Destructor
SurfaceSurfaceAutoLinkUnit::~SurfaceSurfaceAutoLinkUnit()
{
}

/// Set nodes to connect
void SurfaceSurfaceAutoLinkUnit::set_nodes(Node* attached_node, Node* parent_node)
{
  assert(attached_node);
  assert(parent_node);

  // If something new
  if ((m_attached_node != attached_node) || (m_parent_node != parent_node)) {
    // Unsubscribe from all events
    UnsubscribeFromAllEvents();
    if (!m_attached_node.Expired()) {
      m_attached_node->RemoveListener(this);
    }
    if (!m_parent_node.Expired()) {
      m_parent_node->RemoveListener(this);
    }

    // Set new
    m_attached_node = attached_node;
    m_parent_node = parent_node;

    // Subscribe on mode updates
    m_attached_node->AddListener(this);
    m_parent_node->AddListener(this);

    // Subscribe for attachable surface changes
    BaseAttachableSurface* attached_surface = get_attached_surface();
    if (attached_surface) {
      SubscribeToEvent(
        attached_surface,
        E_ATTACHABLE_SURFACE_CHANGED,
        URHO3D_HANDLER(SurfaceSurfaceAutoLinkUnit, on_surface_changed)
      );
    }
    BaseAttachableSurface* parent_surface = get_parent_surface();
    if (parent_surface) {
      SubscribeToEvent(
        parent_surface,
        E_ATTACHABLE_SURFACE_CHANGED,
        URHO3D_HANDLER(SurfaceSurfaceAutoLinkUnit, on_surface_changed)
      );
    }

    // Subscribe to post update for lazy recalculation
    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(SurfaceSurfaceAutoLinkUnit, on_update_views));
  }
}

// Attachable surfaces.
BaseAttachableSurface* SurfaceSurfaceAutoLinkUnit::get_attached_surface()
{
  if (!m_attached_node.Expired()) {
    return m_attached_node->GetDerivedComponent<BaseAttachableSurface>();
  }
}

// Attachable surfaces.
BaseAttachableSurface* SurfaceSurfaceAutoLinkUnit::get_parent_surface()
{
  if (!m_parent_node.Expired()) {
    return m_parent_node->GetDerivedComponent<BaseAttachableSurface>();
  }
}

/// On surface changed
void SurfaceSurfaceAutoLinkUnit::on_surface_changed(
  StringHash eventType,
  VariantMap& eventData
)
{
  m_truss_dirty = true;
}

/// Handle scene node transform dirtied.
void SurfaceSurfaceAutoLinkUnit::OnMarkedDirty(Node* node)
{
  m_truss_dirty = true;
}

/// Create or update all necessary components - override for derived classes
void SurfaceSurfaceAutoLinkUnit::update_guts_int()
{
  // Create dynamic model for rendering truss
  get_component<DynamicModel>();
}

/// Calculate usefullness of beam in direction
float beam_usefullness(
  const Vector3& pos1,
  const Vector3& pos2,
  const Vector3& center,
  const Vector3& direction
)
{
  // Calculate minimum distance in direction
  float dist1 = direction.AbsDotProduct(pos1 - center);
  float dist2 = direction.AbsDotProduct(pos2 - center);
  float dist = Min(dist1, dist2);

  // Calculate angle in direction
  Vector3 beam = pos2 - pos1;
  float angularity = 
    M_PI / 4 - abs(acos(direction.AbsDotProduct(beam.Normalized())) - M_PI / 4);
  float usefullness = angularity * dist * dist;
  if (isnan(usefullness)) {
    usefullness = 0;
  }

  return usefullness;
}

bool usefullness_compare(
  const Pair<float, int>& first,
  const Pair<float, int>& second
)
{
  return first.first_ > second.first_;
}

/// Optimize truss by using several heuristics
void optimize_truss(
  const PODVector<Vector3>& positions1, // attached
  const PODVector<Vector3>& positions2,
  const Matrix3x4& trans1,
  const Matrix3x4& trans2,
  const PODVector<Pair<int, int>>& in_truss,
  PODVector<Pair<int, int>>& out_truss
  )
{
  // Calculate centre of ground structure
  // TODO: ignore boundary effects.
  Vector3 ground_structure_centre = Vector3::ZERO;
  for (int i = 0; i < in_truss.Size(); ++i) {
    ground_structure_centre += positions1[in_truss[i].first_];
    ground_structure_centre += positions2[in_truss[i].second_];
  }
  ground_structure_centre /= in_truss.Size() * 2;

  // Calculate primary directions
  Vector3 normal = (trans1.Translation() - ground_structure_centre).Normalized();
  Vector3 binormal = normal.CrossProduct(trans2 * Vector4(0, 0, 1, 0));
  if (binormal.LengthSquared() < 0.01) {
    binormal = trans2 * Vector4(0, 1, 0, 0);
  }
  binormal.Normalize();
  Vector3 tangent = normal.CrossProduct(binormal).Normalized();
  binormal = normal.CrossProduct(tangent);

  // Calculate absolute usefullness of each beam in primary directions
  PODVector<float> usefullness_tangent;
  usefullness_tangent.Reserve(in_truss.Size());
  PODVector<float> usefullness_binormal;
  usefullness_binormal.Reserve(in_truss.Size());
  float best_in_tangent = 0;
  float best_in_binormal = 0;
  for (int i = 0; i < in_truss.Size(); ++i) {
    const Vector3& pos1 = positions1[in_truss[i].first_];
    const Vector3& pos2 = positions2[in_truss[i].second_];

    float in_tangent = beam_usefullness(pos1, pos2, ground_structure_centre, tangent);
    float in_binormal = beam_usefullness(pos1, pos2, ground_structure_centre, binormal);

    if (in_tangent > best_in_tangent) {
      best_in_tangent = in_tangent;
    }
    if (in_binormal > best_in_binormal) {
      best_in_binormal = in_binormal;
    }

    usefullness_tangent.Push(in_tangent);
    usefullness_binormal.Push(in_binormal);
  }

  // Normalize usefullness
  PODVector<Pair<float, int>> usefullness;
  usefullness.Reserve(in_truss.Size());
  for (int i = 0; i < in_truss.Size(); ++i) {
    const Vector3& pos1 = positions1[in_truss[i].first_];
    const Vector3& pos2 = positions2[in_truss[i].second_];

    float beam_len = (pos1 - pos2).Length();
    float current_usefullness =
      beam_len > 0.01 ?
      ((usefullness_binormal[i] / best_in_binormal +
      usefullness_tangent[i] / best_in_tangent) /
      (beam_len * beam_len * beam_len)) : 0;
    if (isnan(current_usefullness)) {
      current_usefullness = 0;
    }
    usefullness.Push(Pair<float, int>(current_usefullness, i));
  }

  // Sort beams
  Sort(begin(usefullness), end(usefullness), &usefullness_compare);

  // Choose n best beams
  // TODO: check rigidity and case, that new beams increase it
  float last_usefullness = 0;
  for (int i = 0; i < in_truss.Size(); ++i) {
    if (i < 12 || (last_usefullness - usefullness[i].first_) < 0.0000001) {
      // TODO: test collisions with existing beams
      last_usefullness = usefullness[i].first_;
      out_truss.Push(in_truss[usefullness[i].second_]);
    }
  }
}

void get_attachable_vertices(
  BaseAttachableSurface* surface,
  const Matrix3x4& cs,
  PODVector<Vector3>& positions,
  PODVector<Vector3>& normals
)
{
  const MeshGeometry* geometry =
    surface->dynamic_model()->mesh_geometry();

  auto& vertex_indexes =
    geometry->vertices_by_flags(mgfATTACHABLE);

  positions.Reserve(vertex_indexes.Size());
  normals.Reserve(vertex_indexes.Size());
  for (int i = 0; i < vertex_indexes.Size(); ++i) {
    const MeshGeometry::Vertex& vertex =
      geometry->vertices()[vertex_indexes[i]];
    Vector3 pos = cs * vertex.position;
    Vector3 norm = cs * Vector4(vertex.normal, 0.0);
    bool found = false;
    for (int j = 0; j < positions.Size() && !found; ++j){
      if (positions[j] == pos) {
        found = true;
        normals[j] += norm;
        normals[j] /= 2;
      }
    }
    if (!found) {
      positions.Push(pos);
      normals.Push(norm);
    }
  }
}

/// Prepare everything before rendering
void SurfaceSurfaceAutoLinkUnit::on_update_views(
  StringHash eventType,
  VariantMap& eventData
)
{
  Node* node = GetNode();
  // If dirty - update
  // TODO: filter update by checking changes in relative position between parent-child
  if (m_truss_dirty && node)  {
    BaseAttachableSurface* attached_surface = get_attached_surface();
    BaseAttachableSurface* parent_surface = get_parent_surface();
    DynamicModel* dynamic_model = node->GetComponent<DynamicModel>();
    if (attached_surface && parent_surface) {
      // Coordinate systems
      Matrix3x4 to_our_cs = node->GetWorldTransform().Inverse();
      Matrix3x4 attached_cs = m_attached_node->GetWorldTransform();
      Matrix3x4 to_attached_cs = attached_cs.Inverse();
      Matrix3x4 parent_cs = m_parent_node->GetWorldTransform();
      Matrix3x4 to_parent_cs = parent_cs.Inverse();

      // List of vertex positions of each surface
      PODVector<Vector3> attached_positions;
      PODVector<Vector3> attached_normals;
      get_attachable_vertices(
        attached_surface, attached_cs, attached_positions, attached_normals
      );

      PODVector<Vector3> parent_positions;
      PODVector<Vector3> parent_normals;
      get_attachable_vertices(
        parent_surface, parent_cs, parent_positions, parent_normals
      );

      // Find shortest beam
      float shortest_beam = M_INFINITY;
      for (int i = 0; i < attached_positions.Size(); ++i) {
        for (int j = 0; j < parent_positions.Size(); ++j) {
          float distance = (attached_positions[i] - parent_positions[j]).Length();
          if (distance < shortest_beam) {
            shortest_beam = distance;
          }
        }
      }
      float average_edge = 
        attached_surface->average_attachable_edge() + 
        parent_surface->average_attachable_edge();
      average_edge /= 2;
      if (shortest_beam < average_edge) {
        shortest_beam = average_edge;
      }

      // List of potential beams (pairs of vertex)
      PODVector<Pair<int, int>> ground_beams;
      ground_beams.Reserve(attached_positions.Size() * parent_positions.Size());
      for (int i = 0; i < attached_positions.Size(); ++i) {
        for (int j = 0; j < parent_positions.Size(); ++j) {
          Vector3& pos1 = attached_positions[i];
          Vector3& pos2 = parent_positions[j];
          // Filter by length
          // TODO: increase length, until rigidity will start to preserve
          float distance = (pos1 - pos2).Length();
          if (distance > shortest_beam * 3) {
            continue;
          }
          // Filter by normal
          Vector3& norm1 = attached_normals[i];
          Vector3& norm2 = parent_normals[j];
          Vector3 dir = (pos2 - pos1).Normalized();
          const float min_cos = 0.1f;
          if (norm1.DotProduct(dir) < min_cos || (-norm2.DotProduct(dir)) < min_cos) {
            continue;
          }

          // Filter by collision (optimize collision detection)
          //if (attached_geometry->test_collision(to_attached_cs * pos1, to_attached_cs * pos2) >= 0){
          //  continue;
          //}
          //if (parent_geometry->test_collision(to_parent_cs * pos1, to_parent_cs * pos2) >= 0) {
          //  continue;
          //}
          ground_beams.Push(MakePair<int, int>(i, j));
        }
      }

      // Euristics optimization
      PODVector<Pair<int, int>> beams;
      optimize_truss(
        attached_positions, parent_positions, 
        attached_cs,        parent_cs, 
        ground_beams,       beams
      );

      // TODO: 
      // - optimize by structure rigidity:
      //   - test response for different deformations
      //   - remove useless trusses until rigidity preserved
      //   - check beams self-intersection
      // TODO: detect rings case?

      // TODO: recalculate node radius
      float radius = shortest_beam / 25;

      // Create geometry model and dynamic buffer or update existing
      HashMap<int, int> attached_index_map;
      HashMap<int, int> parent_index_map;
      MeshGeometry* mesh = new MeshGeometry(context_);
      MeshGeometry::Edge edge;
      edge.material = 0;
      for (int i = 0; i < beams.Size(); ++i) {
        int attached_original_index = beams[i].first_;
        int parent_original_index = beams[i].second_;

        auto attached_index_it = attached_index_map.Find(attached_original_index);
        int attached_new_index;
        if (attached_index_it == attached_index_map.End()) {
          attached_new_index = mesh->add(
            to_our_cs * attached_positions[attached_original_index], 
            radius
          );
          attached_index_map[attached_original_index] = attached_new_index;
        } else {
          attached_new_index = attached_index_it->second_;
        }

        auto parent_index_it = parent_index_map.Find(parent_original_index);
        int parent_new_index;
        if (parent_index_it == parent_index_map.End()) {
          parent_new_index = mesh->add(
            to_our_cs * parent_positions[parent_original_index],
            radius
          );
          parent_index_map[parent_original_index] = parent_new_index;
        } else {
          parent_new_index = parent_index_it->second_;
        }
        
        edge.vertexes[0] = attached_new_index;
        edge.vertexes[1] = parent_new_index;
        mesh->add(edge);
      }
      MeshBuffer* mesh_buffer = new MeshBuffer(context_);
      mesh_buffer->set_mesh_geometry(mesh);
      dynamic_model->mesh_buffer(mesh_buffer);

      // TODO: use global object for links rendering

      dynamic_model->SetEnabled(true);
    } else {
      dynamic_model->SetEnabled(false);
    }
    // TODO: actually create structure
    m_truss_dirty = false;
  }
}
