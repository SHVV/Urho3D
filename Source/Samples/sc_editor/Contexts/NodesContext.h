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
  /// Calculates closest point for current gizmo part
  Vector3 calculate_gizmo_point();

  // Gizmo node for manipulation
  SharedPtr<Node> m_gizmo;
  // Gizmo part
  enum Axis {
    aX, aY, aZ
  };
  struct GizmoPart {
    SharedPtr<StaticModel> m_component;
    Axis m_axis;
    bool m_rotation;
  };
  // All gizmo parts
  Vector<GizmoPart> m_gizmo_parts;
  // Highligted gizmo part
  GizmoPart* m_focus_part;
  // Selected gizmo part
  GizmoPart* m_active_part;
  // Last position of active gizmo part
  Vector3 m_gizmo_pos;
};
