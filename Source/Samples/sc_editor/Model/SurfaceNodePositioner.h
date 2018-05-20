// SHW Spacecraft editor
//
// Surface-Surface attachment. Used to adjust position relative to parent

#pragma once

#include "BasePositioner.h"
#include "BaseAttachableSurface.h"
#include "ProceduralUnit.h"
#include "SurfaceMount.h"

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

  /// Set position.
  /// Returns true, if attachment was successfull, false - otherwise.
  virtual bool set_position(
    const Vector3& position,
    const Vector3& normal,
    const Quaternion& rotation
  ) override;
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

  /// Set allowed sub-objects
  void set_sub_objects(int value);
  /// Get average edge size
  float average_attachable_edge();
  
  // TODO: Create in proper position, potentially can have several positions for two points units
  //bool update_position(Node* unit_under_mouse, const Ray& pointer_ray, rotation, symmetry_rotation);
  // TODO: attach and listen for attached events
  // TODO: Update on changes

protected:
  // Existing overrides
  /// Handle node being assigned.
  virtual void OnNodeSet(Node* node) override;

  /// Get surface we attached to.
  BaseAttachableSurface* get_surface();
  /// Get surface mount
  SurfaceMount* get_surface_mount();
  /// Get or create mount adapter
  ProceduralUnit* get_mount_adapter(bool create = true);

  /// Convert rotation to angles, and limit them.
  Vector3 calculate_angles(const Quaternion& rotation, SubObjectType snapped_to);
  /// Handle node (not only our) transform being dirtied.
  //virtual void OnMarkedDirty(Node* node) override;
  /// Update mount adapter, if nedded
  void update_adapter();

  /// Event handler on attachable surface change
  void on_surface_changed(
    StringHash eventType,
    VariantMap& eventData
  );

  /// Event handler on surface mount change
  void on_mount_changed(
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
  /// Allowed sub objects
  int m_sub_objects = (int)SubObjectType::POLYGON | (int)SubObjectType::EDGE;
};
