// SHW Spacecraft editor
//
// Unit Model class, represents basic class for every units in constructor

#include "UnitModel.h"

#include <Urho3D\Core\Context.h>
#include <Urho3D\Scene\Node.h>

using namespace Urho3D;

extern const char* EDITOR_CATEGORY;

static const char* state_names[] =
{
  "None",
  "Highlighted",
  "Selected",
  "Hidden",
  0
};

/// Register object attributes.
void UnitModel::RegisterObject(Context* context)
{
  context->RegisterFactory<UnitModel>(EDITOR_CATEGORY);

  // TODO: add Attributes
  URHO3D_ENUM_ATTRIBUTE("State", m_state, state_names, usNONE, AM_NET | AM_NOEDIT);
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

/// Set unit state
void UnitModel::set_state(State value)
{
  if (m_state != value) {
    m_state = value;
    notify_attribute_changed();
  }
}

/// Get unit state
UnitModel::State UnitModel::state()
{
  return m_state;
}

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
