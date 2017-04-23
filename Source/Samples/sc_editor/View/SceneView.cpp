// SHW Spacecraft editor
//
// Main View class, that shows complete scene

#include "SceneView.h"

// Includes from Editor
#include "../Model/UnitModel.h"
#include "../Model/NodeModel.h"
#include "../Model/SceneModel.h"
#include "../View/StructureView.h"

// Includes from Urho3D
#include "Urho3D\Engine\Engine.h"
#include "Urho3D\Graphics\Camera.h"
#include "Urho3D/Graphics/CustomGeometry.h"
#include "Urho3D/Graphics/DebugRenderer.h"
#include "Urho3D\Graphics\Light.h"
#include "Urho3D\Graphics\Material.h"
#include "Urho3D\Graphics\Model.h"
#include "Urho3D\Graphics\Octree.h"
#include "Urho3D\Graphics\Renderer.h"
#include "Urho3D\Graphics\RenderPath.h"
#include "Urho3D\Graphics\Skybox.h"
#include "Urho3D\Graphics\TextureCube.h"
#include "Urho3D\Graphics\Zone.h"
#include "Urho3D\Input\Input.h"
#include "Urho3D\Resource\ResourceCache.h"
#include "Urho3D\Resource\XMLFile.h"
#include "Urho3D\Scene\Node.h"
#include "Urho3D\Scene\Scene.h"
#include "Urho3D\Scene\SceneEvents.h"
#include "Urho3D\UI\UI.h"

using namespace Urho3D;

// Construct.
SceneView::SceneView(Context* context)
: Object(context),
  m_yaw(0),
  m_pitch(0)
{
  m_urho_scene = new Scene(context);
  //Node* hull_node = m_urho_scene->CreateChild("Hull View", LOCAL);
  //m_structure_view = hull_node->CreateComponent<StructureView>();

  //m_scene_root = m_urho_scene->CreateChild("Units root");
  // TODO: Create some helper structures
  // TODO: Setup environment
  ResourceCache* cache = GetSubsystem<ResourceCache>();

  // Create the Octree component to the scene so that drawable objects can be rendered. Use default volume
  // (-1000, -1000, -1000) to (1000, 1000, 1000)
  m_urho_scene->CreateComponent<Octree>();
  m_urho_scene->CreateComponent<DebugRenderer>();

  // Create a Zone for ambient light & fog control
  Node* zoneNode = m_urho_scene->CreateChild("Zone", LOCAL);
  zoneNode->SetTemporary(true);
  Zone* zone = zoneNode->CreateComponent<Zone>();
  zone->SetBoundingBox(BoundingBox(-2000.0f, 2000.0f));
  zone->SetFogColor(Color(0.0f, 0.0f, 0.0f));
  zone->SetFogStart(2000.0f);
  zone->SetFogEnd(3000.0f);
  zone->SetZoneTexture(cache->GetResource<TextureCube>("Textures/Spacebox.xml"));

  // Create a directional light
  Node* lightNode = m_urho_scene->CreateChild("DirectionalLight", LOCAL);
  lightNode->SetTemporary(true);
  lightNode->SetDirection(Vector3(-0.2f, -0.6f, -0.8f)); // The direction vector does not need to be normalized
  Light* light = lightNode->CreateComponent<Light>();
  light->SetLightType(LIGHT_DIRECTIONAL);
  light->SetColor(Color(1.0f, 0.95f, 0.8f));
  light->SetBrightness(7.0f);
  light->SetShadowBias(BiasParameters(0.0001f, 0.1f));
  light->SetShadowCascade(CascadeParameters(16.0f, 80.0f, 400.0f, 2000.0f, 0.9f));
  //light->SetSpecularIntensity(1.5f);
  light->SetCastShadows(true);

  // Create skybox. 
  Node* skyNode = m_urho_scene->CreateChild("Sky", LOCAL);
  skyNode->SetTemporary(true);
  //skyNode->SetScale(500.0f); // The scale actually does not matter
  Skybox* skybox = skyNode->CreateComponent<Skybox>();
  skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
  skybox->SetMaterial(cache->GetResource<Material>("Materials/Spacebox.xml"));

  // Create main axis
  m_main_axis = m_urho_scene->CreateChild("MainAxis", LOCAL);
  m_main_axis->SetTemporary(true);
  auto axis = m_main_axis->CreateComponent<CustomGeometry>();
  axis->SetNumGeometries(1);
  // TODO: proper material
  axis->SetMaterial(cache->GetResource<Material>("Materials/VColUnlit.xml"));
  axis->SetViewMask(0x80000000); // Editor raycasts use viewmask 0x7fffffff
  axis->SetOccludee(false);

  axis->BeginGeometry(0, LINE_LIST);

  Color axis_color(1.0, 1.0, 1.0);
  axis->DefineVertex(Vector3(0.0, 0.0, 1000));
  axis->DefineColor(axis_color);
  axis->DefineVertex(Vector3(0.0, 0.0, -1000));
  axis->DefineColor(axis_color);

  axis->DefineVertex(Vector3(0.0, 3.0, 0));
  axis->DefineColor(axis_color);
  axis->DefineVertex(Vector3(0.0, 0.0, 10));
  axis->DefineColor(axis_color);
  axis->DefineVertex(Vector3(0.0, 0.0, 10));
  axis->DefineColor(axis_color);
  axis->DefineVertex(Vector3(0.0, -3.0, 0));
  axis->DefineColor(axis_color);
  axis->DefineVertex(Vector3(3.0, 0.0, 0));
  axis->DefineColor(axis_color);
  axis->DefineVertex(Vector3(0.0, 0.0, 10));
  axis->DefineColor(axis_color);
  axis->DefineVertex(Vector3(0.0, 0.0, 10));
  axis->DefineColor(axis_color);
  axis->DefineVertex(Vector3(-3.0, 0.0, 0));
  axis->DefineColor(axis_color);

  for (int i = -1000; i <= 1000; i += 10) {
    float size = (i % 100 == 0) ? 3 : 1;
    axis->DefineVertex(Vector3(-size, 0.0, i));
    axis->DefineColor(axis_color);
    axis->DefineVertex(Vector3(size, 0.0, i));
    axis->DefineColor(axis_color);

    axis->DefineVertex(Vector3(0.0, -size, i));
    axis->DefineColor(axis_color);
    axis->DefineVertex(Vector3(0.0, size, i));
    axis->DefineColor(axis_color);
  }

  axis->Commit();

  setup_viewport();
}

