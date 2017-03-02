// SHW Spacecraft editor
//
// Class for dynamically work with geometric meshes

#pragma once

#include <Urho3D/Core/Object.h>

namespace Urho3D {
  class Ray;
};

using namespace Urho3D;

enum SubObjectType {
  sotVERTEX = 1,
  sotEDGE = 2,
  sotPOLYGON = 4
};

// TODO: extract painting
class MeshGeometry : public Object
{
  URHO3D_OBJECT(MeshGeometry, Object);

public:
  /// Default constructor.
  MeshGeometry(Context* context);

  /// Destructor
  virtual ~MeshGeometry();

  // Internal structures

  // Vertices
  struct Vertex {
    Vector3 position;
    Vector3 normal;
    // TODO: add UVW coordinates
    //Vector3 UVW;
    float radius;
    int material;
    bool deleted;

    Vertex() :
      radius(0),
      material(0),
      deleted(false)
    {};
  };

  // Edges
  struct Edge {
    int vertexes[2];
    int material;
    bool secondary;
    bool deleted;

    Edge() :
      material(0),
      secondary(false),
      deleted(false)
    {}
  };

  // Polygons
  struct Polygon {
    // TODO: support any polygons
    int vertexes[4];
    int material;
    bool deleted;

    Polygon() :
      material(0),
      deleted(false)
    {}
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

  /// Set material
  bool set_vertex_material(int index, int material);

  // Edges
  /// Add
  int add(const Edge& edge);
  int add(int v1, int v2);

  /// Remove
  bool remove_edge(int index);

  /// Set material
  bool set_edge_material(int index, int material);

  // Polygons
  /// Add
  int add(const Polygon& polygon);
  int add(int v1, int v2, int v3);
  int add(int v1, int v2, int v3, int v4);

  /// Remove
  bool remove_polygon(int index);

  /// Set material
  bool set_polygon_material(int index, int material);

  /// Pack everything, if there are too much empty objects
  void compact();

  /// Raycast mesh. Returns index of sub object and its type
  int raycast(
    const Ray& ray,
    SubObjectType& res_type,
    int types,
    bool pick_hidden
  );

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

  /// Raycast vertices
  bool ray_cast_vertices(const Ray& ray, bool pick_hidden, float& t, int& index);

  /// Raycast edges
  bool ray_cast_edges(const Ray& ray, bool pick_hidden, float& t, int& index);

  /// Raycast polygons
  bool ray_cast_polygons(const Ray& ray, bool pick_hidden, float& t, int& index);

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
