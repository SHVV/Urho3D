// SHW Spacecraft editor
//
// Base attachable surface. 
// Used to preserve attachment coordinates during topology changes.

#include "Voxel1DAttachableSurface.h"
#include <Urho3D\Core\Context.h>

using namespace Urho3D;

extern const char* EDITOR_CATEGORY;

/// Register object attributes.
void Voxel1DAttachableSurface::RegisterObject(Context* context)
{
  context->RegisterFactory<Voxel1DAttachableSurface>(EDITOR_CATEGORY);

  // TODO: add Attributes
  //URHO3D_ENUM_ATTRIBUTE("State", m_state, state_names, usNONE, AM_NET | AM_NOEDIT);
}

/// Construct.
Voxel1DAttachableSurface::Voxel1DAttachableSurface(Context* context)
  : BaseAttachableSurface(context)
{
}

/// Initialize parameters
void Voxel1DAttachableSurface::initialize(float cell_size, float shift)
{
  m_cell_size = cell_size;
  m_shift = shift;
  notify_changed();
}

/// Convert local position to topology independent one
Vector3 Voxel1DAttachableSurface::local_to_topology(
  const Vector3& position,
  const Vector3& normal
)
{
  Vector3 result = position;
  result.z_ -= m_shift;
  result /= m_cell_size;
  return result;
}

/// Convert position from topology independent to local
Vector3 Voxel1DAttachableSurface::topology_to_local(
  const Vector3& norm_position,
  const Vector3& normal
)
{
  Vector3 result = norm_position;
  result *= m_cell_size;
  result.z_ += m_shift;
  return result;
}