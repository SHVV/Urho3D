// SHW Spacecraft editor
//
// Surface-Surface attachment. Used to adjust position relative to parent

#include "SurfaceNodePositioner.h"

#include <Urho3D\Core\Context.h>
#include <Urho3D\Scene\Node.h>

using namespace Urho3D;

extern const char* EDITOR_CATEGORY;


/// Register object attributes.
void SurfaceNodePositioner::RegisterObject(Context* context)
{
  context->RegisterFactory<SurfaceNodePositioner>(EDITOR_CATEGORY);

  // TODO: add Attributes
  //URHO3D_ENUM_ATTRIBUTE("State", m_state, state_names, usNONE, AM_NET | AM_NOEDIT);
}

/// Construct.
SurfaceNodePositioner::SurfaceNodePositioner(Context* context)
  : BasePositioner(context)
{
}

/// Set position externally.
void SurfaceNodePositioner::set_position(
  const Vector3& position,
  const Vector3& normal
)
{
  if (m_surface) {
    m_surface.Reset();
  }
  BaseAttachableSurface* surface = get_surface();
  Node* node = GetNode();
  if (surface && node) {
    Vector3 position_temp = position;
    Vector3 normal_temp = normal;
    Vector3 tangent = Vector3::RIGHT;
    m_attachment = surface->local_to_topology(
      position_temp,
      normal_temp,
      tangent,
      (int)SubObjectType::POLYGON | (int)SubObjectType::EDGE,
      false
    );

    if (m_attachment) {
      update_node_position();
    }
  }
}

/// Updates internal position representation, based on current node position.
void SurfaceNodePositioner::update_internal_position()
{
  BaseAttachableSurface* surface = get_surface();
  Node* node = GetNode();
  if (surface && node) {
    auto position = node->GetPosition();
    auto rotation = node->GetRotation();
    auto normal = rotation * Vector3::FORWARD;
    auto tangent = rotation * Vector3::RIGHT;

    m_attachment = surface->local_to_topology(
      position,
      normal,
      tangent,
      (int)SubObjectType::POLYGON | (int)SubObjectType::EDGE,
      false
    );
    // TODO: calculate angles between current position and snapped to ajust them
    if (m_attachment) {
      update_node_position();
    }
  }
}

/// Updates node position, based on reference and internal position.
void SurfaceNodePositioner::update_node_position()
{
  BaseAttachableSurface* surface = get_surface();
  Node* node = GetNode();
  if (surface && node && m_attachment) {
    Vector3 position;
    Vector3 normal;
    Vector3 tangent;

    surface->topology_to_local(
      *m_attachment,
      position,
      normal,
      tangent
    );
    Vector3 binormal = normal.CrossProduct(tangent);

    // TODO: apply local angles
    Quaternion rotation(tangent, binormal, normal);
    node->SetPosition(position);
    node->SetRotation(rotation);
  }
}

/// Get supported moves along local linear axis
Axis SurfaceNodePositioner::linear_axis()
{
  return Axis::NONE;
}

/// Get supported rotations around local linear axis
Axis SurfaceNodePositioner::rotation_axis()
{
  if (m_attachment) {
    switch (m_attachment->snapped_to()) {
      case SubObjectType::VERTEX: return Axis::XYZ;
      case SubObjectType::EDGE: return Axis::XZ;
      case SubObjectType::POLYGON: return Axis::Z;
    }
  }
  return Axis::NONE;
}

/// Returns move space for moving this node
MoveSpace SurfaceNodePositioner::move_space()
{
  return MoveSpace::LOCAL;
}

/// Handle node being assigned.
void SurfaceNodePositioner::OnNodeSet(Node* node)
{
  update_internal_position();
}

/// Get surface we attached to.
BaseAttachableSurface* SurfaceNodePositioner::get_surface()
{
  if (m_surface.Expired()) {
    Node* node = GetNode();
    if (node) {
      node = node->GetParent();
      if (node) {
        BaseAttachableSurface* surface =
          node->GetDerivedComponent<BaseAttachableSurface>();
        m_surface = surface;
      }
    }
  }

  return m_surface.Get();
}

/*/// Handle node (not only our) transform being dirtied.
void SurfaceNodePositioner::OnMarkedDirty(Node* node)
{
update_links();
}*/
