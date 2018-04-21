// SHW Spacecraft editor
//
// Base abstract context class, knows about symmetry settings

#include "CreationContext.h"

// Includes from Editor
#include "../Model/DynamicModel.h"
#include "../Model/SceneModel.h"
#include "../Model/ProceduralUnit.h"
#include "../Model/BasePositioner.h"
#include "../Model/BaseAttachableSurface.h"
#include "../Model/SurfaceNodePositioner.h"
#include "../Model/SurfaceSurfacePositioner.h"
#include "../Model/SurfaceMount.h"
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

/// Unit class name parameter ID
ParameterID s_unit_class = 0;
// TODO: move to separate context for procedural units
/// Function name for procedural units
ParameterID s_function_name = 1;

/// Construct.
CreationContext::CreationContext(Context* context)
  : BaseContext(context),
    m_state(0),
    m_moved(false)
{
}

/// Destructor
CreationContext::~CreationContext()
{
}

/// Initialize context
void CreationContext::initialize()
{
}

/// Gets unit class name
StringHash CreationContext::unit_class() const
{
  return m_parameters[s_unit_class].GetStringHash();
}

/// Gets generation function name
StringHash CreationContext::function_name() const
{
  return m_parameters[s_function_name].GetStringHash();
}

/// Create rollower
void CreationContext::create_rollower()
{
  //m_orientation = Quaternion();
  assert(!m_rollowers.Size());
  StringHash unit_class_value = unit_class();
  StringHash function_name_value = function_name();
  if (unit_class_value && function_name_value) {
    m_state = 0;
    for (int i = 0; i < m_symmetry; ++i) {
      UnitModel* unit = model()->create_unit(unit_class_value, Vector3(0, 0, 0));
      ProceduralUnit* proc_unit = dynamic_cast<ProceduralUnit*>(unit);
      if (proc_unit) {
        proc_unit->set_function_name(function_name_value);
        
        // Set parameters from last creation
        Pair<StringHash, StringHash> key(unit_class_value, function_name_value);
        auto& par_it = m_latest_parameters.Find(key);
        if (par_it != m_latest_parameters.End()){
          proc_unit->set_parameters(par_it->second_);
        }
      }

      m_rollowers.Push(SharedPtr<UnitModel>(unit));
      view()->select(unit->GetNode());
    }
    update_rollower_position();
  }
}

template<class T>
T* get_or_create_positioner(Node* node)
{
  T* result = node->GetComponent<T>();
  if (!result) {
    // Remove old positioner, if it is not compatiable with requested.
    auto old = node->GetDerivedComponent<BasePositioner>();
    node->RemoveComponent(old);
    // Create new one
    result = node->CreateComponent<T>();
  }
  return result;
}

