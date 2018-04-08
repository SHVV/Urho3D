// SHW Spacecraft editor
//
// Base attachable surface. 
// Used to preserve attachment coordinates during topology changes.

#include "BaseAttachableSurface.h"
#include "../Model/DynamicModel.h"
#include <Urho3D\Core\Context.h>
#include <Urho3D\Scene\Node.h>
#include <Urho3D\Math\Ray.h>

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
  int snap_to,
  bool snap_optional
)
{
  DynamicModel* model = dynamic_model();
  if (!model) {
    return nullptr;
  }
  const MeshGeometry* geometry = model->mesh_geometry();
  if (!geometry) {
    return nullptr;
  }

  SubObjectType sub_type = SubObjectType::NONE;
  int sub_index = -1;

  // If snapping - trace in direction of normal
  bool real_snap = snap_to_primitive(
    position,
    normal,
    tangent,
    snap_to,
    snap_optional,
    sub_type,
    sub_index
  );

  // Second stage
  // convert position to "topology independent"
  // By default - just scale by BB
  auto& bb = model->GetBoundingBox();
  Vector3 normalized_position = position / bb.HalfSize();
  
  // create and return attachment based on calculated coordinates.
  SubObjectType snap_type = real_snap ? sub_type : SubObjectType::NONE;
  auto result = SharedPtr<BaseTopologyAttachment>(new BaseTopologyAttachment(
    normalized_position,
    position,
    snap_type,
    sub_index,
    geometry->primitives_count_by_flags(snap_type, mgfATTACHABLE)
  ));


  return result;
}

/// Convert topology position to local position
bool BaseAttachableSurface::topology_to_local(
  const BaseTopologyAttachment& topology_position,
  Vector3& position,
  Vector3& normal,
  Vector3& tangent
)
{
  DynamicModel* model = dynamic_model();
  if (!model) {
    return false;
  }
  const MeshGeometry* geometry = model->mesh_geometry();
  if (!geometry) {
    return false;
  }

  // First - try to use topology dependent position, by checking number of primitives.
  SubObjectType snap_type = topology_position.snapped_to();
  int primitive_index = topology_position.primitive_index();
  if (snap_type != SubObjectType::NONE &&
      geometry->primitives_count_by_flags(snap_type, mgfATTACHABLE) 
      == primitive_index) {
    sub_object_to_local(snap_type, primitive_index, position, normal, tangent);

    return true;
  }

  // convert "topology independent" position into local.
  // By default - just scale by BB
  auto& bb = model->GetBoundingBox();
  position = topology_position.position() * bb.HalfSize();
  normal = topology_position.normal();

  // If snapped, find closest attachment subobject, using normal and tangent as a guide
  // Calculate position, normal and tangent, based on component (and main axis)
  if (snap_type != SubObjectType::NONE) {
    SubObjectType sub_type = SubObjectType::NONE;
    int sub_index = -1;
    bool real_snap = snap_to_primitive(
      position,
      normal,
      tangent,
      (int)snap_type,
      false,
      sub_type,
      sub_index
    );
  }

  // TODO:
  // return true, if topology position is valid in current topology, 
  // return false, if position is no longer possible and should be rejected/deleted
  return true;
}

/// Convert sub-object into local position
void BaseAttachableSurface::sub_object_to_local(
  SubObjectType sub_type,
  int sub_index,
  Vector3& position,
  Vector3& normal,
  Vector3& tangent
)
{
  DynamicModel* model = dynamic_model();
  if (!model) {
    return;
  }
  const MeshGeometry* geometry = model->mesh_geometry();
  if (!geometry) {
    return;
  }
  if (sub_index < 0) {
    return;
  }

  switch (sub_type) {
    case SubObjectType::VERTEX: {
      auto& vertex = geometry->vertices()[sub_index];
      position = vertex.position;
      normal = vertex.normal;
      break;
    }
    case SubObjectType::EDGE: {
      auto& edge = geometry->edges()[sub_index];
      auto& v1 = geometry->vertices()[edge.vertexes[0]];
      auto& v2 = geometry->vertices()[edge.vertexes[1]];
      position = (v1.position + v2.position) * 0.5f;
      normal = (v1.normal + v2.normal).Normalized();
      tangent = (v2.position - v1.position).Normalized();

      // Don't need to recalculate tangent, because it is defined already
      return;
    }
    case SubObjectType::POLYGON: {
      auto& polygon = geometry->polygons()[sub_index];
      int i = 0;
      position = Vector3::ZERO;
      normal = Vector3::ZERO;
      while (i < 4 && polygon.vertexes[i] >= 0) {
        auto& v = geometry->vertices()[polygon.vertexes[i]];
        position += v.position;
        normal += v.normal;
        ++i;
      }
      position /= i;
      normal.Normalize();
    }
  }

  // TODO: fix this
  Vector3 bitangent = Vector3::FORWARD.CrossProduct(normal);
  if (bitangent.LengthSquared() < M_LARGE_EPSILON) {
    tangent = Vector3::RIGHT;
  } else {
    tangent = normal.CrossProduct(bitangent);
  }
}

bool BaseAttachableSurface::snap_to_primitive(
  Vector3& position,
  Vector3& normal,
  Vector3& tangent,
  int snap_to,
  bool snap_optional,
  SubObjectType& snap_type,
  int& primitive_index
)
{
  DynamicModel* model = dynamic_model();
  if (!model) {
    return false;
  }
  const MeshGeometry* geometry = model->mesh_geometry();
  if (!geometry) {
    return false;
  }

  int result = true;
  if (!!snap_to) {
    float dist = 1.0;
    Ray ray(position + normal * dist, -normal);
    // Find proper sub object type
    primitive_index = geometry->raycast(ray, snap_type, snap_to, mgfATTACHABLE);
    // If no attachable sub object under position
    if (primitive_index < 0) {
      // Find just closest one
      primitive_index = geometry->closest(ray, snap_type, snap_to, mgfATTACHABLE);
      // Use real snapping in this case, only if it is not optional
      result = !snap_optional;
    }

    // Calculate position, normal and tangent, based on component (and main axis)
    Vector3 new_position;
    Vector3 new_normal;
    Vector3 new_tangent;
    sub_object_to_local(snap_type, primitive_index, new_position, new_normal, new_tangent);

    if (!result) {
      // TODO: Snap only Z axis angle
    }
    position = new_position;
    normal = new_normal;
    tangent = new_tangent;
  }
}

/// Dynamic model component for attaching to.
DynamicModel* BaseAttachableSurface::dynamic_model()
{
  if (m_dynamic_model.Expired()) {
    Node* node = GetNode();
    if (node) {
      m_dynamic_model = node->GetComponent<DynamicModel>();
    }
  }
  return m_dynamic_model.Get();
}
