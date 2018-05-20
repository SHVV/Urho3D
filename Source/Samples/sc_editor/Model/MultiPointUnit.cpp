// SHW Spacecraft editor
//
// Basic procedural unit. Contains link to function for generating guts.
// Default implementation - generate procedural geometry model

#include "MultiPointUnit.h"

#include "../Core/MeshGeometry.h"
#include "../Core/MeshBuffer.h"
#include "../Core/MeshGenerator.h"
#include "../Core/MeshGenerationFunction.h"

#include "DynamicModel.h"

#include <Urho3D\Core\Context.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D\Scene\SceneEvents.h>

using namespace Urho3D;

extern const char* EDITOR_CATEGORY;

/// Register object attributes.
void MultiPointUnit::RegisterObject(Context* context)
{
  context->RegisterFactory<MultiPointUnit>(EDITOR_CATEGORY);

  URHO3D_COPY_BASE_ATTRIBUTES(ProceduralUnit);
}

/// Construct.
MultiPointUnit::MultiPointUnit(Context* context)
  : ProceduralUnit(context)
{
}

/// Destructor
MultiPointUnit::~MultiPointUnit()
{
  UnsubscribeFromAllEvents();
  for (int i = 0; i < m_reference_nodes.Size(); ++i) {
    if (!m_reference_nodes[i].Expired()) {
      m_reference_nodes[i]->RemoveListener(this);
      m_reference_nodes[i]->Remove();
    }
  }
}

/// Sets i'th reference node
/// \return true, if acceps more nodes
bool MultiPointUnit::set_reference_node(int i, Node* node)
{
  assert(i < 2);

  if (i >= m_reference_nodes.Size()) {
    m_reference_nodes.Resize(i + 1);
  }
  if (!m_reference_nodes[i].Expired()) {
    m_reference_nodes[i]->RemoveListener(this);
  }
  m_reference_nodes[i] = node;
  node->AddListener(this);

  // For now use just 2 reference points, so on second node - return false
  return i < 1;
}

/// Gets i'th reference node, if it exists
Node* MultiPointUnit::get_reference_node(int i)
{
  if (i >= 0 && i < m_reference_nodes.Size()) {
    return m_reference_nodes[i];
  } else {
    return nullptr;
  }
}

/// Handle node being assigned.
void MultiPointUnit::OnNodeSet(Node* node)
{
  if (node) {
    //SubscribeToEvent(
    //  GetScene(),
    //  E_NODEREMOVED,
    //  URHO3D_HANDLER(MultiPointUnit, on_node_remove)
    //);
  }
}

/// Handle scene node transform dirtied.
void MultiPointUnit::OnMarkedDirty(Node* node)
{
  if (node != GetNode()) {
    // Check nodes
    for (int i = 0; i < m_reference_nodes.Size(); ++i) {
      if (m_reference_nodes[i].Expired() || !m_reference_nodes[i]->GetParent()) {
        SubscribeToEvent(
          GetScene(),
          E_SCENEPOSTUPDATE,
          URHO3D_HANDLER(MultiPointUnit, on_node_remove)
        );
        //GetNode()->Remove();
        return;
      }
    }
    update_guts();
  }
}

/// On node remove event
void MultiPointUnit::on_node_remove(StringHash eventType, VariantMap& eventData)
{
  GetNode()->Remove();
  if (E_NODEREMOVED == eventType) {
    using namespace NodeRemoved;
    for (int i = 0; i < m_reference_nodes.Size(); ++i) {
      if (eventData[P_NODE].GetPtr() == m_reference_nodes[i].Get()) {
        GetNode()->Remove();
        return;
      }
    }
  }
}

/// Create or update all necessary components - override for derived classes
void MultiPointUnit::update_guts_int()
{
  // Set reference nodes list in parameters
  // Assuming, first parameter should be nodes list
  // TODO: better searching
  auto& parameter_ids = parameters_description().parameter_ids();
  if (parameter_ids.Size() > 0 &&
      parameters()[parameter_ids[0]].GetType() == VariantType::VAR_VARIANTVECTOR) {
    VariantVector reference_node_ids;
    for (int i = 0; i < m_reference_nodes.Size(); ++i) {
      reference_node_ids.Push(m_reference_nodes[i]->GetID());
    }
    set_parameter(parameter_ids[0], reference_node_ids);
  }

  // Update node position based on reference nodes list
  if (m_reference_nodes.Size() > 0) {
    Vector3 position = Vector3::ZERO;
    for (int i = 0; i < m_reference_nodes.Size(); ++i) {
      position += m_reference_nodes[i]->GetWorldPosition();
    }
    position /= m_reference_nodes.Size();
    GetNode()->SetWorldPosition(position);

    // If more than 1 point, calculate primary direction from boundary nodes
    Vector3 normal;
    if (m_reference_nodes.Size() > 1) {
      normal =
        m_reference_nodes[m_reference_nodes.Size() - 1]->GetWorldPosition() -
        m_reference_nodes[0]->GetWorldPosition();
      normal.Normalize();
    } else {
      normal = m_reference_nodes[0]->GetWorldDirection();
    }
    Vector3 secondary_vector = position;
    secondary_vector.z_ = 0;
    secondary_vector.Normalize();
    // TODO: add checks for null vectors

    Vector3 axis_z = normal;
    Vector3 axis_y = secondary_vector;
    Vector3 axis_x = axis_y.CrossProduct(axis_z);
    if (axis_x.Equals(Vector3::ZERO)) {
      axis_y = Vector3(0, 0, 1);
      axis_x = axis_y.CrossProduct(axis_z);
    }
    axis_x.Normalize();
    axis_y = axis_z.CrossProduct(axis_x).Normalized();
    GetNode()->SetWorldRotation(Quaternion(axis_x, axis_y, axis_z));
  }

  ProceduralUnit::update_guts_int();
}
