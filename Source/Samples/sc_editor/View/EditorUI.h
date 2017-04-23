// SHW Spacecraft editor
//
// Editor UI helper class.

#pragma once

#include <Urho3D/Core/Object.h>

// Editor predeclarations
class ContextToolbar;
class IEditor;

// Urho predeclarations
namespace Urho3D {
  class Context;
  class UIElement;
  class Button;
  class XMLFile;
}

using namespace Urho3D;

class EditorUI : public Object {
  // Enable type information.
  URHO3D_OBJECT(EditorUI, Object);
public:

  /// Construct.
  EditorUI(Context* context, IEditor* editor);

  /// Destructor
  virtual ~EditorUI();

  /// Update all dirty UI controls
  void update(float dt);

  /// Update on resize
  void resize(int w, int h);

  /// Returns context toolbar
  ContextToolbar* context_toolbar();

  Button* create_toolbar_button(const String& title, const String& toolTipTitle = "");
  void create_toolbar_icon(UIElement* element);
  UIElement* create_tooltip(UIElement* parent, const String& title, const IntVector2& offset);

  // TODO:
  // - create all guts
  // - subscribe to resize
  // - subscribe to update dirty UI controls
  // - Handle hide / show, fade...
  // - ...

private:
  /// Context switch toolbar
  SharedPtr<ContextToolbar> m_context_toolbar;
  /// Editor
  IEditor* m_editor;
  /// The UI's root UIElement.
  SharedPtr<UIElement> m_root;
  /// Default style
  SharedPtr<XMLFile> m_style;
  /// Icons style
  SharedPtr<XMLFile> m_icons;
};
