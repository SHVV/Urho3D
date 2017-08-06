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
enum SubObjectType {
  sotVERTEX = 1,
  sotEDGE = 2,
  sotPOLYGON = 4
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
  };

  // Polygons
  struct Polygon : BasePrimitive {
    // TODO: support any polygons
    int vertexes[4];

    Polygon() : BasePrimitive()
    {
      vertexes[3] = -1;
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

  /// Returns indexies of all verteces by flags
  PODVector<int> vertices_by_flags(unsigned int flags) const;
  /// Returns indexies of all edges by flags
  PODVector<int> edges_by_flags(unsigned int flags) const;
  /// Returns indexies of all polygons by flags
  PODVector<int> polygons_by_flags(unsigned int flags) const;

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
  PODVector<int> primitives_by_flags(const PODVector<T>& primitives, unsigned int flags) const;

  /// Raycast vertices
  bool ray_cast_vertices(const Ray& ray, unsigned int flags, float& t, int& index) const;

  /// Raycast edges
  bool ray_cast_edges(const Ray& ray, unsigned int flags, float& t, int& index) const;

  /// Raycast polygons
  bool ray_cast_polygons(const Ray& ray, unsigned int flags, float& t, int& index) const;

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

  /// Notification receivers
  PODVector<NotificationReceiver*> m_notification_receivers;
};
