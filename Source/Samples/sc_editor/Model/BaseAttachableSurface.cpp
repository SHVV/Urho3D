// SHW Spacecraft editor
//
// Base attachable surface. 
// Used to preserve attachment coordinates during topology changes.

#include "BaseAttachableSurface.h"

#include <Urho3D\Core\Context.h>
#include <Urho3D\Scene\Node.h>

using namespace Urho3D;

extern const char* EDITOR_CATEGORY;

/// Register object attributes.
void BaseAttachableSurface::RegisterObject(Context* context)
{
  context->RegisterFactory<BaseAttachableSurface>(EDITOR_CATEGORY);

  // TODO: add Attributes
  //URHO3D_ENUM_ATTRIBUTE("State", m_state, state_names, usNONE, AM_NET | AM_NOEDIT);
}

/// Construct.
BaseAttachableSurface::BaseAttachableSurface(Context* context)
  : Component(context)
{
}

/// Convert local coordintaes to topology coordinates. 
/// With optional snapping, in this case position and normal will be adjusted
SharedPtr<BaseTopologyAttachment> BaseAttachableSurface::local_to_topology(
  Vector3& position,
  Vector3& normal,
  Vector3& tangent,
  int snap_to
)
{
  // TODO:
  // If snapping - trace in direction of normal
  // Find proper sub object type
  // Calculate position, normal and tangent, based on component (and main axis)

  // Second stage
  // convert position to "topology independent"

  // create and return attachment based on calculated coordinates.

  return nullptr;
}

/// Convert topology position to local position
bool BaseAttachableSurface::topology_to_local(
  const BaseTopologyAttachment& topology_position,
  Vector3& position,
  Vector3& normal,
  Vector3& tangent
)
{
  // TODO:
  // convert "topology independent" position into local.

  // If snapped, find closest attachment subobject, using normal and tangent as a guide
  // Calculate position, normal and tangent, based on component (and main axis)

  // return true, if topology position is valid in current topology, 
  // return false, if position is no longer possible and should be rejected/deleted

  return true;
}
