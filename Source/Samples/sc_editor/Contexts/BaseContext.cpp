// SHW Spacecraft editor
//
// Base abstract context class, knows about symmetry settings

#include "BaseContext.h"

// Includes from Editor
#include "../Model/NodeModel.h"
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
BaseContext::BaseContext(Context* context, SceneModel* model, SceneView* view)
  : Object(context),
    m_model(model),
    m_view(view)
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
  m_view->scene()->GetComponent<Octree>()->RaycastSingle(query);

  if (results.Size()) {
    RayQueryResult& result = results[0];
    Node* node = result.drawable_->GetNode();
    UnitModel* unit = node->GetComponent<UnitModel>();
    if (unit) {
      MeshGeometry* mesh = unit->structure_mesh();
      if (mesh) {
        Matrix3x4 inverse(node->GetWorldTransform().Inverse());
        Ray local_ray = ray.Transformed(inverse);

        SubObjectType sub_type;
        if (mesh->raycast(local_ray, sub_type, sotPOLYGON, true) >= 0) {
          return node;
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
  return nullptr;
}

/// Get ray from current mouse position and camera
Ray BaseContext::calculate_ray()
{
  UI* ui = GetSubsystem<UI>();
  IntVector2 pos = ui->GetCursorPosition();

  Graphics* graphics = GetSubsystem<Graphics>();
  Camera* camera = m_view->camera()->GetComponent<Camera>();
  return camera->GetScreenRay(
    (float)pos.x_ / graphics->GetWidth(), 
    (float)pos.y_ / graphics->GetHeight()
  );
}
