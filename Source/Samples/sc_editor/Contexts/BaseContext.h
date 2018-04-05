// SHW Spacecraft editor
//
// Base abstract context class, knows about symmetry settings

#pragma once

#include "../Core/Parameters.h"

#include <Urho3D/Core/Object.h>
#include "Urho3D/Math/Ray.h"

// Editor predeclarations
class SceneModel;
class SceneView;
class IEditor;

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
  BaseContext(Context* context);

  /// Destructor
  virtual ~BaseContext();

  /// Set back pointer to editor
  void set_editor(IEditor* editor);

  /// Set context parameters
  void set_parameters(const Parameters& parameters);

  /// Set symmetry
  void set_symmetry(int value);

  /// Initialize context
  virtual void initialize();

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

protected:
  /// Get ray from current mouse position and camera
  Ray calculate_ray();

  /// Get unit under mouse cursor
  Node* get_unit_under_mouse(Vector3* position = nullptr, Vector3* normal = nullptr);

  /// Returns all positions, based on symmetry mode
  Vector<Vector3> get_symmetry_positions(const Vector3& base);

  /// Returns all nodes, symmetrical to node
  Vector<Node*> get_symmety_nodes(Node* node);

  /// Quantize by tenths
  float quantize(float value);

  /// Calculate quantizing step
  float quantizing_step(float value);

  /// Get min axis size of node's drawable
  static float node_size(Node* node);

  /// Angle snapping
  float snap_angle(float value);

  /// Picking filter function
  virtual bool is_pickable(Node* node);

  /// Set tooltip text and show it
  void set_tooltip(const String& text);

  /// Hide tooltip
  void hide_tooltip();

  /// Scene getter
  SceneModel* model();
  /// View getter
  SceneView* view();

  // Undo support. 
  /// Commits transaction, so all tracked changes will be recorded in history
  void commit_transaction();
  
  /// Scene model
  IEditor* m_editor;

  /// Context parameters
  Parameters m_parameters;

  /// Symmetry
  int m_symmetry;

  /// Is active
  bool m_active;
};
