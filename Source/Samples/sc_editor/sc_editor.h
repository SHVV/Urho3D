// SHW Spacecraft editor
//
// Main application class

#pragma once

#include "Sample.h"

class SceneModel;
class SceneView;

namespace Urho3D
{
  class Node;
  class Scene;
}

class SCEditor : public Sample
{
  URHO3D_OBJECT(SCEditor, Sample);

public:
  /// Construct.
  SCEditor(Context* context);

  /// 
  virtual void Setup() override;

  /// Setup after engine initialization and before running the main loop.
  virtual void Start();

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
  /// Set up a viewport for displaying the scene.
  void SetupViewport();
  /// Subscribe to application-wide logic update events.
  void SubscribeToEvents();
  /// Read input and move the camera.
  void MoveCamera(float timeStep);
  /// Animate the vertex data of the objects.
  void AnimateObjects(float timeStep);
  /// Handle the logic update event.
  void HandleUpdate(StringHash eventType, VariantMap& eventData);

  /// Animation flag.
  bool animate_;
  /// Animation's elapsed time.
  float time_;

  /// Scene model
  SharedPtr<SceneModel> m_model;
  /// Scene editor view
  SharedPtr<SceneView> m_view;
};
