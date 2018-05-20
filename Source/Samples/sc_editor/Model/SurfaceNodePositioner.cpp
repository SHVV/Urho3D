// SHW Spacecraft editor
//
// Surface-Surface attachment. Used to adjust position relative to parent

#include "SurfaceNodePositioner.h"

#include "DynamicModel.h"
#include "ProceduralUnit.h"
#include "../MeshGenerators/BeamAdapter.h"

#include <Urho3D\Core\Context.h>
#include <Urho3D\Scene\Node.h>

using namespace Urho3D;

extern const char* EDITOR_CATEGORY;
static const char* MOUNT_ADAPTER_NAME = "MountAdapter";
static const StringHash MOUNT_ADAPTER_NAME_HASH = MOUNT_ADAPTER_NAME;


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
bool SurfaceNodePositioner::set_position(
  const Vector3& position,
  const Vector3& normal,
  const Quaternion& rotation
)
{
  if (m_surface) {
    m_surface.Reset();
    UnsubscribeFromAllEvents();
  }
  BaseAttachableSurface* surface = get_surface();
  Node* node = GetNode();
  if (surface && node) {
    // Subscribe for attachable surface changes
    SubscribeToEvent(
      m_surface,
      E_ATTACHABLE_SURFACE_CHANGED,
      URHO3D_HANDLER(SurfaceNodePositioner, on_surface_changed)
    );

    // Subscribe for surface mount changes
    SurfaceMount* surface_mount = get_surface_mount();
    if (surface_mount) {
      SubscribeToEvent(
        surface_mount,
        E_SURFACE_MOUNT_CHANGED,
        URHO3D_HANDLER(SurfaceNodePositioner, on_mount_changed)
      );
    }

    Vector3 position_temp = position;
    Vector3 normal_temp = normal;
    Vector3 tangent = Vector3::RIGHT;
    m_attachment = surface->local_to_topology(
      position_temp,
      normal_temp,
      tangent,
      m_sub_objects
    );

    if (m_attachment) {
      m_angles = calculate_angles(rotation, m_attachment->snapped_to());
      update_node_position();

      return true;
    }
  }

  return false;
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
      m_sub_objects
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

/// Calculate shift, based on attachment (and may be size)
float SurfaceNodePositioner::calculate_shift()
{
  BaseAttachableSurface* surface = get_surface();
  Node* node = GetNode();
  if (!(surface || node || m_attachment))  {
    return 0;
  }
  DynamicModel* model = surface->dynamic_model();
  if (!model) {
    return 0;
  }
  const MeshGeometry* geometry = model->mesh_geometry();
  if (!geometry) {
    return 0;
  }
  SubObjectType snapped_to = m_attachment->snapped_to();
  int filtered_index = m_attachment->primitive_index();
  const float mult = 1.5f;
  switch (snapped_to) {
    case SubObjectType::EDGE: {
      int index = geometry->edges_by_flags(mgfATTACHABLE)[filtered_index];
      MeshGeometry::Edge edge = geometry->edges()[index];
      return edge.radius(*geometry) * mult;
    }
    case SubObjectType::POLYGON: {
      int index = geometry->polygons_by_flags(mgfATTACHABLE)[filtered_index];
      MeshGeometry::Polygon polygon = geometry->polygons()[index];
      if (polygon.flags & mgfVISIBLE) {
        return 0;
      } else {
        return polygon.radius(*geometry) * mult;
      }
    }
  }

  return 0;
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

    bool valid = surface->topology_to_local(
      *m_attachment,
      position,
      normal,
      tangent
    );

    if (valid) {
      Vector3 binormal = normal.CrossProduct(tangent);
      Quaternion base_rotation(tangent, binormal, normal);
      // Apply local angles
      Quaternion local_rotation = 
        base_rotation * Quaternion(m_angles.x_, m_angles.y_, m_angles.z_);
      node->SetRotation(local_rotation);

      // Apply shift
      Vector3 axis_z = local_rotation * Vector3(0, 0, 1);
      float shift = calculate_shift();
      node->SetPosition(position + axis_z * shift);

      update_adapter();
    } else {
      node->Remove();
    }
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
  Vector3& pos,
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

    // Unapply shift
    float shift = calculate_shift();
    pos = node->GetWorldPosition() - axis_z * shift;
  } else {
    pos = Vector3(0, 0, 0);
    axis_x = Vector3(1, 0, 0);
    axis_y = Vector3(0, 1, 0);
    axis_z = Vector3(0, 0, 1);
  }
}