// Destructor
SceneView::~SceneView()
{

}

void SceneView::setup_viewport()
{
  UI* ui = GetSubsystem<UI>();
  ResourceCache* cache = GetSubsystem<ResourceCache>();

  XMLFile* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
  SharedPtr<Cursor> cursor(new Cursor(context_));
  cursor->SetStyleAuto(style);
  cursor->SetUseSystemShapes(true);
  ui->SetCursor(cursor);

  // Create the camera
  m_camera_node = new Node(context_);
  m_camera_node->SetPosition(Vector3(-100.0f, 100.0f, 100.0f));
  m_camera_node->LookAt(Vector3());
  Camera* camera = m_camera_node->CreateComponent<Camera>();
  camera->SetFarClip(2000.0f);

  Renderer* renderer = GetSubsystem<Renderer>();
  renderer->SetHDRRendering(true);
  renderer->SetShadowMapSize(2048);
  renderer->SetDrawShadows(true);

  Engine* engine = GetSubsystem<Engine>();
  engine->SetMaxFps(0);

  // Set up a viewport to the Renderer subsystem so that the 3D scene can be seen
  SharedPtr<Viewport> viewport(new Viewport(context_, m_urho_scene, m_camera_node->GetComponent<Camera>()));
  renderer->SetViewport(0, viewport);

  // Add post-processing effects appropriate with the example scene
  SharedPtr<RenderPath> effectRenderPath = viewport->GetRenderPath()->Clone();
  //effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/ColorCorrection.xml"));
  //effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/GammaCorrection.xml"));
  //effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/AutoExposure.xml"));
  effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/FXAA3.xml"));
  effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/BloomHDR.xml"));
  effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/GammaCorrection.xml"));

  viewport->SetRenderPath(effectRenderPath);
}

// Get Urho3D scene behind
Scene* SceneView::scene()
{
  return m_urho_scene;
}

// Get current camera node
Node* SceneView::camera()
{
  return m_camera_node;
}

/// Selects node in view
void SceneView::select(Node* node)
{
  if (m_selected.Find(node) == m_selected.End()) {
    m_selected.Push(node);
  }
}

/// Deselects node in view
void SceneView::deselect(Node* node)
{
  m_selected.Remove(node);
}

/// Deselects all node in view
void SceneView::clear_selection()
{
  m_selected.Clear();
}

/// Returns selected nodes
const PODVector<Node*>& SceneView::selected()
{
  return m_selected;
}

