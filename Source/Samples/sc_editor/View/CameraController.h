// SHW Spacecraft editor
//
// Default camera controller for editor

#pragma once

#include "Urho3D/Core/Object.h"
#include "Urho3D/Math/Ray.h"

// Editor predeclarations
class SceneView;

// Urho predeclarations
namespace Urho3D {
  class Node;
}

using namespace Urho3D;

class CameraController : public Object {
  // Enable type information.
  URHO3D_OBJECT(CameraController, Object);
public:

  /// Construct.
  CameraController(Context* context, SceneView* view);

  /// Destructor
  virtual ~CameraController();

  /// Mouse button down handler
  virtual void on_mouse_down(VariantMap& event_data);

  /// Mouse button up handler
  //virtual void on_mouse_up(VariantMap& event_data);

  /// Update controller on each frame
  virtual void update(float dt);

protected:
  /// Get ray from current mouse position and camera
  Ray calculate_ray();

  /// Get unit under mouse cursor
  Node* get_unit_under_mouse();

  /// Scene view
  SharedPtr<SceneView> m_view;

  /// Camera target during movement
  Vector3 m_target;
};
