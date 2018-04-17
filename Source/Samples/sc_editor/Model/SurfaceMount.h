// SHW Spacecraft editor
//
// Surface mount component. 
// Marks ability to attach unit to surface and defines size of required plate.

#pragma once

#include <Urho3D/Scene/Component.h>

using namespace Urho3D;

/// Attachable surface changed.
URHO3D_EVENT(E_SURFACE_MOUNT_CHANGED, SurfaceMountChanged)
{
  URHO3D_PARAM(P_COMP, Component);                    // Component pointer
}

class SurfaceMount : public Component {
  // Enable type information.
  URHO3D_OBJECT(SurfaceMount, Component);
public:

  /// Construct.
  SurfaceMount(Context* context);

  /// Destructor
  virtual ~SurfaceMount() override = default;

  /// Register object attributes.
  static void RegisterObject(Context* context);

  /// Handle attribute write access.
  //virtual void OnSetAttribute(const AttributeInfo& attr, const Variant& src);
  /// Apply attribute changes that can not be applied immediately. Called after scene load or a network update.
  //virtual void ApplyAttributes() override;

  /// Set mount size
  void set_mount_size(double value);

  /// Get mount size
  double mount_size() const;

private:
  /// Mount size.
  double m_mount_size;
};