/// Updates rollower position
void CreationContext::update_rollower_position()
{
  if (m_rollowers.Size()) {
    // Symmetry
    Node* first_rollower_node = m_rollowers[0]->GetNode();
    Vector3 attach_position;
    Vector3 attach_normal;
    Node* unit_under_mouse = get_unit_under_mouse(&attach_position, &attach_normal);
    auto surface =
      first_rollower_node->GetDerivedComponent<BaseAttachableSurface>();
    auto surface_mount =
      first_rollower_node->GetDerivedComponent<SurfaceMount>();

    // TODO: use some more explicit way to detect possible attachments
    // If module has attable surface - use free positioner, or surface - surface
    if (surface) {
      if (!unit_under_mouse) {
        // TODO: localisation
        // TODO: turn symmetry off for inserting to main axis
        String snap_text = "main axis";
        Ray camera_ray = calculate_ray();
        Ray main_axis_ray(Vector3(0, 0, 0), Vector3(0, 0, 1));
        Vector3 pos = main_axis_ray.ClosestPoint(camera_ray);
        // Insert into vertical / horizontal plane, stick to main axis, only if it close to it
        float t = M_INFINITY;
        Plane hor_plane(Vector4(0, 1, 0, 0));
        String plane_text = "horizontal plane";
        float cur_t = camera_ray.HitDistance(hor_plane);
        if (cur_t > 0) {
          t = Min(cur_t, t);
        }
        Plane vert_plane(Vector4(1, 0, 0, 0));
        cur_t = camera_ray.HitDistance(vert_plane);
        if (cur_t > 0) {
          if (cur_t < t) {
            t = cur_t;
            plane_text = "vertical plane";
          }
        }
        float min_size = node_size(first_rollower_node);
        if (t < M_INFINITY) {
          Vector3 plane_pos = camera_ray.origin_ + camera_ray.direction_ * t;
          if ((plane_pos - pos).Length() > min_size) {
            snap_text = plane_text;
            pos = plane_pos;
          }
        }

        // Snapping
        if (min_size > 0) {
          float step = quantizing_step(min_size * 2);
          pos.x_ = round(pos.x_ / step) * step;
          pos.y_ = round(pos.y_ / step) * step;
          pos.z_ = round(pos.z_ / step) * step;
        }
        String tooltip_text = "Snap to: " + snap_text + "\n";
        tooltip_text += String(pos.x_) + "m; " + String(pos.y_) + "m; " + String(pos.z_) + "m";
        set_tooltip(tooltip_text);

        auto positions = get_symmetry_positions(pos);
        assert(m_rollowers.Size() == positions.Size());
        Vector3 z(0, 0, 1);
        for (int i = 0; i < m_rollowers.Size(); ++i) {
          Node* rollower_node = m_rollowers[i]->GetNode();
          // Insert positioner and update its internal position
          BasePositioner* positioner = 
            get_or_create_positioner<BasePositioner>(rollower_node);
          positioner->update_internal_position();

          pos = positions[i];
          rollower_node->SetPosition(pos);
          Vector3 y = pos;
          y.z_ = 0;
          y.Normalize();
          Quaternion symmetry_rotation;
          if (y.LengthSquared() > 0.9) {
            Vector3 x = y.CrossProduct(z);
            symmetry_rotation.FromAxes(x, y, z);
          }

          // WASDQE-rotation
          rollower_node->SetRotation(symmetry_rotation * m_orientation);
        }
      } else {
        hide_tooltip();
        // TODO: attachment
        // TODO: highlight attach point
      }
    } else if (surface_mount) {
      // Case for surface - node attachable units
      if (unit_under_mouse) {
        surface = unit_under_mouse->GetDerivedComponent<BaseAttachableSurface>();
        if (surface) {
          // Get symmetry nodes
          auto attach_nodes = get_symmety_nodes(unit_under_mouse);
          // If more than one unit
          if (attach_nodes.Size() > 1) {
            // Use underlaying nodes as a guide
            for (int i = 0; i < m_rollowers.Size(); ++i) {
              Node* rollower_node = m_rollowers[i]->GetNode();
              if (i < attach_nodes.Size()) {
                // Set new parent
                rollower_node->SetParent(attach_nodes[i]);
                // Get positioner
                auto positioner =
                  get_or_create_positioner<SurfaceNodePositioner>(rollower_node);
                // and set position
                rollower_node->SetEnabledRecursive(
                  positioner->set_position(attach_position, attach_normal, m_orientation)
                );
              } else {
                rollower_node->SetEnabledRecursive(false);
              }
            }
          } else {
            // for just one unit - use symmetry positions
            auto positions = get_symmetry_positions(attach_position);
            // and normals
            auto normals = get_symmetry_positions(attach_normal);
            for (int i = 0; i < m_rollowers.Size(); ++i) {
              Node* rollower_node = m_rollowers[i]->GetNode();
              // Set new parent
              rollower_node->SetParent(unit_under_mouse);
              // Get positioner
              auto positioner =
                get_or_create_positioner<SurfaceNodePositioner>(rollower_node);
              // and set position
              rollower_node->SetEnabledRecursive(
                positioner->set_position(positions[i], normals[i], m_orientation)
              );
            }
          }

          if (m_rollowers[0]->GetNode()->IsEnabled()) {
            set_tooltip("Attach to module");
          }
          return;
        }
      }
      // If no unit under cursor with attachable surface - hide rollowers
      for (int i = 0; i < m_rollowers.Size(); ++i) {
        m_rollowers[i]->GetNode()->SetEnabledRecursive(false);
      }
      hide_tooltip();
    }
  }
}

/// Activates context and allows it to set up all its guts
void CreationContext::activate()
{
  BaseContext::activate();
  // Create rollower
  create_rollower();
}

