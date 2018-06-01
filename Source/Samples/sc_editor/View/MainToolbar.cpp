// SHW Spacecraft editor
//
// "Main Toolbar" class for handling commands from it.

#include "MainToolbar.h"

// Includes from Editor
#include "../IEditor.h"
#include "EditorUI.h"
#include "RadioGroup.h"

// Includes from Urho3D
#include "Urho3D/Core/Context.h"
#include "Urho3D/UI/BorderImage.h"
#include "Urho3D/UI/Button.h"
#include "Urho3D/UI/CheckBox.h"
#include <Urho3D/UI/UIEvents.h>
#include "Urho3D/UI/UI.h"
#include "Urho3D/Graphics/Graphics.h"

/// Construct.
MainToolbar::MainToolbar(Context* context, EditorUI* editor)
: Object(context),
  m_editor_ui(editor),
  m_dirty(true),
  m_symmetry_toolbar(nullptr)
{
  // Create toolbar
  m_symmetry_toolbar = new UIElement(context_);
  m_symmetry_toolbar->SetName("SymmetryToolbar");

  UI* ui = GetSubsystem<UI>();
  ui->GetRoot()->AddChild(m_symmetry_toolbar);

  //m_symmetry_toolbar->SetStyle("Transparent");
  m_symmetry_toolbar->SetAlignment(HA_CENTER, VA_TOP);
  m_symmetry_toolbar->SetLayout(LM_HORIZONTAL);
  m_symmetry_toolbar->SetLayoutSpacing(4);
  //m_symmetry_toolbar->SetSize(40, 40);
  m_symmetry_toolbar->SetLayoutBorder(IntRect(4, 4, 4, 4));
  m_symmetry_toolbar->SetOpacity(0.7);

  m_symmetry_switch = new RadioGroup(context, editor, m_symmetry_toolbar);
  m_symmetry_switch->add_switch("Symmetry2", "Symmetry 2", 2);
  m_symmetry_switch->add_switch("Symmetry3", "Symmetry 3", 3);
  m_symmetry_switch->add_switch("Symmetry4", "Symmetry 4", 4);
  m_symmetry_switch->add_switch("Symmetry6", "Symmetry 6", 6);
  m_symmetry_switch->add_switch("Symmetry8", "Symmetry 8", 8);
  SubscribeToEvent(m_symmetry_switch, E_RADIO_GROUP_SWITCHED, URHO3D_HANDLER(MainToolbar, on_symmetry_switched));
  //CheckBox* button = m_editor_ui->create_toolbar_checkbox("Manipulate", "AAA");

  m_file_toolbar = new UIElement(context_);
  m_file_toolbar->SetName("FileToolbar");
  ui->GetRoot()->AddChild(m_file_toolbar);
  m_file_toolbar->SetAlignment(HA_LEFT, VA_TOP);
  m_file_toolbar->SetLayout(LM_HORIZONTAL);
  m_file_toolbar->SetLayoutSpacing(4);
  m_file_toolbar->SetLayoutBorder(IntRect(4, 4, 4, 4));
  m_file_toolbar->SetOpacity(0.7);
  m_file_toolbar->AddChild(m_editor_ui->create_toolbar_button("FileNew", "Create new spacecraft"));
  m_file_toolbar->AddChild(m_editor_ui->create_toolbar_button("FileOpen", "Open spacecraft from file"));
  m_file_toolbar->AddChild(m_editor_ui->create_toolbar_button("FileSave", "Save spacecraft to file"));


  //SubscribeToEvent(button, E_TOGGLED, URHO3D_HANDLER(MainToolbar, on_button));

  /*m_symmetry_toolbar->AddChild(m_editor_ui->create_toolbar_checkbox("Symmetry2", "Symmetry 2"));
  m_symmetry_toolbar->AddChild(m_editor_ui->create_toolbar_checkbox("Symmetry3", "Symmetry 3"));
  m_symmetry_toolbar->AddChild(m_editor_ui->create_toolbar_checkbox("Symmetry4", "Symmetry 4"));
  m_symmetry_toolbar->AddChild(m_editor_ui->create_toolbar_checkbox("Symmetry6", "Symmetry 6"));
  m_symmetry_toolbar->AddChild(m_editor_ui->create_toolbar_checkbox("Symmetry8", "Symmetry 8"));*/

  /*StringHash old_category;
  for (int i = 0; i < m_context_buttons.Size(); ++i) {
  ContextButton& context_button = m_context_buttons[i];
  CheckBox* button = editor_ui->create_toolbar_checkbox(
  context_button.icon,
  context_button.description
  );
  // Set context id for command handler
  button->SetVar(s_context_id, i);
  context_button.button = button;

  SubscribeToEvent(button, E_TOGGLED, URHO3D_HANDLER(MainToolbar, on_button));

  if (old_category && old_category != context_button.category) {
  UIElement* spacer = editor_ui->create_vertical_spacer(8);
  m_toolbar->AddChild(spacer);
  }
  old_category = context_button.category;
  m_toolbar->AddChild(button);
  }*/

  Graphics* graphics = GetSubsystem<Graphics>();
  resize(graphics->GetWidth(), graphics->GetHeight());
  // TODO: create all guts here
}

/// Destructor
MainToolbar::~MainToolbar()
{
}

/// Update dirty toolbar
void MainToolbar::update(float dt)
{
  m_symmetry_switch->update(dt);
  if (m_dirty) {
    /*for (int i = 0; i < m_context_buttons.Size(); ++i) {
      ContextButton& context_button = m_context_buttons[i];
      bool current = i == m_current_context;
      if (context_button.button->IsChecked() != current) {
        context_button.button->SetChecked(current);
      }
    }*/

    m_dirty = false;
  }
}

/// Update on resize
void MainToolbar::resize(int w, int h)
{
}

/// On symmetry switched handler
void MainToolbar::on_symmetry_switched(StringHash event_type, VariantMap& event_data)
{
  m_editor_ui->editor()->set_symmetry(m_symmetry_switch->switch_state());
}

/// On button pressed handler. Switches context
void MainToolbar::on_button(StringHash event_type, VariantMap& event_data)
{
  CheckBox* clicked =
    static_cast<CheckBox*>(event_data[UIMouseClick::P_ELEMENT].GetPtr());
  if (clicked && clicked->IsChecked()) {
    //activate_context(clicked->GetVar(s_context_id).GetUInt());
  }
  m_dirty = true;
}
