// SHW Spacecraft editor
//
// Base abstract context class, knows about symmetry settings

#pragma once

#include <Urho3D/Core/Object.h>
#include "Urho3D/Math/Ray.h"

// Editor predeclarations
class SceneModel;
class SceneView;

// Urho predeclarations
namespace Urho3D {
  class Node;
}

using namespace Urho3D;

class BaseContext : public Object {
  // Enable type information.
  URHO3D_OBJECT(BaseContext, Object);
public:

  /// Construct.
  BaseContext(Context* context, SceneModel* model, SceneView* view);

  /// Destructor
  virtual ~BaseContext();

  /// Activates context and allows it to set up all its guts
  virtual void activate();

  /// Deactivate context and remove all temporary objects
  virtual void deactivate();

  /// Mouse button down handler
  virtual void on_mouse_down();

  /// Mouse button up handler
  virtual void on_mouse_up();

  /// Mouse button move handler
  virtual void on_mouse_move(float x, float y);

  /// Update context each frame
  virtual void update(float dt);

  // TODO:
  // Symmetry
  // Snapping

protected:
  /// Get ray from current mouse position and camera
  Ray calculate_ray();

  /// Get unit under mouse cursor
  Node* get_unit_under_mouse();

  // Undo support. 
  /// Commits transaction, so all tracked changes will be recorded in history
  void commit_transaction();

  /// Scene model
  SharedPtr<SceneModel> m_model;
  /// Scene view
  SharedPtr<SceneView> m_view;
};
