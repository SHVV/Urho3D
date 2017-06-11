// SHW Spacecraft editor
//
// "Toolbar" class for handling context switching.

#pragma once

#include "../Core/Parameters.h"

#include <Urho3D/Core/Object.h>
#include <Urho3D/Container/HashMap.h>

// Editor predeclarations
class BaseContext;
class EditorUI;

// Urho predeclarations
namespace Urho3D {
  class Context;
  class UIElement;
  class CheckBox;
}

using namespace Urho3D;

class ContextToolbar : public Object {
  // Enable type information.
  URHO3D_OBJECT(ContextToolbar, Object);
public:

  /// Construct.
  ContextToolbar(Context* context, EditorUI* editor);

  /// Destructor
  virtual ~ContextToolbar();

  /// Update dirty toolbar
  void update(float dt);

  /// Update on resize
  void resize(int w, int h);
  // todo make virtual and some base class

  /// Adds context, returns context ID
  int add_context(
    StringHash context,           // Editor context type
    const Parameters& parameters, // Context parameters, like model,..
    StringHash category,          // Category of context, determines tab
    const String& icon,           // Toolbar icon
    const String& title,          // Button title
    const String& description     // Button description for tooltip
  );

  /// Sets default context to ID
  void set_default_context(int id);

  /// Switch active context to id
  void activate_context(int id);

  /// Switch to default context (selection)
  void activate_default_context();

private:
  /// On button pressed handler. Switches context
  void on_button(StringHash event_type, VariantMap& event_data);

  /// Editor UI
  EditorUI* m_editor_ui;

  /// UI toolbar element
  UIElement* m_toolbar;

  /// Is dirty
  bool m_dirty;

  /// Contexts classes map
  HashMap<StringHash, SharedPtr<BaseContext>> m_contexts;

  /// Internal structure to store actual context settngs for each button
  struct ContextButton {
    SharedPtr<BaseContext> context; // Editor context
    Parameters parameters;          // Context parameters, like model,..
    StringHash category;            // Category of context, determines tab
    String icon;                    // Toolbar icon
    String title;                   // Button title
    String description;             // Button description for tooltip
    WeakPtr<CheckBox> button;       // Button pointer
  };

  /// Contexts vector
  Vector<ContextButton> m_context_buttons;

  /// Default context
  unsigned int m_default_context;

  /// Current context
  int m_current_context;
};
