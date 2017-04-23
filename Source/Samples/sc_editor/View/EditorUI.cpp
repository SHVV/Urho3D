// SHW Spacecraft editor
//
// Editor UI helper class.

#include "EditorUI.h"

// Includes from Editor
#include "../IEditor.h"
#include "ContextToolbar.h"

// Includes from Urho3D
#include "Urho3D/Resource/ResourceCache.h"
#include "Urho3D/Resource/XMLFile.h"
#include "Urho3D/UI/Button.h"
#include "Urho3D/UI/Text.h"
#include "Urho3D/UI/ToolTip.h"
#include "Urho3D/UI/UI.h"

/// Construct.
EditorUI::EditorUI(Context* context, IEditor* editor)
: Object(context),
  m_editor(editor),
  m_root(GetSubsystem<UI>()->GetRoot())
{
  // Load XML file containing default UI style sheet
  ResourceCache* cache = GetSubsystem<ResourceCache>();
  m_style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
  m_icons = cache->GetResource<XMLFile>("UI/SCEditorIcons.xml");

  // Set the loaded style as default style
  m_root->SetDefaultStyle(m_style);

  m_context_toolbar = new ContextToolbar(context, editor);
  // TODO: create all guts here

  //SubscribeToEvent("ScreenMode", "ResizeUI");
}

/// Destructor
EditorUI::~EditorUI()
{

}

/// Update all dirty UI controls
void EditorUI::update(float dt)
{
  m_context_toolbar->update(dt);
  // TODO: update all UI here
}

/// Update on resize
void EditorUI::resize(int w, int h)
{
  m_context_toolbar->resize(w, h);
  // TODO: update all UI here
}

/// Returns context toolbar
ContextToolbar* EditorUI::context_toolbar()
{
  return m_context_toolbar;
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

/*
UIElement@ CreateSmallToolBarSpacer(uint width)
{
  UIElement@ spacer = UIElement();
  spacer.SetFixedHeight(width);
  return spacer;
}
*/