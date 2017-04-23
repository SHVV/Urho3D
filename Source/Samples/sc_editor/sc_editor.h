// SHW Spacecraft editor
//
// Main application class

#pragma once

#include "Sample.h"
#include "IEditor.h"

class SceneModel;
class SceneView;
class BaseContext;
class ContextToolbar;
class CameraController;
class NodesContext;
class CreationContext;
class EditorUI;

namespace Urho3D
{
  class Node;
  class Scene;
}

class SCEditor : public Sample, public IEditor
{
  URHO3D_OBJECT(SCEditor, Sample);

public:
  /// Construct.
  SCEditor(Context* context);

  /// 
  virtual void Setup() override;

  /// Setup after engine initialization and before running the main loop.
  virtual void Start();

  /// Returns scene model
  virtual SceneModel* model() override;

  /// Returns scene view
  virtual SceneView* view() override;

  /// Returns EditorUI
  virtual EditorUI* ui() override;

  /// Returns context switcher
  virtual ContextToolbar* context_toolbar() override;

  /// Set current editor's context
  virtual void set_context(BaseContext* context) override;

protected:
  /// Return XML patch instructions for screen joystick layout for a specific sample app, if any.
  virtual String GetScreenJoystickPatchString() const {
    return
      "<patch>"
      "    <remove sel=\"/element/element[./attribute[@name='Name' and @value='Button0']]/attribute[@name='Is Visible']\" />"
      "    <replace sel=\"/element/element[./attribute[@name='Name' and @value='Button0']]/element[./attribute[@name='Name' and @value='Label']]/attribute[@name='Text']/@value\">Animation</replace>"
      "    <add sel=\"/element/element[./attribute[@name='Name' and @value='Button0']]\">"
      "        <element type=\"Text\">"
      "            <attribute name=\"Name\" value=\"KeyBinding\" />"
      "            <attribute name=\"Text\" value=\"SPACE\" />"
      "        </element>"
      "    </add>"
      "</patch>";
  }

private:
  /// Construct the scene content.
  void CreateScene();
  /// Construct an instruction text to the UI.
  void CreateInstructions();
  /// Subscribe to application-wide logic update events.
  void SubscribeToEvents();
  /// Animate the vertex data of the objects.
  void AnimateObjects(float timeStep);
  /// Handle the Mouse move.
  void HandleMouseMove(StringHash eventType, VariantMap& eventData);
  /// Handle the Mouse button down.
  void HandleMouseDown(StringHash eventType, VariantMap& eventData);
  /// Handle the Mouse button up.
  void HandleMouseUp(StringHash eventType, VariantMap& eventData);
  /// Handle the logic update event.
  void HandleUpdate(StringHash eventType, VariantMap& eventData);
  /// Handle the post rendering event.
  void PostRenderUpdate(StringHash eventType, VariantMap& eventData);
  /// Check, that mouse is not on the UI
  bool is_mouse_free();

  /// Animation flag.
  bool animate_;
  /// Animation's elapsed time.
  float time_;

  /// Scene model
  SharedPtr<SceneModel> m_model;
  /// Current context
  SharedPtr<BaseContext> m_context;
  /// Scene editor view
  SharedPtr<SceneView> m_view;
  /// Current camera controller
  SharedPtr<CameraController> m_camera_controller;
  /// UI helper
  SharedPtr<EditorUI> m_editor_ui;

  // All contexts. 
  // TODO: factor out 
  /// Nodes manipulation context
  SharedPtr<NodesContext> m_nodes_context;
  /// Creation context
  SharedPtr<CreationContext> m_creation_context;
};
