// SHW Spacecraft editor
//
// Surface-Surface attachment. Used to adjust position relative to parent

#pragma once

#include "BasePositioner.h"

using namespace Urho3D;

class SurfaceSurfacePositioner : public BasePositioner {
  // Enable type information.
  URHO3D_OBJECT(SurfaceSurfacePositioner, BasePositioner);
public:

  /// Construct.
  SurfaceSurfacePositioner(Context* context);

  /// Destructor
  virtual ~SurfaceSurfacePositioner() override = default;

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
  //virtual Axis linear_axis() override;
  /// Get supported rotations around local linear axis
  //virtual Axis rotation_axis() override;
  /// Returns move space for moving this node
  //virtual MoveSpace move_space() override;

  // TODO: Create in proper position, potentially can have several positions for two points units
  //bool update_position(Node* unit_under_mouse, const Ray& pointer_ray, rotation, symmetry_rotation);
  // TODO: attach and listen for attached events
  // TODO: Update on changes

protected:
  // Existing overrides
  /// Handle node being assigned.
  //virtual void OnNodeSet(Node* node) override;
  /// Handle node (not only our) transform being dirtied.
  //virtual void OnMarkedDirty(Node* node) override;

private:

};
