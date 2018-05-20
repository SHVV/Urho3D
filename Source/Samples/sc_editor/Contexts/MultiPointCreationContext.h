// SHW Spacecraft editor
//
// Context for creating multiple points units like connection trusses

#pragma once

#include "CreationContext.h"

// Editor classes predeclaration
class UnitModel;

using namespace Urho3D;

/// Unit class name parameter ID
extern ParameterID s_unit_class;
// TODO: move to separate context for procedural units
/// Function name for procedural units
extern ParameterID s_function_name;

// TODO: refactor methods and properties and move them between contexts
class MultiPointCreationContext : public CreationContext {
  // Enable type information.
  URHO3D_OBJECT(MultiPointCreationContext, CreationContext);
public:

  /// Construct.
  MultiPointCreationContext(Context* context);

  /// Destructor
  virtual ~MultiPointCreationContext();

  /// Mouse button down handler
  virtual void on_mouse_down();

  /// Mouse button up handler
  virtual void on_mouse_up();

  /// Mouse button move handler
  virtual void on_mouse_move(float x, float y);

protected:
  /// Create rollower
  void create_rollower();

  /// Updates rollower position
  void update_rollower_position();

  /// Is current reference is final
  bool m_more_nodes;
};
