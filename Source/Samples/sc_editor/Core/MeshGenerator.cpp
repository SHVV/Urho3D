// SHW Spacecraft editor
//
// Class for generating meshes.

#include "MeshGenerator.h"

// Editor includes
#include "MeshGenerationFunction.h"

// Includes from Urho3D

using namespace Urho3D;

// Construct.
MeshGenerator::MeshGenerator(Context* context)
: Object(context)
{
}

/// Destructor
MeshGenerator::~MeshGenerator()
{
}

/// Returns default parameters of function
const Parameters& MeshGenerator::default_parameters(StringHash name)
{
  static Parameters empty;
  auto it = m_functions.Find(name);
  return it != m_functions.End() ? it->second_->default_parameters() : empty;
}

/// Generate mesh and return buffer for visualizing it.
MeshBuffer* MeshGenerator::generate_buffer(
  StringHash name, 
  const Parameters& parameters
)
{
  // TODO: make it in future async an generate in background thread
  // TODO: add memory consumption tracking and clearing unused cache

  // First - look into the cache
  auto key = MakePair(name, parameters);
  auto cache = m_generated_buffers.Find(key);
  if (cache != m_generated_buffers.End()) {
    return cache->second_;
  }

  // If not found - generate
  MeshGeometry* mesh = generate_mesh(name, parameters);
  // Failed to generate - return null
  if (!mesh) {
    return nullptr;
  }

  MeshBuffer* buffer = new MeshBuffer(context_);
  buffer->set_mesh_geometry(mesh);

  /// Write result to the cache
  m_generated_buffers[key] = buffer;

  return buffer;
}

/// Generate and return mesh only
MeshGeometry* MeshGenerator::generate_mesh(
  StringHash name, 
  const Parameters& parameters
)
{
  // TODO: add memory consumption tracking and clearing unused cache

  // First - look into the cache
  auto key = MakePair(name, parameters);
  auto cache = m_generated_meshes.Find(key);
  if (cache != m_generated_meshes.End()) {
    return cache->second_;
  }

  // If not found - generate
  auto it = m_functions.Find(name);
  if (it == m_functions.End()) {
    return nullptr;
  }
  MeshGeometry* mesh = it->second_->generate(parameters);
  if (!mesh) {
    return nullptr;
  }

  /// Write result to the cache
  m_generated_meshes[key] = mesh;

  return mesh;
}

/// Register mesh generation function
void MeshGenerator::add_function(MeshGenerationFunction* function)
{
  assert(function);
  function->generator(this);
  m_functions[StringHash(function->name())] = function;
}

