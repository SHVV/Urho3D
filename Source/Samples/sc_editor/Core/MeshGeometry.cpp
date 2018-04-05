// SHW Spacecraft editor
//
// Class for dynamically work with geometric meshes

#include "MeshGeometry.h"

// Includes from Urho3D
#include "Urho3D/Core/Profiler.h"
#include "Urho3D/Math/Ray.h"
#include "Urho3D/Math/Sphere.h"
#include "Urho3D/Math/Plane.h"

/// Default constructor.
MeshGeometry::MeshGeometry(Context* context)
  : Object(context)
{
}

/// Destructor
MeshGeometry::~MeshGeometry()
{

}

/// Vertexes list
const PODVector<MeshGeometry::Vertex>& MeshGeometry::vertices() const
{
  return m_vertices;
}

/// Edges list
const PODVector<MeshGeometry::Edge>& MeshGeometry::edges() const
{
  return m_edges;
}

/// Polygons list
const PODVector<MeshGeometry::Polygon>& MeshGeometry::polygons() const
{
  return m_polygons;
}

// Vertices
/// Add
int MeshGeometry::add(const MeshGeometry::Vertex& vertex)
{
  int index;
  if (m_free_vertices.Size()) {
    index = m_free_vertices.Back();
    m_vertices[index] = vertex;
    m_free_vertices.Pop();
  } else {
    m_vertices.Push(vertex);
    index = m_vertices.Size() - 1;
  }

  send_update(VERTEX_ADDED, index);
  return index;
}

int MeshGeometry::add(const Vector3& pos){
  Vertex vertex;
  vertex.position = pos;

  return add(vertex);
}

int MeshGeometry::add(const Vector3& pos, float radius)
{
  Vertex vertex;
  vertex.position = pos;
  vertex.radius = radius;

  return add(vertex);
}

int MeshGeometry::add(const Vector3& pos, const Vector3& normal)
{
  Vertex vertex;
  vertex.position = pos;
  vertex.normal = normal;

  return add(vertex);
}

/// Remove
bool MeshGeometry::remove_vertex(int index)
{
  Vertex* vertex = get_vertex(index);
  if (vertex) {
    vertex->deleted = true;
    m_free_vertices.Push(index);
    send_update(VERTEX_REMOVED, index);
  }
  return !!vertex;
}

/// Set position
bool MeshGeometry::set_position(int index, Vector3 pos)
{
  Vertex* vertex = get_vertex(index);
  if (vertex) {
    vertex->position = pos;
    send_update(VERTEX_CHANGED, index);
  }
  return !!vertex;
}

/// Set normal
bool MeshGeometry::set_normal(int index, Vector3 norm)
{
  Vertex* vertex = get_vertex(index);
  if (vertex) {
    vertex->normal = norm;
    send_update(VERTEX_CHANGED, index);
  }
  return !!vertex;
}

/// Set radius
bool MeshGeometry::set_radius(int index, float radius)
{
  Vertex* vertex = get_vertex(index);
  if (vertex) {
    vertex->radius = radius;
    send_update(VERTEX_CHANGED, index);
  }
  return !!vertex;
}

/// Set scale
bool MeshGeometry::set_scale(int index, float scale)
{
  Vertex* vertex = get_vertex(index);
  if (vertex) {
    vertex->scale = scale;
    send_update(VERTEX_CHANGED, index);
  }
  return !!vertex;
}

/// Set material
bool MeshGeometry::set_vertex_material(int index, int material)
{
  Vertex* vertex = get_vertex(index);
  if (vertex) {
    vertex->material = material;
    send_update(VERTEX_PAINTED, index);
  }
  return !!vertex;
}

/// Set flags
bool MeshGeometry::set_vertex_flags(int index, unsigned int flags)
{
  Vertex* vertex = get_vertex(index);
  if (vertex) {
    vertex->flags = flags;
    send_update(VERTEX_PAINTED, index);
  }
  return !!vertex;
}

// Edges
/// Add
int MeshGeometry::add(const Edge& edge)
{
  int index;
  if (m_free_edges.Size()) {
    index = m_free_edges.Back();
    m_edges[index] = edge;
    m_free_edges.Pop();
  } else {
    m_edges.Push(edge);
    index = m_edges.Size() - 1;
  }

  send_update(EDGE_ADDED, index);

  return index;
}

