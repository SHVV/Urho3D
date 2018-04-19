// SHW Spacecraft editor
//
// Class for dynamically work with geometric meshes

#pragma once

#include <Urho3D/Core/Object.h>

namespace Urho3D {
  class Ray;
};

using namespace Urho3D;

// TODO: switch to std::bitset
enum class SubObjectType {
  NONE = 0,
  VERTEX = 1,
  EDGE = 2,
  POLYGON = 4
};

// Basic mesh flags
// TODO: refactor
#define mgfVISIBLE  1
#define mgfCOLLISION 2
#define mgfATTACHABLE 4

// TODO: extract painting
// TODO: LODs as flags
// TODO: abstract components instead of hardcoded structures
class MeshGeometry : public Object
{
  URHO3D_OBJECT(MeshGeometry, Object);

public:
  /// Default constructor.
  MeshGeometry(Context* context);

  /// Destructor
  virtual ~MeshGeometry();

  // Internal structures

  // Base primitive
  struct BasePrimitive {
    int material = 0;
    unsigned int flags = 0;
    bool deleted = false;

    bool check_flags(unsigned int a_flags) const
    {
      return !deleted && (!a_flags || (a_flags & flags) != 0);
    }
  };

  // Vertices
  struct Vertex : BasePrimitive {
    Vector3 position;
    Vector3 normal;
    // TODO: add UVW coordinates
    //Vector3 UVW;
    float radius = 0;
    float scale = 1;
  };

  // Edges
  struct Edge : BasePrimitive {
    int vertexes[2];
    bool secondary = false;
    Vector3 center(const MeshGeometry& mesh) const {
      return (mesh.m_vertices[vertexes[0]].position + 
              mesh.m_vertices[vertexes[1]].position) * 0.5;
    };
    Vector3 normal(const MeshGeometry& mesh) const {
      return (mesh.m_vertices[vertexes[0]].normal +
              mesh.m_vertices[vertexes[1]].normal).Normalized();
    };
    Vector3 direction(const MeshGeometry& mesh) const {
      return (mesh.m_vertices[vertexes[1]].position -
              mesh.m_vertices[vertexes[0]].position).Normalized();
    };
    float radius(const MeshGeometry& mesh) const {
      return Min(mesh.m_vertices[vertexes[1]].radius,
                 mesh.m_vertices[vertexes[0]].radius) * 0.7;
    }
  };

  // Polygons
  struct Polygon : BasePrimitive {
    // TODO: support any polygons
    int vertexes[4];

    Polygon() : BasePrimitive()
    {
      vertexes[3] = -1;
    }
    bool has_vertex(int i) const {
      return
        vertexes[0] == i ||
        vertexes[1] == i ||
        vertexes[2] == i ||
        vertexes[3] == i;
    }
    Vector3 center(const MeshGeometry& mesh) const {
      int i = 0;
      Vector3 result = Vector3::ZERO;
      while (i < 4 && vertexes[i] >= 0) {
        result += mesh.m_vertices[vertexes[i]].position;
        ++i;
      }
      return result / i;
    };
    Vector3 normal(const MeshGeometry& mesh) const {
      int i = 0;
      Vector3 result = Vector3::ZERO;
      while (i < 4 && vertexes[i] >= 0) {
        result += mesh.m_vertices[vertexes[i]].normal;
        ++i;
      }
      return result.Normalized();
    };
    float radius(const MeshGeometry& mesh) const {
      float result = 0;
      for (int i = 0; i < 4 && vertexes[i] >= 0; ++i) {
        if (result < mesh.m_vertices[vertexes[i]].radius) {
          result = mesh.m_vertices[vertexes[i]].radius;
        }
      }
      return result;
    }
  };

  // Access functions

  /// Vertices list
  const PODVector<Vertex>& vertices() const;

  /// Edges list
  const PODVector<Edge>& edges() const;

  /// Polygons list
  const PODVector<Polygon>& polygons() const;

  // Manipulation functions

  // Vertices
  /// Add
  int add(const Vertex& vertex);
  int add(const Vector3& pos);
  int add(const Vector3& pos, float radius);
  int add(const Vector3& pos, const Vector3& normal);
  
  /// Remove
  bool remove_vertex(int index);

  /// Set position
  bool set_position(int index, Vector3 pos);

  /// Set normal
  bool set_normal(int index, Vector3 norm);

  /// Set radius
  bool set_radius(int index, float radius);

  /// Set scale
  bool set_scale(int index, float scale);

  /// Set material
  bool set_vertex_material(int index, int material);

  /// Set flags
  bool set_vertex_flags(int index, unsigned int flags);

  // Edges
  /// Add
  int add(const Edge& edge);
  int add(int v1, int v2);

  /// Remove
  bool remove_edge(int index);

  /// Set material
  bool set_edge_material(int index, int material);

  /// Set flags
  bool set_edge_flags(int index, unsigned int flags);

