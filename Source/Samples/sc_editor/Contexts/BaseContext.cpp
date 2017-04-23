// SHW Spacecraft editor
//
// Base abstract context class, knows about symmetry settings

#include "BaseContext.h"

// Includes from Editor
#include "../IEditor.h"
#include "../Model/NodeModel.h"
#include "../Model/DynamicModel.h"
#include "../Model/SceneModel.h"
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
BaseContext::BaseContext(Context* context, IEditor* editor)
  : Object(context),
    m_editor(editor)
{
}

/// Destructor
BaseContext::~BaseContext()
{
}

/// Activates context and allows it to set up all its guts
void BaseContext::activate()
{
}

/// Deactivate context and remove all temporary objects
void BaseContext::deactivate()
{
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
