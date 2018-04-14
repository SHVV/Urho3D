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
  : BasePositioner(context),
    m_angles(Vector3::ZERO)
{
}

/// Set position externally.
void SurfaceNodePositioner::set_position(
  const Vector3& position,
  const Vector3& normal,
  const Quaternion& rotation
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
      m_angles = calculate_angles(rotation, m_attachment->snapped_to());
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

    // Preserve original normal
    if (m_attachment) {
      normal = m_attachment->normal();
    }

    m_attachment = surface->local_to_topology(
      position,
      normal,
      tangent,
      (int)SubObjectType::POLYGON | (int)SubObjectType::EDGE,
      false
    );
    if (m_attachment) {
      // Calculate angles between current position and snapped to ajust them
      Vector3 binormal = normal.CrossProduct(tangent);
      Quaternion base_rotation(tangent, binormal, normal);
      Quaternion base_rotation_inverse = base_rotation.Inverse();
      Quaternion additional_rotation = base_rotation_inverse * rotation;
      m_angles = calculate_angles(additional_rotation, m_attachment->snapped_to());

      update_node_position();
    }
  }
}

/// Convert rotation to angles, and limit them.
Vector3 SurfaceNodePositioner::calculate_angles(
  const Quaternion& rotation,
  SubObjectType snapped_to
)
{
  Vector3 angles = rotation.EulerAngles();
  // Reset euler angles for specific coordinates
  switch (snapped_to) {
    case SubObjectType::POLYGON:
      angles.x_ = 0;
      // Fall-through
    case SubObjectType::EDGE:
      angles.y_ = 0;
  }

  // Limit Euler angles
  angles.x_ = Clamp(angles.x_, -45.0f, 45.0f);
  angles.y_ = Clamp(angles.y_, -45.0f, 45.0f);

  // Quantize angles
  float snap_step = 3;
  angles.x_ = round(angles.x_ / snap_step) * snap_step;
  angles.y_ = round(angles.y_ / snap_step) * snap_step;
  angles.z_ = round(angles.z_ / snap_step) * snap_step;
  return angles;
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
    node->SetPosition(position);

    Vector3 binormal = normal.CrossProduct(tangent);
    Quaternion rotation(tangent, binormal, normal);
    // Apply local angles
    node->SetRotation(rotation * Quaternion(m_angles.x_, m_angles.y_, m_angles.z_));
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

///// Returns move space for moving this node
//MoveSpace SurfaceNodePositioner::move_space()
//{
//  return MoveSpace::LOCAL;
//}

/// Calculate and return gizmo orientation in world coordinates
void SurfaceNodePositioner::axis(
  Vector3& axis_x,
  Vector3& axis_y,
  Vector3& axis_z
)
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
    Quaternion rotation(tangent, binormal, normal);
    // Ignore Z axis rotation
    rotation = rotation * Quaternion(m_angles.x_, m_angles.y_, 0);
    rotation = node->GetParent()->GetWorldRotation() * rotation;
    axis_x = rotation * Vector3(1, 0, 0);
    axis_y = rotation * Vector3(0, 1, 0);
    axis_z = rotation * Vector3(0, 0, 1);
  } else {
    axis_x = Vector3(1, 0, 0);
    axis_y = Vector3(0, 1, 0);
    axis_z = Vector3(0, 0, 1);
  }
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