int MeshGeometry::add(int v1, int v2)
{
  Edge edge;
  // TODO: check vertices
  edge.vertexes[0] = v1;
  edge.vertexes[1] = v2;
  edge.secondary = false;

  return add(edge);
}

/// Remove
bool MeshGeometry::remove_edge(int index)
{
  Edge* edge = get_edge(index);
  if (edge) {
    edge->deleted = true;
    m_free_edges.Push(index);
    send_update(EDGE_REMOVED, index);
  }
  return !!edge;
}

/// Set material
bool MeshGeometry::set_edge_material(int index, int material)
{
  Edge* edge = get_edge(index);
  if (edge) {
    edge->material = material;
    send_update(EDGE_PAINTED, index);
  }
  return !!edge;
}

/// Set flags
bool MeshGeometry::set_edge_flags(int index, unsigned int flags)
{
  Edge* edge = get_edge(index);
  if (edge) {
    edge->flags = flags;
    send_update(EDGE_PAINTED, index);
  }
  return !!edge;
}

// Polygons
/// Add
int MeshGeometry::add(const Polygon& polygon)
{
  int index;
  if (m_free_polygons.Size()) {
    index = m_free_polygons.Back();
    m_polygons[index] = polygon;
    m_free_polygons.Pop();
  } else {
    m_polygons.Push(polygon);
    index = m_polygons.Size() - 1;
  }

  send_update(POLYGON_ADDED, index);
  return index;
}

int MeshGeometry::add(int v1, int v2, int v3)
{
  Polygon polygon;
  // TODO: check vertices
  polygon.vertexes[0] = v1;
  polygon.vertexes[1] = v2;
  polygon.vertexes[2] = v3;
  polygon.vertexes[3] = -1;

  return add(polygon);
}

int MeshGeometry::add(int v1, int v2, int v3, int v4)
{
  Polygon polygon;
  // TODO: check vertices
  polygon.vertexes[0] = v1;
  polygon.vertexes[1] = v2;
  polygon.vertexes[2] = v3;
  polygon.vertexes[3] = v4;

  return add(polygon);
}

/// Remove
bool MeshGeometry::remove_polygon(int index)
{
  Polygon* polygon = get_polygon(index);
  if (polygon) {
    polygon->deleted = true;
    m_free_polygons.Push(index);
    send_update(POLYGON_REMOVED, index);
  }
  return !!polygon;
}

/// Set material
bool MeshGeometry::set_polygon_material(int index, int material)
{
  Polygon* polygon = get_polygon(index);
  if (polygon) {
    polygon->material = material;
    send_update(POLYGON_PAINTED, index);
  }
  return !!polygon;
}

/// Set flags
bool MeshGeometry::set_polygon_flags(int index, unsigned int flags)
{
  Polygon* polygon = get_polygon(index);
  if (polygon) {
    polygon->flags = flags;
    send_update(POLYGON_PAINTED, index);
  }
  return !!polygon;
}

/// Pack everything, if there are too much empty objects
void MeshGeometry::compact()
{
  // TODO: implement
}

MeshGeometry::Vertex* MeshGeometry::get_vertex(int index)
{
#ifdef _DEBUG
  if (index < 0 || index >= m_vertices.Size()) {
    assert(0);
    return nullptr;
  }
#endif // _DEBUG
  if (m_vertices[index].deleted) {
    return nullptr;
  }
  return &m_vertices[index];
}

MeshGeometry::Edge* MeshGeometry::get_edge(int index)
{
#ifdef _DEBUG
  if (index < 0 || index >= m_edges.Size()) {
    assert(0);
    return nullptr;
  }
#endif // _DEBUG
  if (m_edges[index].deleted) {
    return nullptr;
  }
  return &m_edges[index];
}

MeshGeometry::Polygon* MeshGeometry::get_polygon(int index)
{
#ifdef _DEBUG
  if (index < 0 || index >= m_polygons.Size()) {
    assert(0);
    return nullptr;
  }
#endif // _DEBUG
  if (m_polygons[index].deleted) {
    return nullptr;
  }
  return &m_polygons[index];
}

