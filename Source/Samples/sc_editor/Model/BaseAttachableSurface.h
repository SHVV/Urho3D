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

using namespace Urho3D;

// Base structure for topology independent attchment.
// Used for preserving attachment coordinates during topology changes
class BaseTopologyAttachment : public RefCounted {
public:
  BaseTopologyAttachment(
    const Vector3& position,
    const Vector3& normal,
    SubObjectType snaped_to
  ) 
    : m_position(position),
      m_normal(normal),
      m_snaped_to(snaped_to){};
  virtual ~BaseTopologyAttachment() = default;

  SubObjectType snapped_to() const { return m_snaped_to; };
  const Vector3& position() const { return m_position; };
  const Vector3& normal() const { return m_normal; };

protected:
  Vector3 m_position;
  Vector3 m_normal;
  SubObjectType m_snaped_to;
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
  SharedPtr<BaseTopologyAttachment> local_to_topology(
    Vector3& position,
    Vector3& normal,
    Vector3& tangent,
    int snap_to = (int)SubObjectType::NONE
  );
  
  /// Convert topology position to local position
  bool topology_to_local(
    const BaseTopologyAttachment& topology_position,
    Vector3& position,
    Vector3& normal,
    Vector3& tangent
  );

protected:
  // Existing overrides
  /// Handle node being assigned.
  //virtual void OnNodeSet(Node* node) override;
  /// Handle node (not only our) transform being dirtied.
  //virtual void OnMarkedDirty(Node* node) override;

private:

};
