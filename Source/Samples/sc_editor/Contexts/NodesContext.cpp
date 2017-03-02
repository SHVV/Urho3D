// SHW Spacecraft editor
//
// Nodes manipulation context class, used for move, rotate ad clone nodes

#include "NodesContext.h"

// Includes from Editor
#include "../Model/NodeModel.h"
#include "../Model/SceneModel.h"
#include "../View/SceneView.h"

// Includes from Urho3D
#include "Urho3D/Graphics/Graphics.h"
#include "Urho3D/Graphics/Camera.h"
#include "Urho3D/Graphics/Octree.h"
#include "Urho3D/Graphics/Material.h"
#include "Urho3D/Graphics/StaticModel.h"
#include "Urho3D/Scene/Node.h"
#include "Urho3D/Scene/Scene.h"
#include "Urho3D/UI/UI.h"
#include "Urho3D/Input/Input.h"
#include "Urho3D/Resource/ResourceCache.h"

using namespace Urho3D;

/// Construct.
NodesContext::NodesContext(Context* context, SceneModel* model, SceneView* view)
: BaseContext(context, model, view)
{
  ResourceCache* cache = GetSubsystem<ResourceCache>();

  Node* gizmo_node = m_view->scene()->CreateChild("NodesGimbal", LOCAL);
  m_gizmo = gizmo_node->CreateComponent<StaticModel>();
  m_gizmo->SetModel(cache->GetResource<Model>("Models/Editor/Axes.mdl"));
  m_gizmo->SetMaterial(0, cache->GetResource<Material>("Materials/Editor/RedUnlit.xml"));
  m_gizmo->SetMaterial(1, cache->GetResource<Material>("Materials/Editor/GreenUnlit.xml"));
  m_gizmo->SetMaterial(2, cache->GetResource<Material>("Materials/Editor/BlueUnlit.xml"));
  m_gizmo->SetEnabled(false);
  //m_gizmo.viewMask = 0x80000000; // Editor raycasts use viewmask 0x7fffffff
  m_gizmo->SetOccludee(false);
}

/// Destructor
NodesContext::~NodesContext()
{
}

/// Activates context and allows it to set up all its guts
void NodesContext::activate()
{

}

/// Deactivate context and remove all temporary objects
void NodesContext::deactivate()
{
  m_gizmo->SetEnabled(false);
}

/// Mouse button down handler
void NodesContext::on_mouse_down()
{

}

/// Mouse button up handler
void NodesContext::on_mouse_up()
{
  Input* input = GetSubsystem<Input>();
  Node* node = get_unit_under_mouse();
  if (input->GetKeyDown(KEY_CTRL)) {
    if (node) {
      auto selected = m_view->selected();
      if (selected.Find(node) != selected.End()) {
        m_view->deselect(node);
      } else {
        m_view->select(node);
      }
    }
  } else {
    m_view->clear_selection();
    if (node) {
      m_view->select(node);
    }
  }
  auto selected = m_view->selected();
  if (selected.Size()) {
    m_gizmo->GetNode()->SetPosition(selected.Back()->GetPosition());
    m_gizmo->SetEnabled(true);
  } else {
    m_gizmo->SetEnabled(false);
  }
}

/// Mouse button move handler
void NodesContext::on_mouse_move(float x, float y)
{

}

/// Update context each frame
void NodesContext::update(float dt)
{
  UI* ui = GetSubsystem<UI>();
  Node* node = get_unit_under_mouse();
  if (node) {
    ui->GetCursor()->SetShape(CS_ACCEPTDROP);
  } else {
    ui->GetCursor()->SetShape(CS_NORMAL);
  }

  if (m_gizmo->IsEnabled()) {
    Camera* camera = m_view->camera()->GetComponent<Camera>();
    float scale = 0.1 / camera->GetZoom();

    Node* gizmo_node = m_gizmo->GetNode();
    if (camera->IsOrthographic()) {
      scale *= camera->GetOrthoSize();
    } else{
      scale *= (camera->GetView() * gizmo_node->GetPosition()).z_;
    }

    gizmo_node->SetScale(Vector3(scale, scale, scale));
  }
}
