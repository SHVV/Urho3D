// SHW Spacecraft editor
//
// Base attachable surface. 
// Used to preserve attachment coordinates during topology changes.

#pragma once

#include "../Core/MeshGeometry.h"
#include <Urho3D/Scene/Component.h>

namespace Urho3D {
  class Ray;
}

class DynamicModel;

using namespace Urho3D;

/// Attachable surface changed.
URHO3D_EVENT(E_ATTACHABLE_SURFACE_CHANGED, AttachableSurfaceChanged)
{
  URHO3D_PARAM(P_COMP, Component);                    // Component pointer
}

// Base structure for topology independent attchment.
// Used for preserving attachment coordinates during topology changes
class BaseTopologyAttachment : public RefCounted {
public:
  BaseTopologyAttachment(
    const Vector3& position,
    const Vector3& normal,
    SubObjectType snaped_to,
    int primitive_index,
    int primitives_count
  ) 
    : m_position(position),
      m_normal(normal),
      m_snaped_to(snaped_to),
      m_primitive_index(primitive_index),
      m_primitives_count(primitives_count) {};
  virtual ~BaseTopologyAttachment() = default;

  SubObjectType snapped_to() const { return m_snaped_to; };
  const Vector3& position() const { return m_position; };
  const Vector3& normal() const { return m_normal; };
  int primitive_index() const { return m_primitive_index; };
  int primitives_count() const { return m_primitives_count; };

protected:
  Vector3 m_position;
  Vector3 m_normal;
  SubObjectType m_snaped_to;
  int m_primitive_index;
  int m_primitives_count;
};

class BaseAttachableSurface : public Component {
  // Enable type information.
  URHO3D_OBJECT(BaseAttachableSurface, Component);
public:

  /// Construct.
  BaseAttachableSurface(Context* context);

  /// Destructor
  virtual ~BaseAttachableSurface() override = default;

  /// Register object attributes.
  static void RegisterObject(Context* context);

  /// Handle attribute write access.
  //virtual void OnSetAttribute(const AttributeInfo& attr, const Variant& src);
  /// Apply attribute changes that can not be applied immediately. Called after scene load or a network update.
  //virtual void ApplyAttributes() override;
  // TODO: add other component methods, that we need to override

  // GetDependencyNodes?

  /// Convert local coordintaes to topology coordinates. 
  /// With optional snapping, in this case position and normal will be adjusted
  virtual SharedPtr<BaseTopologyAttachment> local_to_topology(
    Vector3& position,
    Vector3& normal,
    Vector3& tangent,
    int snap_to = (int)SubObjectType::NONE,
    bool snap_optional = true
  );
  
  /// Convert topology position to local position
  virtual bool topology_to_local(
    BaseTopologyAttachment& topology_position,
    Vector3& position,
    Vector3& normal,
    Vector3& tangent
  );

  // Different queries:
  /// Farthest point in direction
  virtual float farthest_vertex(
    const Vector3& direction,
    Vector3& position,
    Vector3& normal
  );

  /// Returns average length of attachable edge.
  virtual float average_attachable_edge();

  /// Dynamic model component for attaching to.
  DynamicModel* dynamic_model();

protected:
  /// Convert sub-object into local position
  void sub_object_to_local(
    SubObjectType sub_type,
    int sub_index,
    Vector3& position,
    Vector3& normal,
    Vector3& tangent
  );

  bool snap_to_primitive(
    Vector3& position,
    Vector3& normal,
    Vector3& tangent,
    int snap_to,
    bool snap_optional,
    SubObjectType& snap_type,
    int& primitive_index
  );

  /// Event handler on dynamic model change
  void on_changed(
    StringHash eventType,
    VariantMap& eventData
  );

  /// Send notification on changed
  void notify_changed();

  /// Convert local position to topology independent one
  virtual Vector3 local_to_topology(
    const Vector3& position,
    const Vector3& normal
  );

  /// Convert position from topology independent to local
  virtual Vector3 topology_to_local(
    const Vector3& norm_position,
    const Vector3& normal
  );

  // Existing overrides
  /// Handle node being assigned.
  //virtual void OnNodeSet(Node* node) override;
  /// Handle node (not only our) transform being dirtied.
  //virtual void OnMarkedDirty(Node* node) override;

private:
  /// Cached dynamic model component for attaching to.
  WeakPtr<DynamicModel> m_dynamic_model;
};