/// Deactivate context and remove all temporary objects
void CreationContext::deactivate()
{
  view()->clear_selection();
  // Delete rollowers
  for (int i = 0; i < m_rollowers.Size(); ++i) {
    model()->delete_unit(m_rollowers[i]->GetNode());
  }
  m_rollowers.Clear();
  BaseContext::deactivate();
}

/// Mouse button down handler
void CreationContext::on_mouse_down()
{
  if (!(m_rollowers.Size() && m_rollowers[0]->GetNode()->IsEnabled())) {
    return;
  }
  Ray camera_ray = calculate_ray();
  // 0 state - switch to attributes editing
  if (0 == m_state) {
    if (m_rollowers.Size()) {
      m_last_position = camera_ray.Project(m_rollowers[0]->GetNode()->GetPosition());
    }
  } else {
    m_last_position = camera_ray.Project(m_last_position);
  }

  UI* ui = GetSubsystem<UI>();
  m_last_cursor_pos = ui->GetCursorPosition();
  
  // Increment state on each click
  ++m_state;

  m_moved = false;

  // TODO: factor out
  // Save initial value of parameter
  ParameterID id;
  if (m_rollowers.Size() && get_interactive_parameter(m_state, id)) {
    m_initial_value = m_rollowers[0]->parameters()[id];
  }
}

/// Mouse button up handler
void CreationContext::on_mouse_up()
{
  Ray camera_ray = calculate_ray();
  Vector3 cur_position = camera_ray.Project(m_last_position);
  ParameterID id;
  // Insert node with current parameters, on single click
  if (!get_interactive_parameter(m_state, id) ||
      (m_state == 1 && !m_moved)) {

    // Save parameters for late reusing
    if (m_rollowers.Size()) {
      Pair<StringHash, StringHash> key(unit_class(), function_name());
      m_latest_parameters[key] = m_rollowers[0]->parameters();
    }

    for (int i = 0; i < m_rollowers.Size(); ++i) {
      Node* node = m_rollowers[i]->GetNode();
      if (!node->IsEnabled()) {
        model()->delete_unit(node);
      }
    }

    m_rollowers.Clear();
    view()->clear_selection();
    commit_transaction();
    create_rollower();
  } else {
    if (m_state == 1) {
      ++m_state;

      // TODO: factor out
      UI* ui = GetSubsystem<UI>();
      m_last_cursor_pos = ui->GetCursorPosition();

      m_last_position = camera_ray.Project(m_last_position);
      // Save initial value of parameter
      ParameterID id;
      if (m_rollowers.Size() && get_interactive_parameter(m_state, id)) {
        m_initial_value = m_rollowers[0]->parameters()[id];
      }
    }
  }
  // TODO: right click (double click) - cancel going throught parameters
}

