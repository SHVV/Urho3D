// SHW Spacecraft editor
//
// Base abstract context class, knows about symmetry settings

#include "CreationContext.h"

// Includes from Editor
#include "../Model/DynamicModel.h"
#include "../Model/SceneModel.h"
#include "../Model/ProceduralUnit.h"
#include "../View/SceneView.h"

// Includes from Urho3D
#include "Urho3D/Graphics/Graphics.h"
#include "Urho3D/Graphics/Camera.h"
#include "Urho3D/Graphics/Octree.h"
#include "Urho3D/Scene/Node.h"
#include "Urho3D/Scene/Scene.h"
#include "Urho3D/UI/UI.h"
#include "Urho3D/Input/Input.h"

using namespace Urho3D;

/// Construct.
CreationContext::CreationContext(Context* context, IEditor* editor)
  : BaseContext(context, editor),
    m_state(0)
{
}

/// Destructor
CreationContext::~CreationContext()
{
}

/// Sets unit class name
void CreationContext::set_class_name(StringHash name)
{
  if (name != m_unit_class) {
    deactivate();
    m_unit_class = name;
    activate();
  }
}

/// Gets unit class name
StringHash CreationContext::class_name()
{
  return m_unit_class;
}

/// Sets generation function name
void CreationContext::set_function_name(StringHash name)
{
  if (m_function_name != name) {
    deactivate();
    m_function_name = name;
    activate();
  }
}

/// Gets generation function name
StringHash CreationContext::function_name()
{
  return m_function_name;
}

/// Create rollower
void CreationContext::create_rollower()
{
  if (m_function_name && m_unit_class) {
    m_state = 0;
    UnitModel* unit = model()->create_unit(m_unit_class, Vector3(0, 0, 0));
    ProceduralUnit* proc_unit = dynamic_cast<ProceduralUnit*>(unit);
    if (proc_unit) {
      proc_unit->set_function_name(m_function_name);
      // TODO: set parameters from last creation
    }

    m_rollower = unit;
    update_rollower_position();
    view()->select(m_rollower->GetNode());
  }
}

/// Updates rollower position
void CreationContext::update_rollower_position()
{
  if (m_rollower) {
    // TODO: symmetry
    Node* rollower_node = m_rollower->GetNode();
    Node* unit_under_mouse = get_unit_under_mouse();
    if (!unit_under_mouse) {
      Ray camera_ray = calculate_ray();
      Ray main_axis_ray(Vector3(0, 0, 0), Vector3(0, 0, 1));
      Vector3 pos = main_axis_ray.ClosestPoint(camera_ray);
      // TODO: snapping
      rollower_node->SetPosition(pos);
      rollower_node->SetRotation(Quaternion());
      // TODO: insert into vertical / horizontal plane, stick to main axis, only if it close to it
      // TODO: WASD-rotation
    } else {
      // TODO: attachment
      // TODO: highlight attach point
    }
  }
}

/// Activates context and allows it to set up all its guts
void CreationContext::activate()
{
  // Create rollower
  create_rollower();
}

/// Deactivate context and remove all temporary objects
void CreationContext::deactivate()
{
  // Delete rollower
  if (m_rollower) {
    view()->clear_selection();
    model()->delete_unit(m_rollower->GetNode());
  }
}

/// Mouse button down handler
void CreationContext::on_mouse_down()
{
  Ray camera_ray = calculate_ray();
  // 0 state - switch to attributes editing
  if (0 == m_state) {
    if (m_rollower) {
      m_last_position = camera_ray.Project(m_rollower->GetNode()->GetPosition());
      m_rollower = nullptr;
    }
  } else {
    m_last_position = camera_ray.Project(m_last_position);
  }

  // Increment state on each click
  ++m_state;
}

/// Mouse button up handler
void CreationContext::on_mouse_up()
{
  view()->clear_selection();
  create_rollower();
  // TODO: insert node with last parameters, on single click
  // TODO: go trought all parameters, on drag.
  // TODO: right click - cancel
}

/// Mouse button move handler
void CreationContext::on_mouse_move(float x, float y)
{
  // 0 state - update rollower position
  if (0 == m_state) {
    update_rollower_position();
  }
  // TODO: 1 and following states - update interactive parameters
}

/// Update context each frame
void CreationContext::update(float dt)
{

}

/// Picking filter function
bool CreationContext::is_pickable(Node* node)
{
  if (m_rollower) {
    // TODO: support multi-node units
    return m_rollower->GetNode() != node;
  }
  return true;
}
