// SHW Spacecraft editor
//
// Base positioner component. ~Knows, how to insert unit into space or another unit.
// Used for update position of node, based on attachment

#pragma once

#include <Urho3D/Scene/Component.h>

namespace Urho3D {
  class Ray;
}

using namespace Urho3D;

enum class Axis {
  NONE = 0,
  X = 1,
  Y = 2,
  Z = 4,
  XY = X | Y,
  XZ = X | Z,
  YZ = Y | Z,
  XYZ = X | Y | Z
};

enum class MoveSpace {
  LOCAL,    // Move along local axis
  PARENT,   // Move along parent axis
  GLOBAL    // Move along global axis
};

class BasePositioner : public Component {
  // Enable type information.
  URHO3D_OBJECT(BasePositioner, Component);
public:

  /// Construct.
  BasePositioner(Context* context);

  /// Destructor
  virtual ~BasePositioner() override = default;

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
  );
  /// Updates internal position representation, based on current node position.
  virtual void update_internal_position();
  /// Updates node position, based on reference and internal position.
  virtual void update_node_position();

  /// Get supported moves along local linear axis
  virtual Axis linear_axis();
  /// Get supported rotations around local linear axis
  virtual Axis rotation_axis();
  /// Returns move space for moving this node
  //virtual MoveSpace move_space();
  /// Calculate and return gizmo orientation in world coordinates
  virtual void axis(
    Vector3& pos,
    Vector3& axis_x,
    Vector3& axis_y,
    Vector3& axis_z
  );

  // TODO: Create in proper position, potentially can have several positions for two points units
  //bool update_position(Node* unit_under_mouse, const Ray& pointer_ray, rotation, symmetry_rotation);
  // TODO: Symmetry support
  // TODO: attach and listen for attached events
  // TODO: Move-rotate
  // TODO: Update on changes

  // TODO: ...

protected:
  // Existing overrides
  /// Handle node being assigned.
  virtual void OnNodeSet(Node* node) override;
  /// Handle node (not only our) transform being dirtied.
  //virtual void OnMarkedDirty(Node* node) override;

  /// Checks, is node lays on parent Z axis.
  bool is_on_z_axis();

private:

};
