// SHW Spacecraft editor
//
// Base abstract context class, knows about symmetry settings

#pragma once

#include "BaseContext.h"

// Editor classes predeclaration
class UnitModel;

using namespace Urho3D;

class CreationContext : public BaseContext {
  // Enable type information.
  URHO3D_OBJECT(CreationContext, BaseContext);
public:

  /// Construct.
  CreationContext(Context* context, IEditor* editor);

  /// Destructor
  virtual ~CreationContext();

  /// Sets unit class name
  void set_class_name(StringHash name);
  /// Gets unit class name
  StringHash class_name();

  // TODO: extract to procedural units creation context
  /// Sets generation function name
  void set_function_name(StringHash name);
  /// Gets generation function name
  StringHash function_name();

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
  /// Picking filter function
  virtual bool is_pickable(Node* node) override;

  /// Create rollower
  void create_rollower();

  /// Updates rollower position
  void update_rollower_position();

  /// Current unit class name
  StringHash m_unit_class;

  /// Current function name
  StringHash m_function_name;

  /// Rollower unit
  // TODO: add several nodes by symmetry
  SharedPtr<UnitModel> m_rollower;

  /// Creation state
  int m_state;

  /// Latest point on mouse down
  Vector3 m_last_position;

  // TODO: latest parameters backup for creating similar units in one click
};
