// SHW Spacecraft editor
//
// "RadioGrouop" class for handling set of button switches as single object.

#include "RadioGroup.h"

// Includes from Editor
#include "EditorUI.h"

// Includes from Urho3D
#include "Urho3D/Core/Context.h"
#include "Urho3D/UI/CheckBox.h"
#include <Urho3D/UI/UIEvents.h>
#include "Urho3D/UI/UI.h"
#include "Urho3D/Graphics/Graphics.h"

static String s_base_group_style = "ToolBarToggle";
static StringHash s_radio_button_value = "RadioButtonValue";

/// Construct.
RadioGroup::RadioGroup(Context* context, EditorUI* editor, UIElement* parent)
: Object(context),
  m_editor_ui(editor),
  m_dirty(true),
  m_state(-1)
{
  // Create group
  m_parent = new UIElement(context_);
  //m_parent->SetName("SymmetryToolbar");

  parent->AddChild(m_parent);

  m_parent->SetLayout(LM_HORIZONTAL);
  m_parent->SetLayoutSpacing(0);
}

/// Destructor
RadioGroup::~RadioGroup()
{
}

/// Update dirty toolbar
void RadioGroup::update(float dt)
{
  if (m_dirty) {
    for (int i = 0; i < m_buttons.Size(); ++i) {
      CheckBox* btn = m_buttons[i];
      int btn_state = btn->GetVar(s_radio_button_value).GetUInt();
      if ((btn_state == m_state) != btn->IsChecked()) {
        btn->SetChecked(btn_state == m_state);
      }
    }

    m_dirty = false;
  }
}

/// Add switch
void RadioGroup::add_switch(const String& icon, const String& tooltip, int value)
{
  CheckBox* btn = m_editor_ui->create_toolbar_checkbox(icon, tooltip);
  btn->SetVar(s_radio_button_value, value);
  SubscribeToEvent(btn, E_TOGGLED, URHO3D_HANDLER(RadioGroup, on_button));
  m_buttons.Push(btn);
  m_parent->AddChild(btn);

  // Update styles
  for (int i = 0; i < m_buttons.Size(); ++i) {
    CheckBox* btn = m_buttons[i];
    if (m_buttons.Size() == 1) {
      btn->SetStyle(s_base_group_style);
    } else {
      if (i == 0) {
        btn->SetStyle(s_base_group_style + "GroupLeft");
      } else if (i == m_buttons.Size() - 1) {
        btn->SetStyle(s_base_group_style + "GroupRight");
      } else {
        btn->SetStyle(s_base_group_style + "GroupMiddle");
      }
    }
  }
}

/// Switch state
void RadioGroup::switch_state(int value)
{
  if (m_state != value) {
    m_state = value;
    m_dirty = true;
  }
}

int RadioGroup::switch_state() const
{
  return m_state;
}

/// On button pressed handler. Switches context
void RadioGroup::on_button(StringHash event_type, VariantMap& event_data)
{
  CheckBox* clicked =
    static_cast<CheckBox*>(event_data[UIMouseClick::P_ELEMENT].GetPtr());
  if (clicked) {
    int new_state = clicked->GetVar(s_radio_button_value).GetUInt();
    bool changed = false;
    if (clicked->IsChecked()) {
      if (new_state != m_state) {
        m_state = new_state;
        changed = true;
      }
    } else {
      if (new_state == m_state) {
        m_state = -1; // Reset to default state
        changed = true;
      }
    }
    if (changed) {
      m_dirty = true;
      using namespace RadioGroupSwitched;

      VariantMap& eventData = GetEventDataMap();
      eventData[P_RADIO_GROUP] = this;

      SendEvent(E_RADIO_GROUP_SWITCHED, eventData);
    }
  }
}
