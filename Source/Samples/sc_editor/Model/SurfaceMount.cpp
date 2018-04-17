// SHW Spacecraft editor
//
// Surface mount component. 
// Marks ability to attach unit to surface and defines size of required plate.

#include "SurfaceMount.h"
#include <Urho3D\Core\Context.h>

using namespace Urho3D;

extern const char* EDITOR_CATEGORY;

/// Register object attributes.
void SurfaceMount::RegisterObject(Context* context)
{
  context->RegisterFactory<SurfaceMount>(EDITOR_CATEGORY);

  // Add attributes
  URHO3D_ATTRIBUTE("Mount Size", float, m_mount_size, 0.0, AM_DEFAULT | AM_NOEDIT);
}

/// Construct.
SurfaceMount::SurfaceMount(Context* context)
  : Component(context)
{
}

/// Set mount size
void SurfaceMount::set_mount_size(double value)
{
  if (m_mount_size != value) {
    m_mount_size = value;

    // Notify subscribers
    using namespace SurfaceMountChanged;

    VariantMap& event_data = GetEventDataMap();
    event_data[P_COMP] = this;

    SendEvent(E_SURFACE_MOUNT_CHANGED, event_data);
  }
}

/// Get mount size
double SurfaceMount::mount_size() const
{
  return m_mount_size;
}
