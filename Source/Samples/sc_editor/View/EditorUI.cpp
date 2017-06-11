// SHW Spacecraft editor
//
// Editor UI helper class.

#include "EditorUI.h"

// Includes from Editor
#include "../IEditor.h"
#include "ContextToolbar.h"
#include "MainToolbar.h"

// Includes from Urho3D
#include "Urho3D/Resource/ResourceCache.h"
#include "Urho3D/Resource/XMLFile.h"
#include "Urho3D/UI/Button.h"
#include "Urho3D/UI/CheckBox.h"
#include "Urho3D/UI/Text.h"
#include "Urho3D/UI/ToolTip.h"
#include "Urho3D/UI/UI.h"
#include "Urho3D/Graphics/GraphicsEvents.h"

/// Construct.
EditorUI::EditorUI(Context* context, IEditor* editor)
: Object(context),
  m_editor(editor),
  m_root(GetSubsystem<UI>()->GetRoot()),
  m_context_tooltip_visible(false)
{
  // Load XML file containing default UI style sheet
  ResourceCache* cache = GetSubsystem<ResourceCache>();
  m_style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
  m_icons = cache->GetResource<XMLFile>("UI/SCEditorIcons.xml");

  // Set the loaded style as default style
  m_root->SetDefaultStyle(m_style);

  m_context_toolbar = new ContextToolbar(context, this);
  m_main_toolbar = new MainToolbar(context, this);
  // TODO: create all guts here

  SubscribeToEvent(E_SCREENMODE, URHO3D_HANDLER(EditorUI, on_resize));
}

/// Destructor
EditorUI::~EditorUI()
{

}

/// Update all dirty UI controls
void EditorUI::update(float dt)
{
  m_context_toolbar->update(dt);
  m_main_toolbar->update(dt);
  if (m_context_tooltip) {
    // TODO: factor out
    UI* ui = GetSubsystem<UI>();
    IntVector2 pos = ui->GetCursorPosition();
    // Check the cursor is visible and there is no UI element in front of the cursor
    bool mouse_free = ui->GetCursor()->IsVisible() && !ui->GetElementAt(pos, false);
    bool tooltip_visible = mouse_free && m_context_tooltip_visible;
    m_context_tooltip->SetPosition(pos + IntVector2(20, 20));
    m_context_tooltip->SetVisible(tooltip_visible);
  }
  // TODO: update all UI here
}

/// On resize event handler
void EditorUI::on_resize(StringHash event_type, VariantMap& event_data)
{
  using namespace ScreenMode;
  resize(event_data[P_WIDTH].GetInt(), event_data[P_HEIGHT].GetInt());
}

/// Update on resize
void EditorUI::resize(int w, int h)
{
  m_context_toolbar->resize(w, h);
  m_main_toolbar->resize(w, h);
  // TODO: update all UI here
}

/// Returns context toolbar
ContextToolbar* EditorUI::context_toolbar()
{
  return m_context_toolbar;
}

/// Returns editor
IEditor* EditorUI::editor()
{
  return m_editor;
}

/// Set context tooltip text and show it
void EditorUI::set_context_tooltip(const String& text)
{
  if (!m_context_tooltip) {
    m_context_tooltip = m_root->CreateChild<BorderImage>();
    m_context_tooltip->SetStyle("ToolTipBorderImage");

    m_context_tooltip_text = m_context_tooltip->CreateChild<Text>();
    m_context_tooltip_text->SetStyle("ToolTipText");
    //m_context_tooltip_text->SetAutoLocalizable(true);
  }
  m_context_tooltip->SetSize(1, 1);
  m_context_tooltip_text->SetText(text);
  m_context_tooltip_visible = true;
  //m_context_tooltip->SetVisible(true);
}

/// Hide cntext tooltip
void EditorUI::hide_context_tooltip()
{
  m_context_tooltip_visible = false;
}

Button* EditorUI::create_toolbar_button(const String& title, const String& toolTipTitle)
{
  Button* button = new Button(context_);
  button->SetName(title);
  button->SetDefaultStyle(m_style);
  button->SetStyle("ToolBarButton");
  button->SetFixedSize(36, 36);
  create_toolbar_icon(button);

  const String& tooltip_text = toolTipTitle.Empty() ? title : toolTipTitle;

  create_tooltip(
    button, 
    tooltip_text, 
    IntVector2(button->GetWidth() + 10, button->GetHeight() - 10)
  );

  return button;
}

CheckBox* EditorUI::create_toolbar_checkbox(const String& title, const String& toolTipTitle)
{
  CheckBox* button = new CheckBox(context_);
  button->SetName(title);
  button->SetDefaultStyle(m_style);
  button->SetStyle("ToolBarToggle");
  button->SetFixedSize(36, 36);
  create_toolbar_icon(button);

  const String& tooltip_text = toolTipTitle.Empty() ? title : toolTipTitle;

  create_tooltip(
    button,
    tooltip_text,
    IntVector2(button->GetWidth() + 10, button->GetHeight() - 10)
  );

  return button;
}


void EditorUI::create_toolbar_icon(UIElement* element)
{
  BorderImage* icon = new BorderImage(context_);
  icon->SetName("Icon");
  icon->SetDefaultStyle(m_icons);
  icon->SetStyle(element->GetName());
  icon->SetFixedSize(30, 30);
  element->AddChild(icon);
}

UIElement* EditorUI::create_tooltip(UIElement* parent, const String& title, const IntVector2& offset)
{
  ToolTip* toolTip = parent->CreateChild<ToolTip>();
  toolTip->SetPosition(offset);

  BorderImage* textHolder = toolTip->CreateChild<BorderImage>();
  textHolder->SetStyle("ToolTipBorderImage");

  Text* toolTipText = textHolder->CreateChild<Text>();
  toolTipText->SetStyle("ToolTipText");
  toolTipText->SetAutoLocalizable(true);
  toolTipText->SetText(title);

  return toolTip;
}

UIElement* EditorUI::create_vertical_spacer(int space)
{
  UIElement* spacer = new UIElement(context_);
  spacer->SetFixedHeight(space);
  return spacer;
}
