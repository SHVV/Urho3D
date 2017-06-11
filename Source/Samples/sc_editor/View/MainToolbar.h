// SHW Spacecraft editor
//
// "Main Toolbar" class for handling commands from it.

#pragma once

#include "../Core/Parameters.h"

#include <Urho3D/Core/Object.h>
#include <Urho3D/Container/HashMap.h>

// Editor predeclarations
class BaseContext;
class EditorUI;
class RadioGroup;

// Urho predeclarations
namespace Urho3D {
  class Context;
  class UIElement;
  class CheckBox;
}

using namespace Urho3D;

class MainToolbar : public Object {
  // Enable type information.
  URHO3D_OBJECT(MainToolbar, Object);
public:

  /// Construct.
  MainToolbar(Context* context, EditorUI* editor);

  /// Destructor
  virtual ~MainToolbar();

  /// Update dirty toolbar
  void update(float dt);

  /// Update on resize
  void resize(int w, int h);
  // todo make virtual and some base class

private:
  /// On button pressed handler.
  void on_button(StringHash event_type, VariantMap& event_data);

  /// On symmetry switched handler
  void on_symmetry_switched(StringHash event_type, VariantMap& event_data);

  /// Editor
  EditorUI* m_editor_ui;

  /// UI toolbar element
  UIElement* m_symmetry_toolbar;

  /// Symmetry switch group
  SharedPtr<RadioGroup> m_symmetry_switch;

  /// Is dirty
  bool m_dirty;
};
