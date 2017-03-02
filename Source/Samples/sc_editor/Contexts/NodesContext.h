// SHW Spacecraft editor
//
// Nodes manipulation context class, used for move, rotate ad clone nodes

#pragma once

#include "BaseContext.h"

// Urho predeclarations
namespace Urho3D {
  class StaticModel;
}

using namespace Urho3D;

class NodesContext : public BaseContext {
  // Enable type information.
  URHO3D_OBJECT(NodesContext, BaseContext);
public:

  /// Construct.
  NodesContext(Context* context, SceneModel* model, SceneView* view);

  /// Destructor
  virtual ~NodesContext();

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

private:
  // Gimabl node for manipulation
  SharedPtr<StaticModel> m_gizmo;
};
