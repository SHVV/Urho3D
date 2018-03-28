// SHW Spacecraft editor
//
// Surface-Surface attachment. Used to adjust position relative to parent

#include "SurfaceSurfacePositioner.h"

#include <Urho3D\Core\Context.h>
#include <Urho3D\Scene\Node.h>

using namespace Urho3D;

extern const char* EDITOR_CATEGORY;


/// Register object attributes.
void SurfaceSurfacePositioner::RegisterObject(Context* context)
{
  context->RegisterFactory<SurfaceSurfacePositioner>(EDITOR_CATEGORY);

  // TODO: add Attributes
  //URHO3D_ENUM_ATTRIBUTE("State", m_state, state_names, usNONE, AM_NET | AM_NOEDIT);
}

/// Construct.
SurfaceSurfacePositioner::SurfaceSurfacePositioner(Context* context)
  : BasePositioner(context)
{
}

/// Updates internal position representation, based on current node position.
void SurfaceSurfacePositioner::update_internal_position()
{
  // TODO:
}

/// Updates node position, based on reference and internal position.
void SurfaceSurfacePositioner::update_node_position()
{
  // TODO:
}

/*/// Handle node (not only our) transform being dirtied.
void SurfaceSurfacePositioner::OnMarkedDirty(Node* node)
{
update_links();
}*/