// Set scene model
//void SceneView::set_model(SceneModel* model)
//{
//  if (!m_model) {
//    UnsubscribeFromAllEvents();
//  }
//  m_model = model;
//  SubscribeToEvent(
//    model->scene(), 
//    E_COMPONENTADDED, 
//    URHO3D_HANDLER(SceneView, on_component_add)
//  );
//  SubscribeToEvent(
//    model->scene(),
//    E_COMPONENTREMOVED,
//    URHO3D_HANDLER(SceneView, on_component_remove)
//  );
//  full_update();
//}

// Update each frame
void SceneView::update(float dt)
{
  //move_camera(dt);
}

void SceneView::move_camera(float dt)
{
  // Do not move if the UI has a focused element (the console)
  if (GetSubsystem<UI>()->GetFocusElement())
    return;

  Input* input = GetSubsystem<Input>();

  // Movement speed as world units per second
  const float MOVE_SPEED = 20.0f;
  // Mouse sensitivity as degrees per pixel
  const float MOUSE_SENSITIVITY = 0.1f;
  // Mouse sensitivity as units per pixel
  const float MOUSE_SENSITIVITY_UNITS = 0.02f;
  // Mouse wheel sensitivity as units per pixel
  const float MOUSE_WHEEL_SPEED = 0.4f;

  // Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
  IntVector2 mouseMove = input->GetMouseMove();
  if (input->GetMouseButtonDown(MOUSEB_RIGHT)) {
    m_yaw += MOUSE_SENSITIVITY * mouseMove.x_;
    m_pitch += MOUSE_SENSITIVITY * mouseMove.y_;
    m_pitch = Clamp(m_pitch, -90.0f, 90.0f);
    // Construct new orientation for the camera scene node from yaw and pitch. Roll is fixed to zero
    m_camera_node->SetRotation(Quaternion(m_pitch, m_yaw, 0.0f));
    //cameraNode_->Translate((Vector3::LEFT * mouseMove.x_ + Vector3::UP * mouseMove.y_) * 0.01);
  }

  if (input->GetMouseButtonDown(MOUSEB_MIDDLE)) {
    m_camera_node->Translate((Vector3::LEFT * mouseMove.x_ + Vector3::UP * mouseMove.y_) * MOUSE_SENSITIVITY_UNITS);
  }
  m_camera_node->Translate((Vector3::FORWARD * input->GetMouseMoveWheel()) * MOUSE_WHEEL_SPEED);

  // Read WASD keys and move the camera scene node to the corresponding direction if they are pressed
  if (input->GetKeyDown(KEY_W))
    m_camera_node->Translate(Vector3::FORWARD * MOVE_SPEED * dt);
  if (input->GetKeyDown(KEY_S))
    m_camera_node->Translate(Vector3::BACK * MOVE_SPEED * dt);
  if (input->GetKeyDown(KEY_A))
    m_camera_node->Translate(Vector3::LEFT * MOVE_SPEED * dt);
  if (input->GetKeyDown(KEY_D))
    m_camera_node->Translate(Vector3::RIGHT * MOVE_SPEED * dt);
}

/// Post render update
void SceneView::post_render()
{
  auto debug_renderer = m_urho_scene->GetComponent<DebugRenderer>();
  for (int i = 0; i < m_selected.Size(); ++i) {
    debug_renderer->AddNode(m_selected[i], 2, false);
    auto drawable = m_selected[i]->GetDerivedComponent<Drawable>();
    if (drawable) {
      debug_renderer->AddBoundingBox(drawable->GetBoundingBox(), m_selected[i]->GetWorldTransform(), Color(1.0, 1.0, 1.0), false);
    }
  }
}

//// Update view scene completely
//void SceneView::full_update()
//{
//  // TODO: go through all model scene nodes and create corresponding views
//}
//
//// New component event handler, listens for new units in scene model
//void SceneView::on_component_add(StringHash eventType, VariantMap& eventData)
//{
//  Component* component = static_cast<Component*>(eventData[ComponentAdded::P_COMPONENT].GetPtr());
//  if (component->IsInstanceOf<UnitModel>()) {
//    dispatch_new_unit(static_cast<UnitModel*>(component));
//  }
//}
//
//// Delete component event handler, listens for deleted units in scene model
//void SceneView::on_component_remove(StringHash eventType, VariantMap& eventData)
//{
//  // TODO: implement
//}
//
//// Choose correct view for newly created component
//void SceneView::dispatch_new_unit(UnitModel* unit)
//{
//  StringHash type = unit->GetType();
//  //if (NodeModel::GetTypeStatic() == type) {
//  //  m_structure_view->add_node(static_cast<NodeModel*>(unit));
//  //}
//  // TODO: add other component types
//}