/// Mouse button move handler
void CreationContext::on_mouse_move(float x, float y)
{
  m_moved = true;
  // 0 state - update rollower position
  if (0 == m_state) {
    update_rollower_position();
  } else {
    // 1 and following states - update interactive parameters
    Ray camera_ray = calculate_ray();
    Vector3 cur_position = camera_ray.Project(m_last_position);
    ParameterID id;
    if (get_interactive_parameter(m_state, id)) {
      auto& description = m_rollowers[0]->parameters_description()[id];
      // TODO: localization
      String tooltip_text = description.m_name + ": ";
      int flags = description.m_flags;

      // Moduling direction: upper-right +, lower-down -
      UI* ui = GetSubsystem<UI>();
      IntVector2 cursor_pos = ui->GetCursorPosition();
      cursor_pos -= m_last_cursor_pos;
      float diagonal = cursor_pos.x_ - cursor_pos.y_;

      // Raw value based on distance from last position in scene
      float raw_value = 0;
      // TODO: add angular value
      if (flags & pfLINEAR_VALUE) {
        if (cursor_pos.Length() >= FLT_EPSILON) {
          diagonal /= cursor_pos.Length();
        }
        raw_value = (cur_position - m_last_position).Length() * diagonal * 2;
      } else {
        Graphics* graphics = GetSubsystem<Graphics>();
        diagonal /= (graphics->GetWidth() + graphics->GetHeight()) / 4;
        raw_value = Max(-1.0, Min(1.0, diagonal));
        raw_value *= Max(abs(description.m_min.GetFloat()),
          abs(description.m_max.GetFloat()));
      }

      // TODO: factor out
      Variant real_value;
      if (m_initial_value.GetType() == VAR_DOUBLE || m_initial_value.GetType() == VAR_FLOAT) {
        raw_value = raw_value + m_initial_value.GetFloat();
        // Snapping to some round values
        raw_value = quantize(raw_value);

        // Checking limits
        raw_value = Max(raw_value, description.m_min.GetFloat());
        raw_value = Min(raw_value, description.m_max.GetFloat());

        real_value = raw_value;
        tooltip_text += String(real_value);
        if (flags & pfLINEAR_VALUE) {
          tooltip_text += "m";
        }
      } else if (m_initial_value.GetType() == VAR_INT) {
        String addition = "";
        // For size in cells - search cell size
        float cell_size = 1;
        if (flags & pfCELLS) {
          auto& descriptions = m_rollowers[0]->parameters_description();
          const Vector<ParameterID> ids = descriptions.parameter_ids();
          for (int i = 0; i < ids.Size(); ++i) {
            ParameterID cur_id = ids[i];
            if (descriptions[cur_id].m_flags & pfCELL_SIZE) {
              cell_size = m_rollowers[0]->parameters()[cur_id].GetFloat();
              break;
            }
          }
          raw_value /= cell_size;
        }

        raw_value = raw_value + m_initial_value.GetInt();
        raw_value = round(raw_value);

        // Checking limits
        raw_value = Max(raw_value, description.m_min.GetInt());
        raw_value = Min(raw_value, description.m_max.GetInt());

        real_value = (int)raw_value;
        if (flags & pfCELLS) {
          addition = " (" + String(cell_size * real_value.GetFloat()) + "m)";
        }
        tooltip_text += String(real_value) + addition;
      } else {
        // TODO: other parameter types: Vector2
        real_value = m_initial_value;
      }
      set_tooltip(tooltip_text);
      for (int i = 0; i < m_rollowers.Size(); ++i) {
        m_rollowers[i]->set_parameter(id, real_value);
      }
    }
  }
}

/// Update context each frame
void CreationContext::update(float dt)
{
  Quaternion rotation;
  auto input = GetSubsystem<Input>();
  if (input->GetKeyPress(KEY_W))
    rotation = Quaternion(90, Vector3::RIGHT);
  if (input->GetKeyPress(KEY_S))
    rotation = Quaternion(-90, Vector3::RIGHT);
  if (input->GetKeyPress(KEY_A))
    rotation = Quaternion(90, Vector3::UP);
  if (input->GetKeyPress(KEY_D))
    rotation = Quaternion(-90, Vector3::UP);
  if (input->GetKeyPress(KEY_Q))
    rotation = Quaternion(90, Vector3::FORWARD);
  if (input->GetKeyPress(KEY_E))
    rotation = Quaternion(-90, Vector3::FORWARD);

  if (rotation != Quaternion::IDENTITY) {
    m_orientation = m_orientation * rotation;
    update_rollower_position();
  } 
  if (input->GetKeyPress(KEY_R)) {
    m_orientation = Quaternion();
    update_rollower_position();
  }
}

/// Get i-th interactive parameter of current procedural object
bool CreationContext::get_interactive_parameter(int ind, ParameterID& id)
{
  // TODO: factor out accessor to units
  auto& nodes = view()->selected();
  if (nodes.Size()) {
    UnitModel* unit = nodes[0]->GetDerivedComponent<UnitModel>();
    const ParametersDescription& descriptions = unit->parameters_description();
    const Vector<ParameterID> ids = descriptions.parameter_ids();
    int cur_index = 0;
    for (int i = 0; i < ids.Size(); ++i) {
      ParameterID cur_id = ids[i];
      if (descriptions[cur_id].m_flags & pfINTERACTIVE) {
        ++cur_index;
        if (cur_index == ind) {
          id = cur_id;
          return true;
        }
      }
    }
  }
  return false;
}

/// Picking filter function
bool CreationContext::is_pickable(Node* node)
{
  for (int i = 0; i < m_rollowers.Size(); ++i) {
    // TODO: support multi-node units
    Node* sel_node = m_rollowers[i]->GetNode();
    if (sel_node == node || sel_node == node->GetParent()) {
      return false;
    }
  }
  return true;
}
