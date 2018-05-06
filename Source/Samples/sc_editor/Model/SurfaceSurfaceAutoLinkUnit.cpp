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
#include "Urho3D\Core\CoreEvents.h"

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

/// Prepare everything before rendering
void SurfaceSurfaceAutoLinkUnit::on_update_views(
  StringHash eventType,
  VariantMap& eventData
)
{
  Node* node = GetNode();
  // If dirty - update
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
      const MeshGeometry* attached_geometry =
        attached_surface->dynamic_model()->mesh_geometry();
      const MeshGeometry* parent_geometry =
        parent_surface->dynamic_model()->mesh_geometry();

      auto& attached_vertex_indexes = 
        attached_geometry->vertices_by_flags(mgfATTACHABLE);
      auto& parent_vertex_indexes =
        parent_geometry->vertices_by_flags(mgfATTACHABLE);
      
      PODVector<Vector3> attached_positions;
      attached_positions.Reserve(attached_vertex_indexes.Size());
      for (int i = 0; i < attached_vertex_indexes.Size(); ++i) {
        Vector3 position = 
          attached_geometry->vertices()[attached_vertex_indexes[i]].position;
        attached_positions.Push(attached_cs * position);
      }
      PODVector<Vector3> parent_positions;
      parent_positions.Reserve(parent_vertex_indexes.Size());
      for (int i = 0; i < parent_vertex_indexes.Size(); ++i) {
        Vector3 position =
          parent_geometry->vertices()[parent_vertex_indexes[i]].position;
        parent_positions.Push(parent_cs * position);
      }

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

      // List of potential beams (pairs of vertex)
      PODVector<Pair<int, int>> beams;
      beams.Reserve(attached_positions.Size() * parent_positions.Size());
      for (int i = 0; i < attached_positions.Size(); ++i) {
        for (int j = 0; j < parent_positions.Size(); ++j) {
          Vector3& pos1 = attached_positions[i];
          Vector3& pos2 = parent_positions[j];
          // Filter by length
          float distance = (pos1 - pos2).Length();
          if (distance > shortest_beam * 2) {
            continue;
          }
          // Filter by collision (optimize collision detection)
          //if (attached_geometry->test_collision(to_attached_cs * pos1, to_attached_cs * pos2) >= 0){
          //  continue;
          //}
          //if (parent_geometry->test_collision(to_parent_cs * pos1, to_parent_cs * pos2) >= 0) {
          //  continue;
          //}
          beams.Push(MakePair<int, int>(i, j));
        }
      }

      // TODO: 
      // - optimize by structure rigidity:
      //   - test response for different deformations
      //   - remove useless trusses until rigidity preserved
      //   - check beams self-intersection
      // TODO: detect rings case

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
