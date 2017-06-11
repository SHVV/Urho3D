// SHW Spacecraft editor
//
// Base abstract context class, knows about symmetry settings

#include "BaseContext.h"

// Includes from Editor
#include "../IEditor.h"
#include "../Model/NodeModel.h"
#include "../Model/DynamicModel.h"
#include "../Model/SceneModel.h"
#include "../View/EditorUI.h"
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
BaseContext::BaseContext(Context* context)
  : Object(context),
    m_active(false),
    m_symmetry(1)
{
}

/// Destructor
BaseContext::~BaseContext()
{
}

/// Set back pointer to editor
void BaseContext::set_editor(IEditor* editor)
{
  m_editor = editor;
}

/// Set context parameters
void BaseContext::set_parameters(const Parameters& parameters)
{
  if (parameters != m_parameters) {
    bool reactivate = m_active;
    if (reactivate) {
      deactivate();
    }
    m_parameters = parameters;
    if (reactivate) {
      activate();
    }
  }
}

/// Set symmetry
void BaseContext::set_symmetry(int value)
{
  if (value != m_symmetry) {
    bool reactivate = m_active;
    if (reactivate) {
      deactivate();
    }
    m_symmetry = value;
    if (reactivate) {
      activate();
    }
  }
}

/// Initialize context
void BaseContext::initialize()
{
}

/// Activates context and allows it to set up all its guts
void BaseContext::activate()
{
  m_active = true;
}

/// Deactivate context and remove all temporary objects
void BaseContext::deactivate()
{
  m_active = false;
  hide_tooltip();
}

/// Mouse button down handler
void BaseContext::on_mouse_down()
{
}

/// Mouse button up handler
void BaseContext::on_mouse_up()
{
}

/// Mouse button move handler
void BaseContext::on_mouse_move(float x, float y)
{
}

/// Update context each frame
void BaseContext::update(float dt)
{
}

/// Get unit under mouse cursor
Node* BaseContext::get_unit_under_mouse()
{
  Ray ray = calculate_ray();
  
  PODVector<RayQueryResult> results;
  RayOctreeQuery query(results, ray, RAY_OBB, 2000, DRAWABLE_GEOMETRY);
  view()->scene()->GetComponent<Octree>()->Raycast(query);

  float t = M_INFINITY;
  Node* res = nullptr;
  for (int i = 0; i < results.Size(); ++i) {
    RayQueryResult& result = results[i];
    Node* node = result.drawable_->GetNode();
    DynamicModel* dynamic_model = node->GetDerivedComponent<DynamicModel>();
    if (dynamic_model && is_pickable(node)) {
      const MeshGeometry* mesh = dynamic_model->mesh_geometry();
      if (mesh) {
        Matrix3x4 inverse(node->GetWorldTransform().Inverse());
        Ray local_ray = ray.Transformed(inverse);

        SubObjectType sub_type;
        if (mesh->raycast(local_ray, sub_type, sotPOLYGON, true, t) >= 0) {
          res = node;
        }

        /*int ind = mesh->raycast(local_ray, sub_type, sotVERTEX | sotEDGE | sotPOLYGON, true);
        if (ind >= 0) {
          switch (sub_type) {
          case sotVERTEX:
            ui->GetCursor()->SetShape(CS_CROSS);
            break;
          case sotEDGE:
            ui->GetCursor()->SetShape(CS_IBEAM);
            break;
          case sotPOLYGON:
            ui->GetCursor()->SetShape(CS_ACCEPTDROP);
            break;
          default:
            break;
          }
          m_view->select(node);
        }*/
      }
    }
  }
  return res;
}

/// Returns all positions, based on symmetry mode
Vector<Vector3> BaseContext::get_symmetry_positions(const Vector3& base)
{
  Vector<Vector3> res;
  res.Push(base);
  float da = 2 * M_PI / m_symmetry;
  for (int i = 1; i < m_symmetry; ++i) {
    Vector3 vec;
    vec.z_ = base.z_;
    float a = da * i;
    float sa = sin(a);
    float ca = cos(a);
    vec.x_ = base.x_ * ca - base.y_ * sa;
    vec.y_ = base.x_ * sa + base.y_ * ca;
    res.Push(vec);
  }

  return res;
}