/// Transform mesh with matrix
void MeshGeometry::transform(const Matrix3x4& tr)
{
  for (unsigned int i = 0; i < m_vertices.Size(); ++i) {
    Vertex& vertex = m_vertices[i];
    if (!vertex.deleted) {
      vertex.position = tr * vertex.position;
      vertex.normal = tr * Vector4(vertex.normal, 0.0f);
    }
  }
  send_update(GLOBAL_UPDATE);
}

/// Returns indexies of all verteces by flags
PODVector<int> MeshGeometry::vertices_by_flags(unsigned int flags) const
{
  return primitives_by_flags(m_vertices, flags);
}

/// Returns indexies of all edges by flags
PODVector<int> MeshGeometry::edges_by_flags(unsigned int flags) const
{
  return primitives_by_flags(m_edges, flags);
}

/// Returns indexies of all polygons by flags
PODVector<int> MeshGeometry::polygons_by_flags(unsigned int flags) const
{
  return primitives_by_flags(m_polygons, flags);
}

/// Raycast mesh. Returns index of sub object and its type
int MeshGeometry::raycast(
  const Ray& ray,
  SubObjectType& res_type,
  int types,
  unsigned int flags
) const
{
  float t = M_INFINITY;
  
  return raycast(ray, res_type, types, flags, t);
}

/// The same with returning t result
int MeshGeometry::raycast(
  const Ray& ray,
  SubObjectType& res_type,
  int types,
  unsigned int flags,
  float& t
) const
{
  URHO3D_PROFILE(RayCast);

  int index = -1;

  if (types & (int)SubObjectType::VERTEX) {
    if (ray_cast_vertices(ray, flags, t, index)) {
      res_type = SubObjectType::VERTEX;
    }
  }

  if (types & (int)SubObjectType::EDGE) {
    if (ray_cast_edges(ray, flags, t, index)) {
      res_type = SubObjectType::EDGE;
    }
  }

  if (types & (int)SubObjectType::POLYGON) {
    if (ray_cast_polygons(ray, flags, t, index)) {
      res_type = SubObjectType::POLYGON;
    }
  }

  return index;
}

/// Distance from point P to edge p1-p2
float point_edge_distance(const Vector3& p, const Vector3& p1, const Vector3& p2)
{
  Vector3 edge_vector = p2 - p1;
  Vector3 d1 = p - p1;
  Vector3 d2 = p - p2;
  float dd1 = d1.DotProduct(edge_vector);
  float dd2 = d2.DotProduct(edge_vector);
  // If edge is closer, than vertex - choose it
  if (dd1 * dd2 < 0) {
    float edge_length_sq = edge_vector.LengthSquared();
    dd1 /= edge_length_sq;
    Vector3 proj = p1 + edge_vector * dd1;
    return (proj - p).Length();
  } else if (dd1 < 0) {
    return d1.Length();
  } else {
    return d2.Length();
  }
}

// Distance from point to triangle
float point_triangle_distance(const Vector3& p, const Vector3& p1, const Vector3& p2, const Vector3& p3)
{
  Plane plane(p1, p2, p3);
  float plane_distance = plane.normal_.DotProduct(p - p1);
  float d1 = (p2 - p1).DotProduct(p - p1);
  float d2 = (p3 - p2).DotProduct(p - p2);
  float d3 = (p1 - p3).DotProduct(p - p3);
  // Inside
  if ((d1 > 0 && d2 > 0 && d3 > 0) || (d1 < 0 && d2 < 0 && d3 < 0)) {
    return plane_distance;
  }
  // Check edges
  float edge1_distance = point_edge_distance(p, p1, p2);
  float edge2_distance = point_edge_distance(p, p2, p3);
  float edge3_distance = point_edge_distance(p, p3, p1);
  return Min(Min(edge1_distance, edge2_distance), edge3_distance);
}