/// Set allowed sub-objects
void SurfaceNodePositioner::set_sub_objects(int value)
{
  m_sub_objects = value;
}

/// Get average edge size
float SurfaceNodePositioner::average_attachable_edge()
{
  BaseAttachableSurface* surface = get_surface();
  if (surface) {
    return surface->average_attachable_edge();
  }
  return 0;
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
        auto surface = node->GetDerivedComponent<BaseAttachableSurface>();
        m_surface = surface;
      }
    }
  }

  return m_surface.Get();
}

/// Get surface mount
SurfaceMount* SurfaceNodePositioner::get_surface_mount()
{
  Node* node = GetNode();
  if (node) {
    return node->GetDerivedComponent<SurfaceMount>();
  }
}

/// Get or create mount adapter
ProceduralUnit* SurfaceNodePositioner::get_mount_adapter(bool create)
{
  Node* node = GetNode();
  ProceduralUnit* result = nullptr;
  if (node) {
    Node* mount_adapter_node = node->GetChild(MOUNT_ADAPTER_NAME_HASH);
    if (!mount_adapter_node) {
      if (create) {
        mount_adapter_node = node->CreateChild(MOUNT_ADAPTER_NAME);
        result = mount_adapter_node->CreateComponent<ProceduralUnit>();
      }
    } else {
      result = mount_adapter_node->GetComponent<ProceduralUnit>();
    }
  }
  return result;
}

/// Update mount adapter, if nedded
void SurfaceNodePositioner::update_adapter()
{
  BaseAttachableSurface* surface = get_surface();
  Node* node = GetNode();
  if (!(surface || node || m_attachment))  {
    return;
  }
  DynamicModel* model = surface->dynamic_model();
  if (!model) {
    return;
  }
  const MeshGeometry* geometry = model->mesh_geometry();
  if (!geometry) {
    return;
  }
  SurfaceMount* surface_mount = get_surface_mount();
  if (!surface_mount) {
    return;
  }
  SubObjectType snapped_to = m_attachment->snapped_to();
  int filtered_index = m_attachment->primitive_index();
  switch (snapped_to) {
    case SubObjectType::EDGE: {
      int index = geometry->edges_by_flags(mgfATTACHABLE)[filtered_index];
      MeshGeometry::Edge edge = geometry->edges()[index];
      ProceduralUnit* mount_adapter = get_mount_adapter();
      mount_adapter->set_function_name(BeamAdapter::s_name);
      Parameters parameters = mount_adapter->parameters();
      parameters[BeamAdapter::s_beam_radius] = edge.radius(*geometry);
      parameters[BeamAdapter::s_plate_size] = surface_mount->mount_size();
      parameters[BeamAdapter::s_shift] = calculate_shift();
      mount_adapter->set_parameters(parameters);
      // Compensate Z rotation
      mount_adapter->GetNode()->SetRotation(Quaternion(0, 0, -m_angles.z_));
      mount_adapter->GetNode()->SetEnabled(true);
      return;
    }
    case SubObjectType::POLYGON: {
      int index = geometry->polygons_by_flags(mgfATTACHABLE)[filtered_index];
      MeshGeometry::Polygon polygon = geometry->polygons()[index];
      if (!(polygon.flags & mgfVISIBLE)) {
        ProceduralUnit* mount_adapter = get_mount_adapter();
        mount_adapter->set_function_name(BeamAdapter::s_name);
        Parameters parameters = mount_adapter->parameters();
        parameters[BeamAdapter::s_beam_radius] = 0;
        parameters[BeamAdapter::s_plate_size] = surface_mount->mount_size();
        parameters[BeamAdapter::s_shift] = calculate_shift();
        mount_adapter->set_parameters(parameters);
        // Compensate Z rotation
        mount_adapter->GetNode()->SetRotation(Quaternion(0, 0, -m_angles.z_));
      }
      break;
    }
  }
  ProceduralUnit* mount_adapter = get_mount_adapter(false);
  if (mount_adapter) {
    mount_adapter->GetNode()->SetEnabled(false);
  }
}

/// Event handler on attachable surface change
void SurfaceNodePositioner::on_surface_changed(
  StringHash eventType,
  VariantMap& eventData
)
{
  // TODO: may be we need some checks here
  update_node_position();
}

/// Event handler on surface mount change
void SurfaceNodePositioner::on_mount_changed(
  StringHash eventType,
  VariantMap& eventData
)
{
  // TODO: may be we need some checks here
  update_adapter();
}


/*/// Handle node (not only our) transform being dirtied.
void SurfaceNodePositioner::OnMarkedDirty(Node* node)
{
update_links();
}*/
