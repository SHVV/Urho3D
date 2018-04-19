// SHW Spacecraft editor
//
// Surface-Surface attachment. Used to adjust position relative to parent

#pragma once

#include "BasePositioner.h"
#include "BaseAttachableSurface.h"

using namespace Urho3D;

class SurfaceNodePositioner : public BasePositioner {
  // Enable type information.
  URHO3D_OBJECT(SurfaceNodePositioner, BasePositioner);
public:

  /// Construct.
  SurfaceNodePositioner(Context* context);

  /// Destructor
  virtual ~SurfaceNodePositioner() override = default;

  /// Register object attributes.
  static void RegisterObject(Context* context);

  /// Handle attribute write access.
  //virtual void OnSetAttribute(const AttributeInfo& attr, const Variant& src);
  /// Apply attribute changes that can not be applied immediately. Called after scene load or a network update.
  //virtual void ApplyAttributes() override;
  // TODO: add other component methods, that we need to override

  // GetDependencyNodes?

  /// Updates internal position representation, based on current node position.
  virtual void update_internal_position() override;
  /// Updates node position, based on reference and internal position.
  virtual void update_node_position() override;

  /// Get supported moves along local linear axis
  virtual Axis linear_axis() override;
  /// Get supported rotations around local linear axis
  virtual Axis rotation_axis() override;
  /// Returns move space for moving this node
  //virtual MoveSpace move_space() override;
  /// Calculate and return gizmo orientation in world coordinates
  virtual void axis(
    Vector3& pos,
    Vector3& axis_x,
    Vector3& axis_y,
    Vector3& axis_z
  );


  // TODO: Create in proper position, potentially can have several positions for two points units
  //bool update_position(Node* unit_under_mouse, const Ray& pointer_ray, rotation, symmetry_rotation);
  // TODO: attach and listen for attached events
  // TODO: Update on changes

  /// Set position.
  /// Returns true, if attachment was successfull, false - otherwise.
  bool set_position(const Vector3& position, const Vector3& normal, const Quaternion& rotation);

protected:
  // Existing overrides
  /// Handle node being assigned.
  virtual void OnNodeSet(Node* node) override;
  /// Get surface we attached to.
  BaseAttachableSurface* get_surface();
  /// Convert rotation to angles, and limit them.
  Vector3 calculate_angles(const Quaternion& rotation, SubObjectType snapped_to);
  /// Handle node (not only our) transform being dirtied.
  //virtual void OnMarkedDirty(Node* node) override;

  /// Event handler on attachable surface change
  void on_changed(
    StringHash eventType,
    VariantMap& eventData
  );

  /// Calculate shift, based on attachment (and may be size)
  float calculate_shift();

private:
  /// Topology attachment
  SharedPtr<BaseTopologyAttachment> m_attachment;
  /// Euler angles
  Vector3 m_angles;
  /// Surface, we attached to
  WeakPtr<BaseAttachableSurface> m_surface;
};
