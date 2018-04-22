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
  Vector3 normalized_position = local_to_topology(position, normal);
  
  if (!snap_optional && sub_type == SubObjectType::NONE) {
    return nullptr;
  }

  // create and return attachment based on calculated coordinates.
  SubObjectType snap_type = real_snap ? sub_type : SubObjectType::NONE;
  auto result = SharedPtr<BaseTopologyAttachment>(new BaseTopologyAttachment(
    normalized_position,
    normal,
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
      == topology_position.primitives_count()) {
    sub_object_to_local(snap_type, primitive_index, position, normal, tangent);

    return true;
  }

  // convert "topology independent" position into local.
  normal = topology_position.normal();
  position = topology_to_local(topology_position.position(), normal);
  // TODO: calcualte tangent for base convertion too

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

    return sub_type != SubObjectType::NONE;
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
      int real_index = geometry->vertices_by_flags(mgfATTACHABLE)[sub_index];
      auto& vertex = geometry->vertices()[real_index];
      position = vertex.position;
      normal = vertex.normal;
      break;
    }
    case SubObjectType::EDGE: {
      int real_index = geometry->edges_by_flags(mgfATTACHABLE)[sub_index];
      auto& edge = geometry->edges()[real_index];
      position = edge.center(*geometry);
      normal = edge.normal(*geometry);
      tangent = edge.direction(*geometry);

      // Don't need to recalculate tangent, because it is defined already
      return;
    }
    case SubObjectType::POLYGON: {
      int real_index = geometry->polygons_by_flags(mgfATTACHABLE)[sub_index];
      auto& polygon = geometry->polygons()[real_index];
      position = polygon.center(*geometry);
      normal = polygon.normal(*geometry);
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
    float dist = 0.0;
    Ray ray(position + normal * dist, -normal);
    // Find proper sub object type
    //primitive_index = geometry->raycast(ray, snap_type, snap_to, mgfATTACHABLE);
    // If no attachable sub object under position
    //if (primitive_index < 0) {
      // Find just closest one
      primitive_index = geometry->closest(ray, snap_type, snap_to, mgfATTACHABLE);
      // Use real snapping in this case, only if it is not optional
      result = !snap_optional;
    //}

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
  return result;
}

/// Convert local position to topology independent one
Vector3 BaseAttachableSurface::local_to_topology(
  const Vector3& position,
  const Vector3& normal
)
{
  DynamicModel* model = dynamic_model();
  if (model) {
    // By default - just scale by BB
    auto& bb = model->GetBoundingBox();
    return position / bb.HalfSize();
  } else {
    return position;
  }
}

/// Convert position from topology independent to local
Vector3 BaseAttachableSurface::topology_to_local(
  const Vector3& norm_position,
  const Vector3& normal
)
{
  DynamicModel* model = dynamic_model();
  if (model) {
    // By default - just scale by BB
    auto& bb = model->GetBoundingBox();
    return norm_position * bb.HalfSize();
  } else {
    return norm_position;
  }
}

/// Dynamic model component for attaching to.
DynamicModel* BaseAttachableSurface::dynamic_model()
{
  if (m_dynamic_model.Expired()) {
    Node* node = GetNode();
    if (node) {
      m_dynamic_model = node->GetComponent<DynamicModel>();

      // Subscribe for dynamic object changes
      SubscribeToEvent(
        m_dynamic_model,
        E_DYNAMIC_MODEL_CHANGED,
        URHO3D_HANDLER(BaseAttachableSurface, on_changed)
      );
    }
  }
  return m_dynamic_model.Get();
}

/// Event handler on dynamic model change
void BaseAttachableSurface::on_changed(
  StringHash eventType,
  VariantMap& eventData
)
{
  // TODO: possible filtering out unnecessary updates
  notify_changed();
}

/// Send notification on changed
void BaseAttachableSurface::notify_changed()
{
  // Notify subscribers
  using namespace AttachableSurfaceChanged;

  VariantMap& event_data = GetEventDataMap();
  event_data[P_COMP] = this;

  SendEvent(E_ATTACHABLE_SURFACE_CHANGED, event_data);
}
