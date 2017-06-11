// SHW Spacecraft editor
//
// "RadioGroup" class for handling set of button switches as single object.

#pragma once

#include <Urho3D/Core/Object.h>

// Editor predeclarations
class EditorUI;

// Urho predeclarations
namespace Urho3D {
  class Context;
  class UIElement;
  class CheckBox;
}

using namespace Urho3D;

URHO3D_EVENT(E_RADIO_GROUP_SWITCHED, RadioGroupSwitched)
{
  URHO3D_PARAM(P_RADIO_GROUP, RadioGroup);                    // This pointer
}

class RadioGroup : public Object {
  // Enable type information.
  URHO3D_OBJECT(RadioGroup, Object);
public:

  /// Construct.
  RadioGroup(Context* context, EditorUI* editor, UIElement* parent);

  /// Destructor
  virtual ~RadioGroup();

  /// Update dirty control
  void update(float dt);

  /// Add switch
  void add_switch(const String& icon, const String& tooltip, int value);

  /// Switch state
  void switch_state(int value);
  int switch_state() const;

private:
  /// On button pressed handler.
  void on_button(StringHash event_type, VariantMap& event_data);

  /// Editor
  EditorUI* m_editor_ui;

  /// Parent UI element
  UIElement* m_parent;

  /// Set of all buttons
  Vector<CheckBox*> m_buttons;

  /// Is dirty
  bool m_dirty;

  /// State
  int m_state;
};