/// Find sub object, closest to the ray
int MeshGeometry::closest(
  const Ray& ray,
  SubObjectType& res_type,
  int types,
  unsigned int flags
) const
{
  URHO3D_PROFILE(Closest);
  float distance = M_INFINITY;
  int result = -1;

  // First - find closest vertex
  float vertex_distance = M_INFINITY;
  int vertex_index = closest_vertex(ray, flags, vertex_distance);

  // If only vertex needed - return it (early return)
  if ((types & ~(int)SubObjectType::VERTEX) == 0) {
    res_type = SubObjectType::VERTEX;
    return vertex_index;
  }

  // Not found
  if (vertex_index < 0) {
    return -1;
  }

  // Second - find closest edge, connected to closest vertex.
  // Assuming edge is always connected to the same type vertex.
  PODVector<int> edges = vertex_edges(vertex_index, flags);
  int edge_index = -1;
  float edge_distance = M_INFINITY;

  for (int i = 0; i < edges.Size(); ++i) {
    auto& edge = m_edges[edges[i]];
    float cur_edge_distance = point_edge_distance(
      ray.origin_,
      m_vertices[edge.vertexes[0]].position,
      m_vertices[edge.vertexes[1]].position
    );

    if (cur_edge_distance < edge_distance) {
      edge_distance = cur_edge_distance;
      edge_index = edges[i];
    }
  }

  // Third - closest polygon
  int polygon_index = -1;
  float polygon_distance = M_INFINITY;
  if (edge_index >= 0) {
    PODVector<int> polygons = edge_polygons(edge_index, flags);
    for (int i = 0; i < polygons.Size(); ++i) {
      auto& polygon = m_polygons[polygons[i]];
      float current_distance = point_triangle_distance(
        ray.origin_,
        m_vertices[polygon.vertexes[0]].position,
        m_vertices[polygon.vertexes[1]].position,
        m_vertices[polygon.vertexes[2]].position
      );
      if (current_distance < polygon_distance) {
        polygon_distance = current_distance;
        polygon_index = polygons[i];
      }
      if (polygon.vertexes[3] >= 0) {
        current_distance = point_triangle_distance(
          ray.origin_,
          m_vertices[polygon.vertexes[2]].position,
          m_vertices[polygon.vertexes[3]].position,
          m_vertices[polygon.vertexes[1]].position
        );
        if (current_distance < polygon_distance) {
          polygon_distance = current_distance;
          polygon_index = polygons[i];
        }
      }
    }
  }

  // Combine all together
  if (types & (int)SubObjectType::VERTEX) {
    if (vertex_distance < distance) {
      res_type = SubObjectType::VERTEX;
      distance = vertex_distance;
      result = vertex_index;
    }
  }

  if (types & (int)SubObjectType::EDGE) {
    if (edge_distance < distance) {
      res_type = SubObjectType::EDGE;
      distance = edge_distance;
      result = edge_index;
    }
  }

  if (types & (int)SubObjectType::POLYGON) {
    if (polygon_distance < distance) {
      res_type = SubObjectType::POLYGON;
      distance = polygon_distance;
      result = polygon_index;
    }
  }

  return result;
}

/// Find vertex, closest to the ray
int MeshGeometry::closest_vertex(
  const Ray& ray,
  unsigned int flags,
  float& distance
) const
{
  int result = -1;
  // TODO: use search tree for faster ray cast
  for (unsigned int i = 0; i < m_vertices.Size(); ++i) {
    const Vertex& vertex = m_vertices[i];
    if (vertex.check_flags(flags)) {
      if (vertex.normal.DotProduct(ray.direction_) < 0) {
        float cur_dist = (ray.origin_ - vertex.position).LengthSquared();
        if (cur_dist < distance) {
          distance = cur_dist;
          result = i;
        }
      }
    }
  }
  if (result >= 0) {
    distance = sqrt(distance);
  }
  return result;
}

/// Find edges by vertex and flag
PODVector<int> MeshGeometry::vertex_edges(
  int vertex, 
  unsigned int flags
) const
{
  PODVector<int> result;
  for (unsigned int i = 0; i < m_edges.Size(); ++i) {
    const Edge& edge = m_edges[i];
    if (edge.check_flags(flags)) {
      if (edge.vertexes[0] == vertex || edge.vertexes[1] == vertex) {
        result.Push(i);
      }
    }
  }
  return result;
}

/// Find faces by edge
PODVector<int> MeshGeometry::edge_polygons(int edge, unsigned int flags) const
{
  PODVector<int> result;
  int v1 = m_edges[edge].vertexes[0];
  int v2 = m_edges[edge].vertexes[1];
  for (unsigned int i = 0; i < m_polygons.Size(); ++i) {
    auto& polygon = m_polygons[i];
    if (polygon.check_flags(flags)) {
      if (polygon.has_vertex(v1) && polygon.has_vertex(v2)) {
        result.Push(i);
      }
    }
  }
  return result;
}

