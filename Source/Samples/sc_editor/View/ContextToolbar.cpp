// SHW Spacecraft editor
//
// "Toolbar" class for handling context switching.

#include "ContextToolbar.h"

// Includes from Editor
#include "../IEditor.h"
#include "EditorUI.h"

// Units and generators
#include "../Model/ProceduralUnit.h"
#include "../MeshGenerators/TestGenerator.h"

// Include all contexts for registering
#include "../Contexts/NodesContext.h"
#include "../Contexts/CreationContext.h"

// Includes from Urho3D
#include "Urho3D/Core/Context.h"
#include "Urho3D/Input/Input.h"
#include "Urho3D/UI/BorderImage.h"
#include "Urho3D/UI/CheckBox.h"
#include <Urho3D/UI/UIEvents.h>
#include "Urho3D/UI/UI.h"
#include "Urho3D/Graphics/Graphics.h"

static StringHash s_context_id = "ContextID";

/// Construct.
ContextToolbar::ContextToolbar(Context* context, EditorUI* editor)
: Object(context),
  m_editor_ui(editor),
  m_dirty(true),
  m_toolbar(nullptr),
  m_default_context(0),
  m_current_context(-1)
{
  // Register context factories
  // TODO: move from here
  context->RegisterFactory<NodesContext>(EDITOR_CATEGORY);
  context->RegisterFactory<CreationContext>(EDITOR_CATEGORY);

  // Create all context buttons
  // TODO: move from here

  // Modules manipulation context
  int id = add_context(
    NodesContext::GetTypeStatic(),    // Context type name
    Parameters(),                     // Context parameters, like model,..
    "Default",                        // Category of context, determines tab
    "Manipulate",                     // Toolbar icon
    "Manipulate",                     // Button title
    "Select and manipulate modules"   // Button description for tooltip
  );
  set_default_context(id);

  // Spherical tank context
  Parameters spherical_tank;
  spherical_tank[s_unit_class] = ProceduralUnit::GetTypeStatic();
  spherical_tank[s_function_name] = StringHash(TestGenerator::s_name);

  add_context(
    CreationContext::GetTypeStatic(), // Context type name
    spherical_tank,                   // Context parameters, like model,..
    "Propulsion",                     // Category of context, determines tab
    "SphereTank",                     // Toolbar icon
    "Spherical Tank",                 // Button title
    "Create spherical fuel tank"      // Button description for tooltip
  );

  //add_context(
  //  CreationContext::GetTypeStatic(), // Context type name
  //  spherical_tank,                   // Context parameters, like model,..
  //  "Propulsion",                     // Category of context, determines tab
  //  "Beam",                           // Toolbar icon
  //  "Beam",                           // Button title
  //  "Connect modules with beams"      // Button description for tooltip
  //);

  activate_default_context();
}

/// Destructor
ContextToolbar::~ContextToolbar()
{
}

/// Update dirty toolbar
void ContextToolbar::update(float dt)
{
  if (!m_toolbar) {
    // Create toolbar
    m_toolbar = new BorderImage(context_);
    m_toolbar->SetName("ContextsToolbar");

    UI* ui = GetSubsystem<UI>();
    ui->GetRoot()->AddChild(m_toolbar);

    m_toolbar->SetStyle("Transparent");
    m_toolbar->SetLayout(LM_VERTICAL);
    m_toolbar->SetLayoutSpacing(4);
    m_toolbar->SetLayoutBorder(IntRect(4, 4, 4, 4));
    m_toolbar->SetOpacity(0.7);

    Graphics* graphics = GetSubsystem<Graphics>();
    resize(graphics->GetWidth(), graphics->GetHeight());

    StringHash old_category;
    for (int i = 0; i < m_context_buttons.Size(); ++i) {
      ContextButton& context_button = m_context_buttons[i];
      CheckBox* button = m_editor_ui->create_toolbar_checkbox(
        context_button.icon,
        context_button.description
      );
      // Set context id for command handler
      button->SetVar(s_context_id, i);
      context_button.button = button;

      SubscribeToEvent(button, E_TOGGLED, URHO3D_HANDLER(ContextToolbar, on_button));

      if (old_category && old_category != context_button.category) {
        UIElement* spacer = m_editor_ui->create_vertical_spacer(8);
        m_toolbar->AddChild(spacer);
      }
      old_category = context_button.category;
      m_toolbar->AddChild(button);
    }
    // TODO: create all guts here
  }

  if (m_dirty) {
    for (int i = 0; i < m_context_buttons.Size(); ++i) {
      ContextButton& context_button = m_context_buttons[i];
      bool current = i == m_current_context;
      if (context_button.button->IsChecked() != current) {
        context_button.button->SetChecked(current);
      }
    }

    m_dirty = false;
  }

  /// Handle Esc
  Input* input = GetSubsystem<Input>();
  if (input->GetKeyPress(KEY_ESCAPE)) {
    activate_context(m_default_context);
  }
}

/// Update on resize
void ContextToolbar::resize(int w, int h)
{
  if (m_toolbar) {
    m_toolbar->SetFixedSize(44, h - 40);
    m_toolbar->SetPosition(0, 40);
    m_toolbar->SetFixedHeight(h - 40);
  }
}

/// Adds context, returns context ID
int ContextToolbar::add_context(
  StringHash context,           // Editor context type
  const Parameters& parameters, // Context parameters, like model,..
  StringHash category,          // Category of context, determines tab
  const String& icon,           // Toolbar icon
  const String& title,          // Button title
  const String& description     // Button description for tooltip
)
{
  // Create context, if does not exist
  if (m_contexts.Find(context) == m_contexts.End()) {
    SharedPtr<BaseContext> context_ptr(
      dynamic_cast<BaseContext*>(context_->CreateObject(context).Get())
    );
    context_ptr->set_editor(m_editor_ui->editor());
    context_ptr->initialize();
    m_contexts[context] = context_ptr;
  }

  ContextButton button;
  button.context = m_contexts[context];
  button.category = category;
  button.description = description;
  button.icon = icon;
  button.parameters = parameters;
  button.title = title;
  m_context_buttons.Push(button);

  return m_context_buttons.Size() - 1;
}

/// Sets default context to ID
void ContextToolbar::set_default_context(int id)
{
  m_default_context = id;
}

/// Switch active context to id
void ContextToolbar::activate_context(int id)
{
  m_current_context = id;
  m_dirty = true;

  ContextButton& context_button = m_context_buttons[id];
  if (context_button.context) {
    context_button.context->set_parameters(context_button.parameters);
  }
  m_editor_ui->editor()->set_context(context_button.context);
}

/// Switch to default context (selection)
void ContextToolbar::activate_default_context()
{
  activate_context(m_default_context);
}

/// On button pressed handler. Switches context
void ContextToolbar::on_button(StringHash event_type, VariantMap& event_data)
{
  CheckBox* clicked = 
    static_cast<CheckBox*>(event_data[UIMouseClick::P_ELEMENT].GetPtr());
  if (clicked && clicked->IsChecked()) {
    activate_context(clicked->GetVar(s_context_id).GetUInt());
  }
  m_dirty = true;
}
