// SHW Spacecraft editor
//
// Class for dynamically work with geometric meshes

#include "MeshGeometry.h"

// Includes from Urho3D
#include "Urho3D/Core/Profiler.h"
#include "Urho3D/Math/Ray.h"
#include "Urho3D/Math/Sphere.h"

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

/// Pack everything, if there are too much empty objects
void MeshGeometry::compact()
{
  // TODO: implement
}

MeshGeometry::Vertex* MeshGeometry::get_vertex(int index)
{
  if (index < 0 || index >= m_vertices.Size()) {
    return nullptr;
  }
  if (m_vertices[index].deleted) {
    return nullptr;
  }
  return &m_vertices[index];
}

MeshGeometry::Edge* MeshGeometry::get_edge(int index)
{
  if (index < 0 || index >= m_edges.Size()) {
    return nullptr;
  }
  if (m_edges[index].deleted) {
    return nullptr;
  }
  return &m_edges[index];
}

MeshGeometry::Polygon* MeshGeometry::get_polygon(int index)
{
  if (index < 0 || index >= m_polygons.Size()) {
    return nullptr;
  }
  if (m_polygons[index].deleted) {
    return nullptr;
  }
  return &m_polygons[index];
}

/// Raycast mesh. Returns index of sub object and its type
int MeshGeometry::raycast(
  const Ray& ray, 
  SubObjectType& res_type, 
  int types, 
  bool pick_hidden
)
{
  URHO3D_PROFILE(RayCast);

  float t = M_INFINITY;
  int index = -1;

  if (types & sotVERTEX) {
    if (ray_cast_vertices(ray, pick_hidden, t, index)) {
      res_type = sotVERTEX;
    }
  }

  if (types & sotEDGE) {
    if (ray_cast_edges(ray, pick_hidden, t, index)) {
      res_type = sotEDGE;
    }
  }

  if (types & sotPOLYGON) {
    if (ray_cast_polygons(ray, pick_hidden, t, index)) {
      res_type = sotPOLYGON;
    }
  }

  return index;
}

/// Raycast vertices
bool MeshGeometry::ray_cast_vertices(
  const Ray& ray,
  bool pick_hidden,
  float& t,
  int& index
)
{
  URHO3D_PROFILE(RayCastVertices);
  // TODO: use search tree for faster ray cast
  bool res = false;
  for (int i = 0; i < m_vertices.Size(); ++i) {
    const Vertex& vertex = m_vertices[i];
    if (!vertex.deleted && (pick_hidden || vertex.material >= 0)) {
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
  bool pick_hidden,
  float& t,
  int& index
)
{
  URHO3D_PROFILE(RayCastEdges);
  // TODO: use search tree for faster ray cast
  bool res = false;
  for (int i = 0; i < m_edges.Size(); ++i) {
    const Edge& edge = m_edges[i];
    if (!edge.deleted && (pick_hidden || edge.material >= 0)) {
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
        r = Min(beam_len / 30, r);

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
  bool pick_hidden,
  float& t,
  int& index
)
{
  URHO3D_PROFILE(RayCastPolygons);
  // TODO: use search tree for faster ray cast
  bool res = false;
  for (int i = 0; i < m_polygons.Size(); ++i) {
    const Polygon& polygon = m_polygons[i];
    if (!polygon.deleted && (pick_hidden || polygon.material >= 0)) {
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
  for (int i = 0; i < m_notification_receivers.Size(); ++i) {
    m_notification_receivers[i]->on_update(type, index);
  }
}
