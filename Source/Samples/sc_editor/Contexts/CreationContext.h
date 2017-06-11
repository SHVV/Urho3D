// SHW Spacecraft editor
//
// Base abstract context class, knows about symmetry settings

#pragma once

#include "BaseContext.h"

// Editor classes predeclaration
class UnitModel;

using namespace Urho3D;

/// Unit class name parameter ID
extern ParameterID s_unit_class;
// TODO: move to separate context for procedural units
/// Function name for procedural units
extern ParameterID s_function_name;

class CreationContext : public BaseContext {
  // Enable type information.
  URHO3D_OBJECT(CreationContext, BaseContext);
public:

  /// Construct.
  CreationContext(Context* context);

  /// Destructor
  virtual ~CreationContext();

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
  /// Picking filter function
  virtual bool is_pickable(Node* node) override;

  /// Create rollower
  void create_rollower();

  /// Updates rollower position
  void update_rollower_position();

  /// Get i-th interactive parameter of current procedural object
  // i - 1 based index
  bool get_interactive_parameter(int i, ParameterID& id);

  /// Gets unit class name
  StringHash unit_class() const;

  // TODO: extract to procedural units creation context
  /// Gets generation function name
  StringHash function_name() const;

  /// Rollower unit
  // TODO: add several nodes by symmetry
  //SharedPtr<UnitModel> m_rollower;
  Vector<SharedPtr<UnitModel>> m_rollowers;

  /// Creation state
  int m_state;

  /// Latest point on mouse down
  Vector3 m_last_position;

  /// Latest cursor position
  IntVector2 m_last_cursor_pos;

  /// Flag, that mouse was moved since the last button press
  bool m_moved;

  /// Initial parameter value
  Variant m_initial_value;

  /// Base unit orientation
  Quaternion m_orientation;

  // Latest parameters backup for creating similar units in one click
  // Map based on unit type / function name key
  HashMap<Pair<StringHash, StringHash>, Parameters> m_latest_parameters;
};
