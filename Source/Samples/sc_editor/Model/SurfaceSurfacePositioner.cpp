// SHW Spacecraft editor
//
// Surface-Surface attachment. Used to adjust position relative to parent

#include "SurfaceSurfacePositioner.h"
#include "SurfaceSurfaceAutoLinkUnit.h"
#include "../Core/MathUtils.h"

#include <Urho3D\Core\Context.h>
#include <Urho3D\Scene\Node.h>

using namespace Urho3D;

extern const char* EDITOR_CATEGORY;
static const char* AUTO_LINK_NAME = "AutoLink";
static const StringHash AUTO_LINK_NAME_HASH = AUTO_LINK_NAME;

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

/// Destructor
SurfaceSurfacePositioner::~SurfaceSurfacePositioner()
{
  if (!m_auto_link.Expired()) {
    m_auto_link->GetNode()->Remove();
  }
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

    Vector3 position_temp = position;
    Vector3 normal_temp = normal;
    Vector3 tangent = Vector3::RIGHT;
    m_attachment = attached_surface->local_to_topology(
      position_temp,
      normal_temp,
      tangent,
      (int)SubObjectType::POLYGON | (int)SubObjectType::EDGE | (int)SubObjectType::VERTEX,
      mgfATTACHABLE | mgfSNAPPABLE
    );

    // Set orientation right here
    if (m_attachment) {
      tangent = MathUtils::ortogonal(normal_temp);
      Vector3 binormal = normal_temp.CrossProduct(tangent);
      Quaternion base_rotation(normal_temp, -binormal, tangent);
      // Apply local rotation
      Quaternion local_rotation = base_rotation * rotation;
      node->SetRotation(local_rotation);
      m_distance =
        attached_surface->average_attachable_edge() +
        our_surface->average_attachable_edge();
      m_distance /= 2;

      SurfaceSurfaceAutoLinkUnit* auto_link_unit = get_auto_link(true);
      if (auto_link_unit) {
        auto_link_unit->set_nodes(node, node->GetParent());
        auto_link_unit->GetNode()->SetEnabled(true);
      }

      update_node_position();
      return true;
    }
  }

  SurfaceSurfaceAutoLinkUnit* auto_link_unit = get_auto_link(false);
  if (auto_link_unit) {
    auto_link_unit->GetNode()->SetEnabled(false);
  }
  // On update +
  // - calculate farthest point on our surface using normal as reference direction
  // - calculate distance between feature and attachment position
  // - quantize distance based on smallest surface feature
  return false;
}

/// Updates internal position representation, based on current node position.
void SurfaceSurfacePositioner::update_internal_position()
{
  Node* node = GetNode();
  BaseAttachableSurface* our_surface = get_our_surface();
  BaseAttachableSurface* attached_surface = get_attached_surface();
  if (node && our_surface && attached_surface) {
    auto position = node->GetPosition();
    auto rotation = node->GetRotation();
    auto normal = rotation * Vector3::FORWARD;
    auto tangent = rotation * Vector3::RIGHT;

    // Preserve original normal
    if (m_attachment) {
      normal = m_attachment->normal();
    }

    m_attachment = attached_surface->local_to_topology(
      position,
      normal,
      tangent,
      (int)SubObjectType::POLYGON | (int)SubObjectType::EDGE | (int)SubObjectType::VERTEX,
      mgfATTACHABLE | mgfSNAPPABLE
    );
    if (m_attachment) {
      // TODO: deal with angles

      // Update distance
      float reference_distance = get_reference_distance(normal);
      float full_distance = normal.DotProduct(node->GetPosition() - position);
      m_distance = full_distance - reference_distance;
      update_node_position();
    }
  }
}

/// Updates node position, based on reference and internal position.
void SurfaceSurfacePositioner::update_node_position()
{
  BaseAttachableSurface* attached_surface = get_attached_surface();
  BaseAttachableSurface* our_surface = get_our_surface();
  Node* node = GetNode();
  if (attached_surface && node && m_attachment && our_surface) {
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
      float reference_distance = get_reference_distance(normal);
      // Apply distance
      node->SetPosition(position + normal * (m_distance + reference_distance));
    } else {

    }
  }
}

/// Calculate reference distance
float SurfaceSurfacePositioner::get_reference_distance(const Vector3& normal)
{
  BaseAttachableSurface* our_surface = get_our_surface();
  Node* node = GetNode();

  Quaternion inverted_rotation = node->GetRotation().Inverse();
  Vector3 local_attachment_direction = inverted_rotation * normal;
  Vector3 reference_position;
  Vector3 reference_normal;
  return our_surface->farthest_vertex(
    local_attachment_direction,
    reference_position,
    reference_normal
  );
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

/// Get or create mount adapter
SurfaceSurfaceAutoLinkUnit* SurfaceSurfacePositioner::get_auto_link(bool create)
{
  if (m_auto_link.Expired()) {
    Node* node = GetNode();
    if (node) {
      Node* parent_node = node->GetParent();
      if (parent_node) {
        Node* auto_link = node->GetChild(AUTO_LINK_NAME_HASH);
        if (!auto_link) {
          if (create) {
            auto_link = node->CreateChild(AUTO_LINK_NAME);
            m_auto_link = auto_link->CreateComponent<SurfaceSurfaceAutoLinkUnit>();
          }
        } else {
          m_auto_link = auto_link->GetComponent<SurfaceSurfaceAutoLinkUnit>();
        }
      }
    }
  }
  return m_auto_link.Get();
}