  // Polygons
  /// Add
  int add(const Polygon& polygon);
  int add(int v1, int v2, int v3);
  int add(int v1, int v2, int v3, int v4);

  /// Remove
  bool remove_polygon(int index);

  /// Set material
  bool set_polygon_material(int index, int material);

  /// Set flags
  bool set_polygon_flags(int index, unsigned int flags);

  /// Pack everything, if there are too much empty objects
  void compact();

  /// Transform mesh with matrix
  void transform(const Matrix3x4& tr);

  /// Raycast mesh. Returns index of sub object and its type
  int raycast(
    const Ray& ray,
    SubObjectType& res_type,
    int types,
    unsigned int flags
  ) const;
  /// The same with returning t result
  int raycast(
    const Ray& ray,
    SubObjectType& res_type,
    int types,
    unsigned int flags,
    float& t
  ) const;

  /// Find sub object, closest to the ray
  int closest(
    const Ray& ray,
    SubObjectType& res_type,
    int types,
    unsigned int flags
  ) const;

  /// Find edges by vertex and flag
  PODVector<int> vertex_edges(int vertex, unsigned int flags) const;

  /// Find faces by edge
  PODVector<int> edge_polygons(int edge, unsigned int flags) const;

  /// Returns indexies of all verteces by flags
  PODVector<int>& vertices_by_flags(unsigned int flags) const;
  /// Returns indexies of all edges by flags
  PODVector<int>& edges_by_flags(unsigned int flags) const;
  /// Returns indexies of all polygons by flags
  PODVector<int>& polygons_by_flags(unsigned int flags) const;

  /// Calculate count of sub objects by flag
  int primitives_count_by_flags(SubObjectType sub_type, unsigned int flags) const;

  /// Calcualte and return intermediate position of sub-object
  Vector3 position(SubObjectType& type, int index) const;

  /// Calcualte and return intermediate normal of sub-object
  Vector3 normal(SubObjectType& type, int index) const;

  // TODO: names for material slots
  // TODO: ability to merge models and combine material IDs by slot names

  // Notifications
  // TODO: batch updates
  enum UpdateType{
    VERTEX_ADDED,
    VERTEX_REMOVED,
    VERTEX_CHANGED,
    VERTEX_PAINTED,
    EDGE_ADDED,
    EDGE_REMOVED,
    EDGE_PAINTED,
    POLYGON_ADDED,
    POLYGON_REMOVED,
    POLYGON_PAINTED,
    GLOBAL_UPDATE
  };

  class NotificationReceiver {
  public:
    virtual void on_update(UpdateType type, int i){};
  };
  /// Add notification receiver
  void add_notification_receiver(NotificationReceiver* target);
  /// Remove notofication receiver
  void remove_notification_receiver(NotificationReceiver* target);

protected:
  /// Getters for internal usage
  Vertex* get_vertex(int index);
  Edge* get_edge(int index);
  Polygon* get_polygon(int index);

  /// Get primitive indexes by flag
  template<class T>
  PODVector<int>& primitives_by_flags(
    unsigned int flags,
    const PODVector<T>& primitives,
    HashMap<int, PODVector<int>>& index_map
  ) const;

  /// Get primitivs count by flag
  template<class T>
  int primitives_count_by_flags(const PODVector<T>& primitives, unsigned int flags) const;

  /// Raycast vertices
  bool ray_cast_vertices(const Ray& ray, unsigned int flags, float& t, int& index) const;

  /// Raycast edges
  bool ray_cast_edges(const Ray& ray, unsigned int flags, float& t, int& index) const;

  /// Raycast polygons
  bool ray_cast_polygons(const Ray& ray, unsigned int flags, float& t, int& index) const;

  /// Find vertex, closest to the ray origin
  bool closest_vertex(
    const Ray& ray,
    unsigned int flags,
    float& distance,
    int& index
  ) const;

  /// Find edge, closest to the ray origin
  bool closest_edge(
    const Ray& ray,
    unsigned int flags,
    float& distance,
    int& index
  ) const;

  /// Find polygon, closest to the ray origin
  bool closest_polygon(
    const Ray& ray,
    unsigned int flags,
    float& distance,
    int& index
  ) const;

  /// Send update to all subscribers
  void send_update(UpdateType type, int i = -1);

private:
  /// Vertices
  PODVector<Vertex> m_vertices;
  /// Edges
  PODVector<Edge> m_edges;
  /// Polygons
  PODVector<Polygon> m_polygons;

  /// Free vertex positions
  PODVector<int> m_free_vertices;
  /// Free edge positions
  PODVector<int> m_free_edges;
  /// Free polygon positions
  PODVector<int> m_free_polygons;

  /// Cached indexes by flag
  mutable HashMap<int, PODVector<int>> m_vertex_by_flag;
  mutable HashMap<int, PODVector<int>> m_edge_by_flag;
  mutable HashMap<int, PODVector<int>> m_polygon_by_flag;

  /// Notification receivers
  PODVector<NotificationReceiver*> m_notification_receivers;
};