/// Lathe
MeshGeometry* MeshGenerator::lathe(
  const Polyline2& profile, int sectors,
  TriangulationType triangulation,
  float start_angle, float end_angle
)
{
  // TODO: support closed polylines
  // TODO: smooth segments
  // TODO: control triangulation by segment
  // TODO: different sectors for each segment (or automatic tesselation)
  // TODO: mark generated items with numbers
  // TODO: base symmetry
  // TODO: UV - coordinates
  // TODO: Vertex colors
  // TODO: refactor to make it KISS
  // TODO: use section based generation for simplification
  // TODO: factor out normals calculation

  // Assert(end_angle > start_angle);
  MeshGeometry* mesh = new MeshGeometry(context_);

  int vertices = sectors + 1;
  bool full_circle = start_angle == end_angle;
  if (full_circle) {
    vertices = sectors;
    end_angle = start_angle + 2 * M_PI;
  }

  PODVector<int> last_segment;
  PODVector<int> cur_segment;

  auto& segments = profile.segments();
  float d_a = (end_angle - start_angle) / sectors;
  for (int i = 0; i < segments.Size(); ++i) {
    auto& seg = segments[i];

    if (seg.m_pos.y_ < M_LARGE_EPSILON) {
      MeshGeometry::Vertex v;
      // Case with degenerative points
      v.position.z_ = seg.m_pos.x_;
      v.material = seg.m_node_material_id;
      v.radius = seg.m_node_radius;
      if ((0 == i)) {
        // Start point
        v.normal.z_ = -1;
        cur_segment.Push(mesh->add(v));
      } else if (segments.Size() - 1 == i) {
        // End point
        v.normal.z_ = 1;
        int index = mesh->add(v);
        cur_segment.Push(index);
        
        // Triangle fan
        for (int j = 0; j < last_segment.Size(); ++j) {
          MeshGeometry::Edge e;
          e.material = seg.m_longitudal_material_id;
          e.secondary = false;
          e.vertexes[0] = last_segment[j];
          e.vertexes[1] = index;
          mesh->add(e);

          if (full_circle || j < last_segment.Size() - 1) {
            MeshGeometry::Polygon p;
            p.material = seg.m_plate_material_id;
            p.vertexes[0] = last_segment[j];
            if (j == last_segment.Size() - 1) {
              p.vertexes[1] = last_segment[0];
            } else {
              p.vertexes[1] = last_segment[j + 1];
            }
            p.vertexes[2] = index;
            mesh->add(p);
          }
        }
      }
      // TODO: other degenrative points are not handled
    } else {
      float angle = start_angle;
      if (triangulation == ttTRIANGLE && (i & 1)) {
        angle += d_a / 2;
      }

      // Segment normal
      Vector2 seg_norm;
      if (0 == i) {
        seg_norm = segments[i + 1].m_pos - seg.m_pos;
      } else if (i == segments.Size() - 1 || !seg.m_smooth_vertex) {
        seg_norm = seg.m_pos - segments[i - 1].m_pos;
      } else {
        seg_norm = 
          (segments[i + 1].m_pos - seg.m_pos).Normalized() +
          (seg.m_pos - segments[i - 1].m_pos).Normalized();
      }
      seg_norm.Normalize();

      // Add vertices
      for (int j = 0; j < vertices; ++j) {
        float ca = cos(angle);
        float sa = sin(angle);

        MeshGeometry::Vertex v;
        v.position = Vector3(ca * seg.m_pos.y_, sa * seg.m_pos.y_, seg.m_pos.x_);
        v.normal = Vector3(ca * seg_norm.x_, sa * seg_norm.x_, -seg_norm.y_);
        v.normal.Normalize();
        v.material = seg.m_node_material_id;
        v.radius = seg.m_node_radius;
        cur_segment.Push(mesh->add(v));
  
        // Increment angle
        angle += d_a;
      }

      // Lateral edges always the same
      for (int j = 0; j < sectors; ++j) {
        MeshGeometry::Edge e;
        e.material = seg.m_lateral_material_id;
        e.secondary = false;
        e.vertexes[0] = cur_segment[j];
        if (full_circle && (j == (sectors - 1))) {
          e.vertexes[1] = cur_segment[0];
        } else {
          e.vertexes[1] = cur_segment[j + 1];
        }
        mesh->add(e);
      }

      // Connect last segment with new one
      if (last_segment.Size()) {
        // Lateral edges
        for (int j = 0; j < vertices; ++j) {
          MeshGeometry::Edge e;
          e.material = seg.m_longitudal_material_id;
          e.secondary = false;
          // Major edge (always the same)
          if (last_segment.Size() < vertices) {
            e.vertexes[0] = last_segment[0];
          } else {
            e.vertexes[0] = last_segment[j];
          }
          e.vertexes[1] = cur_segment[j];
          mesh->add(e);
        }

        // Diagonal edges exist only if previous segment is not degenerative
        if (last_segment.Size() == vertices) {
          for (int j = 0; j < sectors; ++j) {
            MeshGeometry::Edge e;
            e.material = seg.m_longitudal_material_id;
            e.secondary = triangulation != ttTRIANGLE;
            int next_index = (full_circle && (j == sectors - 1)) ? 0 : (j + 1);
            bool forward = true;
            switch (triangulation) {
              case ttTRIANGLE:
              case ttSEGMENT: {
                forward = !(i & 1);
                break;
              }
              case ttSECTOR: {
                forward = (j & 1);
                break;
              }
              case ttDIAMOND: {
                forward = ((j + i) & 1);
                break;
              }
            }
            if (forward) {
              e.vertexes[0] = last_segment[j];
              e.vertexes[1] = cur_segment[next_index];
            } else {
              e.vertexes[0] = last_segment[next_index];
              e.vertexes[1] = cur_segment[j];
            }
            int e_ind = mesh->add(e);

            MeshGeometry::Polygon p;
            p.material = seg.m_plate_material_id;
            if (triangulation != ttTRIANGLE) {
              if (forward) {
                p.vertexes[3] = cur_segment[j];
                p.vertexes[2] = cur_segment[next_index];
                p.vertexes[1] = last_segment[next_index];
                p.vertexes[0] = last_segment[j];
              } else {
                p.vertexes[3] = last_segment[j];
                p.vertexes[2] = cur_segment[j];
                p.vertexes[1] = cur_segment[next_index];
                p.vertexes[0] = last_segment[next_index];
              }
              mesh->add(p);
            } else {
              if (i & 1) {
                p.vertexes[2] = last_segment[j];
                p.vertexes[1] = cur_segment[j];
                p.vertexes[0] = last_segment[next_index];
                mesh->add(p);
                p.vertexes[2] = last_segment[next_index];
                p.vertexes[1] = cur_segment[j];
                p.vertexes[0] = cur_segment[next_index];
                mesh->add(p);
              } else {
                p.vertexes[2] = last_segment[j];
                p.vertexes[1] = cur_segment[j];
                p.vertexes[0] = cur_segment[next_index];
                mesh->add(p);
                p.vertexes[2] = last_segment[j];
                p.vertexes[1] = cur_segment[next_index];
                p.vertexes[0] = last_segment[next_index];
                mesh->add(p);
              }
            }
          }
        } else {
          // Add triangle fan
          for (int j = 0; j < cur_segment.Size(); ++j) {
            if (full_circle || j < cur_segment.Size() - 1) {
              MeshGeometry::Polygon p;
              p.material = seg.m_plate_material_id;
              // TODO: check CW/CCW
              p.vertexes[0] = cur_segment[j];
              p.vertexes[1] = last_segment[0];
              if (j == cur_segment.Size() - 1) {
                p.vertexes[2] = cur_segment[0];
              } else {
                p.vertexes[2] = cur_segment[j + 1];
              }
              mesh->add(p);
            }
          }
        }
      }
      
      // Duplicate vertecies for non-smooth segments
      if (!seg.m_smooth_vertex && i > 0 && i < segments.Size() - 1) {
        cur_segment.Clear();
        // Segment normal
        Vector2 seg_norm = segments[i + 1].m_pos - seg.m_pos;
        seg_norm.Normalize();

        float angle = start_angle;
        if (triangulation == ttTRIANGLE && (i & 1)) {
          angle += d_a / 2;
        }

        // Add vertecies
        for (int j = 0; j < vertices; ++j) {
          float ca = cos(angle);
          float sa = sin(angle);

          MeshGeometry::Vertex v;
          v.position = Vector3(ca * seg.m_pos.y_, sa * seg.m_pos.y_, seg.m_pos.x_);
          v.normal = Vector3(ca * seg_norm.x_, sa * seg_norm.x_, -seg_norm.y_);
          v.normal.Normalize();
          v.material = seg.m_node_material_id;
          v.radius = seg.m_node_radius;
          cur_segment.Push(mesh->add(v));

          // Increment angle
          angle += d_a;
        }
      }
    }
    last_segment = cur_segment;
    cur_segment.Clear();
  }

  return mesh;
}