/// Calculate count of sub objects by flag
int MeshGeometry::primitives_count_by_flags(
  SubObjectType sub_type, 
  unsigned int flags
) const
{
  switch (sub_type) {
    case SubObjectType::VERTEX: 
      return primitives_count_by_flags(m_vertices, flags);
    case SubObjectType::EDGE:
      return primitives_count_by_flags(m_edges, flags);
    case SubObjectType::POLYGON:
      return primitives_count_by_flags(m_polygons, flags);
  }
  return 0;
}

/// Get primitive indexes by flag
template<class T>
PODVector<int> MeshGeometry::primitives_by_flags(
  const PODVector<T>& primitives, 
  unsigned int flags
) const
{
  PODVector<int> result;
  for (unsigned int i = 0; i < primitives.Size(); ++i) {
    if (primitives[i].check_flags(flags)) {
      result.Push(i);
    }
  }
  return result;
}

/// Get primitivs count by flag
template<class T>
int MeshGeometry::primitives_count_by_flags(
  const PODVector<T>& primitives, 
  unsigned int flags
) const
{
  int result = 0;
  for (unsigned int i = 0; i < primitives.Size(); ++i) {
    if (primitives[i].check_flags(flags)) {
      ++result;
    }
  }
  return result;
}

/// Raycast vertices
bool MeshGeometry::ray_cast_vertices(
  const Ray& ray,
  unsigned int flags,
  float& t,
  int& index
) const
{
  URHO3D_PROFILE(RayCastVertices);
  // TODO: use search tree for faster ray cast
  bool res = false;
  for (unsigned int i = 0; i < m_vertices.Size(); ++i) {
    const Vertex& vertex = m_vertices[i];
    if (vertex.check_flags(flags)) {
      float cur_t = ray.HitDistance(Sphere(vertex.position, vertex.radius));
      if (cur_t < t) {
        t = cur_t;
        index = i;
        res = true;
      }
      /*Vector3 pos_rel = vertex.position - ray.origin_;
      float r = vertex.radius;
      float a = 1;
      float b = -2 * ray.direction_.DotProduct(pos_rel);
      float c = pos_rel.LengthSquared() - r * r;
      float D = b * b - 4 * a * c;
      if (D >= 0) {
        float cur_t = (-b - sqrt(D)) / (2 * a);
        if (cur_t < t) {
          t = cur_t;
          index = i;
          res = true;
          // TODO: return position and normal, if needed
          //Vector3 pos = ray.direction_ * t + ray.origin_;
          //Vector3 normal = (pos - vertex.position).Normalized();
        }
      }*/
    }
  }
  return res;
}

float raycast_quadric(const Matrix4& q, const Ray& r)
{
  Vector4 C(r.origin_, 1.);
  Vector4 AC = q * C;
  Vector4 D = Vector4(r.direction_, 0.);
  Vector4 AD = q * D;
  float a = D.DotProduct(AD);
  float b = C.DotProduct(AD) + D.DotProduct(AC);
  float c = C.DotProduct(AC);
  float d = b * b - 4.0 * a * c;
  if (d >= 0.0) {
    float ds = sqrt(d);
    float t0 = ((-b + ds) / (2.0*a));
    float t1 = ((-b - ds) / (2.0*a));
    float tx = Max(t1, t0);
    float tn = Min(t1, t0);
    if (tn < 0.) tn = tx;
    if (tn >= 0.) {
      return tn;
    }

    /*vec4 pos = C + tn * D;
    vec3 norm = normalize((q * pos).xyz);
    if (dot(norm, r.d) > 0) {
      pos = C + tx * D;
      norm = normalize((q * pos).xyz);
      return Hit(tx, norm, 0);
    }
    else {
      return Hit(tn, norm, 0);
    }*/
  }
  return M_INFINITY;
}

