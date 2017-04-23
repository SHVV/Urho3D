// SHW Spacecraft editor
//
// Unit Model class, represents basic class for every units in constructor

#include "UnitModel.h"

#include "../Core/MeshGeometry.h"
#include "../Core/MeshBuffer.h"
#include "../Core/MeshGenerator.h"

#include "SceneModel.h"
#include "DynamicModel.h"

#include "../MeshGenerators/TestGenerator.h"

#include <Urho3D\Core\Context.h>
#include <Urho3D\Scene\Node.h>
#include <Urho3D\Graphics\StaticModel.h>

using namespace Urho3D;

extern const char* EDITOR_CATEGORY;

//static const char* state_names[] =
//{
//  "None",
//  "Highlighted",
//  "Selected",
//  "Hidden",
//  0
//};

/// Register object attributes.
void UnitModel::RegisterObject(Context* context)
{
  context->RegisterFactory<UnitModel>(EDITOR_CATEGORY);

  // TODO: add Attributes
  //URHO3D_ENUM_ATTRIBUTE("State", m_state, state_names, usNONE, AM_NET | AM_NOEDIT);
}

/// Construct.
UnitModel::UnitModel(Context* context)
  : Component(context)
{
}

/// Destructor
UnitModel::~UnitModel()
{
}

///// Get scene model
//SceneModel* UnitModel::scene_model()
//{
//  // TODO: decide how to get scene this
//  return SceneModel::get();
//}

// Parameters management
/// Get all parameters of the component
const Parameters& UnitModel::parameters() const
{
  return m_parameters;
}

/// Set all parameters in one go
void UnitModel::set_parameters(const Parameters& parameters)
{
  m_parameters = parameters;
  apply_parameters();
}

/// Set one parameter by index
void UnitModel::set_parameter(int index, const Variant& parameter)
{
  // Don't do anything, if parameter was not changed
  if (parameter != m_parameters.access_parameters_vector()[index]) {
    m_parameters.access_parameters_vector()[index] = parameter;
    apply_parameters(index);
  }
}

// Components tracking
/// Gets or creates component by its type and automatically incremented index.
Component* UnitModel::get_component(StringHash type, CreateMode mode)
{
  Node* node = GetNode();
  assert(node);
  Component* component = nullptr;
  for (int i = 0; i < m_tracked_components.Size(); ++i) {
    component = m_tracked_components[i];
    if (component->GetType() == type) {
      // If component does not exist in update list
      if (m_update_components.Find(component) == m_update_components.End()) {
        // Return it
        m_update_components.Push(component);
        return component;
      }
    }
  }
  // Looks like we have not found component in tracked list -> create it
  component = node->CreateComponent(type, mode);
  m_update_components.Push(component);

  return component;
}

/// Called on setting parameters
void UnitModel::apply_parameters(int index)
{
  // By default update guts
  update_guts();
  // And notify, that attributes were changed
  notify_attribute_changed();
}

/// Create or update all necessary components
void UnitModel::update_guts()
{
  start_updating();
  update_guts_int();
  finish_updating();
}

/// Create or update all necessary components - override for derived classes
void UnitModel::update_guts_int()
{
}

/// Starts updating all guts. Resets tracking
void UnitModel::start_updating()
{
  m_update_components.Clear();
}

/// Finishes updating process. Removes all untouched tracked components
void UnitModel::finish_updating()
{
  Node* node = GetNode();
  for (int i = 0; i < m_tracked_components.Size(); ++i) {
    // If tracked component is not in new list -> remove it
    if (m_update_components.Find(m_tracked_components[i]) == m_update_components.End()) {
      node->RemoveComponent(m_tracked_components[i]);
    }
  }
  m_tracked_components = m_update_components;
  m_update_components.Clear();
}

/*/// Update all links due to movements
void UnitModel::update_links()
{
  // TODO: update all existing links
}*/

/// Handle node being assigned.
void UnitModel::OnNodeSet(Node* node)
{
  if (node) {
    update_guts();
  }
}

/*/// Handle node (not only our) transform being dirtied.
void UnitModel::OnMarkedDirty(Node* node)
{
  update_links();
}*/

///// Set unit state
//void UnitModel::set_state(State value)
//{
//  if (m_state != value) {
//    m_state = value;
//    notify_attribute_changed();
//  }
//}
//
///// Get unit state
//UnitModel::State UnitModel::state()
//{
//  return m_state;
//}


/// Notify all subscribers, that attribute of model has been changed
void UnitModel::notify_attribute_changed()
{
  // TODO: test this
  using namespace ModelAttributeChanged;

  VariantMap& eventData = GetEventDataMap();
  eventData[P_COMP] = this;
  // TODO: add attribute's categories

  SendEvent(E_MODEL_ATTRIBUTE_CHANGED, eventData);
  MarkNetworkUpdate();
}
