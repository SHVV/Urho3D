// SHW Spacecraft editor
//
// "Toolbar" class for handling context switching.

#include "ContextToolbar.h"

// Includes from Editor
#include "../IEditor.h"
#include "EditorUI.h"

// Include all contexts for registering
#include "../Contexts/NodesContext.h"
#include "../Contexts/CreationContext.h"

// Includes from Urho3D
#include "Urho3D/Core/Context.h"
#include "Urho3D/UI/BorderImage.h"
#include "Urho3D/UI/Button.h"
#include "Urho3D/UI/UI.h"

/// Construct.
ContextToolbar::ContextToolbar(Context* context, IEditor* editor)
: Object(context),
  m_editor(editor),
  m_dirty(true),
  m_toolbar(nullptr)
{
  // Register context factories
  // TODO: move from here
  //context->RegisterFactory<NodesContext>(EDITOR_CATEGORY);
  //context->RegisterFactory<CreationContext>(EDITOR_CATEGORY);
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

    m_toolbar->SetStyle("EditorToolBar");
    m_toolbar->SetLayout(LM_VERTICAL);
    m_toolbar->SetLayoutSpacing(4);
    m_toolbar->SetLayoutBorder(IntRect(4, 4, 4, 4));
    m_toolbar->SetOpacity(0.7);
    m_toolbar->SetFixedSize(44, 800); //TODO: real height
    m_toolbar->SetPosition(0, 40); // TODO: real height
    m_toolbar->SetFixedHeight(800); //TODO: real height

    // TODO: temporary
    EditorUI* editor_ui = m_editor->ui();
    Button* nodes_btn = editor_ui->create_toolbar_button("Manipulate", "Select and manipulate modules");
    m_toolbar->AddChild(nodes_btn);
    Button* create_btn = editor_ui->create_toolbar_button("SphereTank", "Create Spherical Fuel tank");
    m_toolbar->AddChild(create_btn);
    Button* create_btn2 = editor_ui->create_toolbar_button("Beam", "Connect modules with beams");
    m_toolbar->AddChild(create_btn2);

    // TODO: create all guts here
  }

  if (m_dirty) {
    // TODO: update buttons status here

    m_dirty = false;
  }
}

/// Update on resize
void ContextToolbar::resize(int w, int h)
{
  // TODO: update size and layout here
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
  // TODO: implement
  return 0;
}

/// Sets default context to ID
void ContextToolbar::set_default_context(int id)
{
  // TODO: implement
}

/// Switch active context to id
void activate_context(int id)
{
  // TODO: implement
}

/// Switch to default context (selection)
void activate_default_context()
{
  // TODO: implement
}
