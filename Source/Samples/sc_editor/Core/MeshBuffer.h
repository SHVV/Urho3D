// SHW Spacecraft editor
//
// Class for dynamically work with geometric meshes (creates all buffers)

#pragma once

// Editor includes
#include "MeshGeometry.h"

// Urho3D includes
#include <Urho3D/Core/Object.h>
#include <Urho3D/Graphics/Model.h>

using namespace Urho3D;

class MeshBuffer : public Object, public MeshGeometry::NotificationReceiver
{
  URHO3D_OBJECT(MeshBuffer, Object);

public:
  /// Default constructor.
  MeshBuffer(Context* context);

  /// Destructor
  virtual ~MeshBuffer();

  /// Set mesh geometry, that we must show
  void set_mesh_geometry(MeshGeometry* mesh_geometry);

  /// Get Urho3D Model
  Model* model();

  // Creating custom sets for higlighting and selection

  struct GeometryDescription {
    SubObjectType geometry_type;
    int material;
  };
  /// Geometry material description
  const GeometryDescription& geometry_description(int geometry_index);
  
  // Notifications
  class NotificationReceiver {
  public:
    virtual void on_update(){};
  };
  /// Add notification receiver
  void add_notification_receiver(NotificationReceiver* target);
  /// Remove notofication receiver
  void remove_notification_receiver(NotificationReceiver* target);


protected:
  /// Prepare everything before rendering
  void on_update_views(StringHash eventType, VariantMap& eventData);

  /// Do actual update
  bool update();

  /// Mesh geometry notifications
  virtual void on_update(MeshGeometry::UpdateType type, int i) override;

  /// Send update to all subscribers
  void send_update();

  ///// Helper function for creating geometries from indexes
  //void create_geometry(
  //  const Vector<PODVector<int>>& indexies,
  //  SubObjectType type,
  //  PODVector<GeometryDescription>& geometries_description,
  //  Vector<SharedPtr<Geometry>>& geometries
  //);

  /// Helper function for creating geometries from primitives
  template<class T>
  void create_geometry(
    const PODVector<T>& primitives,
    SubObjectType type,
    PODVector<GeometryDescription>& geometries_description,
    Vector<SharedPtr<Geometry>>& geometries,
    std::function<void(int, const T&, PODVector<int>&)> filler
  );

  /// Helper function for moving geometries from existing mesh
  void move_geometry(
    SubObjectType type,
    PODVector<GeometryDescription>& geometries_description,
    Vector<SharedPtr<Geometry>>& geometries
  );
private:

  /// Mesh, that we are watching
  SharedPtr<MeshGeometry> m_mesh_geometry;

  /// Buffers
  /// Vertex buffer for storing all nodes (balls, joints, plates)
  SharedPtr<VertexBuffer> m_vertex_buffer;
  /// Full model with everything in place
  SharedPtr<Model> m_model;
  /// Geometries description
  PODVector<GeometryDescription> m_geometries;

  /// Set of dirty vertices
  HashSet<int> m_dirty_vertices;
  /// Set of vertices to create
  //HashSet<int> m_new_vertices;
  /// Flag, that vertices index is dirty
  bool m_vertices_dirty;
  /// Flag, that edge index is dirty
  bool m_edges_dirty;
  /// Flag, that polygons index is dirty
  bool m_polygons_dirty;
  /// Flag, that everything is dirty
  bool m_dirty;

  /// Notification receivers
  PODVector<NotificationReceiver*> m_notification_receivers;
};