/// Returns all nodes, symmetrical to node
Vector<Node*> BaseContext::get_symmety_nodes(Node* node)
{
  Vector<Node*> result;
  if (node) {
    UnitModel* unit = node->GetDerivedComponent<UnitModel>();
    if (unit) {
      Vector<Vector3> positions = get_symmetry_positions(node->GetWorldPosition());
      for (int i = 0; i < positions.Size(); ++i) {
        Vector3& point = positions[i];
        PODVector<Drawable*> query_result;
        PointOctreeQuery query(query_result, point, DRAWABLE_GEOMETRY);
        view()->scene()->GetComponent<Octree>()->GetDrawables(query);
        for (int j = 0; j < query_result.Size(); ++j) {
          Node* test_node = query_result[j]->GetNode();
          if (test_node && test_node != node) {
            // TODO: softer limits
            if ((test_node->GetWorldPosition() - point).LengthSquared() < M_LARGE_EPSILON) {
              UnitModel* test_unit = test_node->GetDerivedComponent<UnitModel>();
              if (test_unit && test_unit->GetType() == unit->GetType()) {
                if (test_unit->parameters() == unit->parameters()) {
                  if (result.Find(test_node) == result.End()) {
                    result.Push(test_node);
                  }
                }
              }
            }
          }
        }
      }
      result.Push(node);
    }
  }
  return result;
}

/// Quantize by tenths
float BaseContext::quantize(float value)
{
  float res_val = abs(value);
  if (res_val > M_LARGE_EPSILON) {
    float step = quantizing_step(res_val);
    res_val = round(value / step) * step;
  } else {
    res_val = 0;
  }
  return res_val;
}

/// Calculate quantizing step
float BaseContext::quantizing_step(float value)
{
  float base_steps = 7.5;
  float base_step = value / base_steps;
  int step_log = floor(log10(base_step));
  float step = pow(10, step_log);
  float steps = value / step;
  if (steps > base_steps * 5) {
    step *= 5;
  } else if (steps > base_steps * 2) {
    step *= 2;
  }
  return step;
}

/// Get min axis size of node's drawable
float BaseContext::node_size(Node* node)
{
  auto drawable = node->GetDerivedComponent<Drawable>();
  float min_size = 0;
  if (drawable) {
    Vector3 hs = drawable->GetBoundingBox().HalfSize();
    min_size = Min(hs.x_, Min(hs.y_, hs.z_));
  }
  return min_size;
}

/// Set tooltip text and show it
void BaseContext::set_tooltip(const String& text)
{
  m_editor->ui()->set_context_tooltip(text);
}

/// Hide tooltip
void BaseContext::hide_tooltip()
{
  m_editor->ui()->hide_context_tooltip();
}

/// Picking filter function
bool BaseContext::is_pickable(Node* node)
{
  return true;
}


/// Get ray from current mouse position and camera
Ray BaseContext::calculate_ray()
{
  UI* ui = GetSubsystem<UI>();
  IntVector2 pos = ui->GetCursorPosition();

  Graphics* graphics = GetSubsystem<Graphics>();
  Camera* camera = view()->camera()->GetComponent<Camera>();
  return camera->GetScreenRay(
    (float)pos.x_ / graphics->GetWidth(), 
    (float)pos.y_ / graphics->GetHeight()
  );
}

/// Scene getter
SceneModel* BaseContext::model()
{
  return m_editor->model();
}

/// View getter
SceneView* BaseContext::view()
{
  return m_editor->view();
}

// Undo support
/// Commits transaction, so all tracked changes will be recorded
void BaseContext::commit_transaction()
{
  // Да-да, когда-нибудь это тоже будет :-)
}