void piramid(
  int n, float scale, const Vector3& pos,
  const Vector<Vector3>& base, MeshGeometry* mesh
)
{
  if (n) {
    // Recursion
    float new_scale = scale * 0.5;
    int new_n = n - 1;
    for (int i = 0; i < 4; ++i) {
      Vector3 new_pos = pos + base[i] * new_scale;
      piramid(new_n, new_scale, new_pos, base, mesh);
    }
  } else {
    // Terminal
    int indexies[4];
    // Vertecies
    for (int i = 0; i < 4; ++i) {
      indexies[i] = mesh->add(base[i] * scale + pos, 0.15 * scale);
    }
    // Edges
    for (int i = 0; i < 4; ++i) {
      for (int j = i + 1; j < 4; ++j) {
        mesh->add(indexies[i], indexies[j]);
      }
    }
    // Polygons
    mesh->add(indexies[0], indexies[1], indexies[2]);
    mesh->add(indexies[0], indexies[3], indexies[1]);
    mesh->add(indexies[1], indexies[3], indexies[2]);
    mesh->add(indexies[0], indexies[2], indexies[3]);
  }
}

void piramid(int level, float size, MeshGeometry* mesh)
{
  // Base
  Vector<Vector3> base;
  base.Push(Vector3(1, 1, -1));
  base.Push(Vector3(-1, -1, -1));
  base.Push(Vector3(-1, 1, 1));
  base.Push(Vector3(1, -1, 1));

  piramid(level, size, Vector3(0, 0, 0), base, mesh);
}
