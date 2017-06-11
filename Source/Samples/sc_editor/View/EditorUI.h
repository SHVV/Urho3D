// SHW Spacecraft editor
//
// Editor UI helper class.

#pragma once

#include <Urho3D/Core/Object.h>

// Editor predeclarations
class ContextToolbar;
class MainToolbar;
class IEditor;

// Urho predeclarations
namespace Urho3D {
  class Context;
  class UIElement;
  class Button;
  class XMLFile;
  class CheckBox;
  class Text;
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

  /// Returns editor
  IEditor* editor();

  /// Set context tooltip text and show it
  void set_context_tooltip(const String& text);

  /// Hide cntext tooltip
  void hide_context_tooltip();

  Button* create_toolbar_button(const String& title, const String& toolTipTitle = "");
  CheckBox* create_toolbar_checkbox(const String& title, const String& toolTipTitle = "");
  void create_toolbar_icon(UIElement* element);
  UIElement* create_tooltip(UIElement* parent, const String& title, const IntVector2& offset);
  UIElement* create_vertical_spacer(int space);

  void on_resize(StringHash event_type, VariantMap& event_data);

  // TODO:
  // - create all guts
  // - subscribe to resize
  // - subscribe to update dirty UI controls
  // - Handle hide / show, fade...
  // - ...

private:
  /// Context switch toolbar
  SharedPtr<ContextToolbar> m_context_toolbar;
  /// Main toolbar
  SharedPtr<MainToolbar> m_main_toolbar;
  /// Editor
  IEditor* m_editor;
  /// The UI's root UIElement.
  SharedPtr<UIElement> m_root;
  /// Default style
  SharedPtr<XMLFile> m_style;
  /// Icons style
  SharedPtr<XMLFile> m_icons;
  /// Context tooltip
  SharedPtr<UIElement> m_context_tooltip;
  /// Context tooltip text
  SharedPtr<Text> m_context_tooltip_text;
  /// Context tooltip visible
  bool m_context_tooltip_visible;
};