/// Raycast edges
bool MeshGeometry::ray_cast_edges(
  const Ray& ray,
  unsigned int flags,
  float& t,
  int& index
) const
{
  URHO3D_PROFILE(RayCastEdges);
  // TODO: use search tree for faster ray cast
  bool res = false;
  for (unsigned int i = 0; i < m_edges.Size(); ++i) {
    const Edge& edge = m_edges[i];
    if (edge.check_flags(flags)) {
      const Vertex& v1 = m_vertices[edge.vertexes[0]];
      const Vertex& v2 = m_vertices[edge.vertexes[1]];
      Vector3 beam_dir = v2.position - v1.position;
      float pre_t = ray.HitDistance(
        Sphere(
          (v1.position + v2.position) * 0.5,
          beam_dir.Length() * 0.5
        )
      );
      if (pre_t < t) {
        // TODO: factor out radius calculation
        float r = Min(v1.radius, v2.radius) * 0.7;
        // Limit thickness of beam
        float beam_len = beam_dir.Length();
        r = Min(beam_len / 15, r);

        Vector3 up = beam_dir.CrossProduct(ray.direction_).Normalized();

        // TODO: check for parallel case
        Vector3 right = up.CrossProduct(beam_dir).Normalized();

        // TODO: optimize
        Vector3 r1 = beam_dir.Normalized();
        Vector3 tr = v1.position + beam_dir * 0.5;
        Matrix4 trans = Matrix4(
          r1.x_, r1.y_, r1.z_, 0.0,
          up.x_, up.y_, up.z_, 0.0,
          right.x_, right.y_, right.z_, 0.0,
          tr.x_, tr.y_, tr.z_, 1.0
        );
        trans = trans.Transpose().Inverse();
        Matrix4 inv_trans = trans.Transpose();
        float dist = (v2.position - v1.position).Length();
        dist = dist / 2.0;
        float cx = 1.0 / (dist * dist);
        float cyz = 1.0 / (r * r);
        Matrix4 cylinder = Matrix4(
          cx, 0.0, 0.0, 0.0,
          0.0, cyz, 0.0, 0.0,
          0.0, 0.0, cyz, 0.0,
          0.0, 0.0, 0.0, -1
        );

        Matrix4 trans_cylinder = inv_trans * cylinder * trans;
        float cur_t = raycast_quadric(trans_cylinder, ray);
        if (cur_t < t) {
          t = cur_t;
          index = i;
          res = true;
        }
      }
    }
  }

  return res;
}

/// Raycast polygons
bool MeshGeometry::ray_cast_polygons(
  const Ray& ray,
  unsigned int flags,
  float& t,
  int& index
) const
{
  URHO3D_PROFILE(RayCastPolygons);
  // TODO: use search tree for faster ray cast
  // TODO: backface picking does not work
  bool res = false;
  for (unsigned int i = 0; i < m_polygons.Size(); ++i) {
    const Polygon& polygon = m_polygons[i];
    if (polygon.check_flags(flags)) {
      /*float pre_t = ray.HitDistance(
        Sphere(
          (m_vertices[polygon.vertexes[0]].position, +v2.position) * 0.5,
          beam_dir.Length() * 0.5
        )
      );
      if (pre_t < t)*/ {
        float cur_t = ray.HitDistance(
          m_vertices[polygon.vertexes[0]].position,
          m_vertices[polygon.vertexes[1]].position,
          m_vertices[polygon.vertexes[2]].position
        );
        if (cur_t < t) {
          t = cur_t;
          index = i;
          res = true;
        }
        // Quad
        if (polygon.vertexes[3] >= 0) {
          cur_t = ray.HitDistance(
            m_vertices[polygon.vertexes[0]].position,
            m_vertices[polygon.vertexes[2]].position,
            m_vertices[polygon.vertexes[3]].position
          );
          if (cur_t < t) {
            t = cur_t;
            index = i;
            res = true;
          }
        }
      }
    }
  }

  return res;
}

/// Add notification receiver
void MeshGeometry::add_notification_receiver(NotificationReceiver* target)
{
  m_notification_receivers.Push(target);
}

/// Remove notofication receiver
void MeshGeometry::remove_notification_receiver(NotificationReceiver* target)
{
  m_notification_receivers.Remove(target);
}

/// Send update to all subscribers
void MeshGeometry::send_update(UpdateType type, int index)
{
  for (unsigned int i = 0; i < m_notification_receivers.Size(); ++i) {
    m_notification_receivers[i]->on_update(type, index);
  }
}
