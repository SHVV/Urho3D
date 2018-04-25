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

/// Set position externally.
bool SurfaceSurfacePositioner::set_position(
  const Vector3& position,
  const Vector3& normal,
  const Quaternion& rotation
)
{
  UnsubscribeFromAllEvents();
  Node* node = GetNode();
  BaseAttachableSurface* our_surface = get_our_surface();
  BaseAttachableSurface* attached_surface = get_attached_surface();
  if (node && our_surface && attached_surface) {
    // Subscribe for both attachable surface changes
    SubscribeToEvent(
      our_surface,
      E_ATTACHABLE_SURFACE_CHANGED,
      URHO3D_HANDLER(SurfaceSurfacePositioner, on_surface_changed)
    );
    SubscribeToEvent(
      attached_surface,
      E_ATTACHABLE_SURFACE_CHANGED,
      URHO3D_HANDLER(SurfaceSurfacePositioner, on_surface_changed)
    );
  }

  // TODO: 
  // - set orientation right here
  //
  // On update +
  // - calculate farthest point on our surface using normal as reference direction
  // - calculate distance between feature and attachment position
  // - quantize distance based on smallest surface feature
  return true;
}

/// Updates internal position representation, based on current node position.
void SurfaceSurfacePositioner::update_internal_position()
{
  // TODO:
}

/// Updates node position, based on reference and internal position.
void SurfaceSurfacePositioner::update_node_position()
{
  BaseAttachableSurface* attached_surface = get_attached_surface();
  Node* node = GetNode();
  if (attached_surface && node && m_attachment) {
    Vector3 position;
    Vector3 normal;
    Vector3 tangent;

    bool valid = attached_surface->topology_to_local(
      *m_attachment,
      position,
      normal,
      tangent
    );

    if (valid) {
      // TODO: think about angles, do we need them?
      //Vector3 binormal = normal.CrossProduct(tangent);
      //Quaternion base_rotation(tangent, binormal, normal);
      //// Apply local angles
      //Quaternion local_rotation = 
      //  base_rotation * Quaternion(m_angles.x_, m_angles.y_, m_angles.z_);
      //node->SetRotation(local_rotation);

      // Apply distance
      node->SetPosition(position + normal * m_distance);
    } else {

    }
  }
}

/// Handle node being assigned.
void SurfaceSurfacePositioner::OnNodeSet(Node* node)
{
  update_internal_position();
}

/// Event handler on attachable surface change
void SurfaceSurfacePositioner::on_surface_changed(
  StringHash eventType,
  VariantMap& eventData
)
{
  // TODO: may be we need some checks here
  update_node_position();
}

/// Get our surface.
BaseAttachableSurface* SurfaceSurfacePositioner::get_our_surface()
{
  Node* node = GetNode();
  if (node) {
    return node->GetDerivedComponent<BaseAttachableSurface>();
  }
}

/// Get surface, we attached to.
BaseAttachableSurface* SurfaceSurfacePositioner::get_attached_surface()
{
  Node* node = GetNode();
  if (node) {
    node = node->GetParent();
    if (node) {
      return node->GetDerivedComponent<BaseAttachableSurface>();
    }
  }
}
