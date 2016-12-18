// SHW Spacecraft editor
//
// Unit Model class, represents basic class for every units in constructor

#include "UnitModel.h"

#include <Urho3D\Core\Context.h>

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
    // TODO: fire event on state changed
  }
}

/// Get unit state
UnitModel::State UnitModel::state()
{
  return m_state;
}
