// SHW Spacecraft editor
//
// Main application class

#include "sc_editor.h"

// Includes from editor
#include "Model/UnitModel.h"
#include "Model/NodeModel.h"
#include "Model/SceneModel.h"

#include "View/SceneView.h"
#include "View/StructureView.h"

// Includes from Urho
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/Profiler.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/IndexBuffer.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/RenderPath.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Skybox.h>
#include <Urho3D/Graphics/TextureCube.h>
#include <Urho3D/Graphics/VertexBuffer.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>

#include <Urho3D/DebugNew.h>

URHO3D_DEFINE_APPLICATION_MAIN(SCEditor)

SCEditor::SCEditor(Context* context)
: Sample(context),
  animate_(true),
  time_(0.0f)
{
  // Register all editor factories and objects

  // Model
  UnitModel::RegisterObject(context);
  NodeModel::RegisterObject(context);

  // View components
  StructureView::RegisterObject(context);
}

void SCEditor::Setup()
{
  Sample::Setup();
  engineParameters_["WindowResizable"] = true;
  engineParameters_["RenderPath"] = "RenderPaths/PBRDeferredHWDepthSHW.xml";
}

void SCEditor::Start()
{
  // Execute base class startup
  Sample::Start();

  // Create the scene content
  CreateScene();

  // Create the UI content
  CreateInstructions();

  // Setup the viewport for displaying the scene
  SetupViewport();

  // Hook up to the frame update events
  SubscribeToEvents();

  // Set the mouse mode to use in the sample
  Sample::InitMouseMode(MM_FREE);
}

void SCEditor::CreateScene()
{
  // Create Model-View
  m_model = new SceneModel(context_);
  m_view = new SceneView(context_);
  m_view->set_model(m_model);
  scene_ = m_view->scene();

  // Create the camera
  cameraNode_ = new Node(context_);
  cameraNode_->SetPosition(Vector3(0.0f, 2.0f, -20.0f));
  Camera* camera = cameraNode_->CreateComponent<Camera>();
  camera->SetFarClip(300.0f);

  // Create test guts
  NodeModel* node = m_model->create_node(Vector3(0, 0, 0));
  node->GetNode()->Translate(Vector3(1, 2, 3));
  node->set_radius(0.1);
  node = m_model->create_node(Vector3(1, 1, 3));
  node->set_radius(0.2);
  node = m_model->create_node(Vector3(1, 0, 3));
  node->set_radius(0.3);
}

void SCEditor::CreateInstructions()
{
  SetLogoVisible(false); // We need the full rendering window
  ResourceCache* cache = GetSubsystem<ResourceCache>();
  Graphics* graphics = GetSubsystem<Graphics>();

  graphics->SetWindowIcon(0);
  graphics->SetWindowTitle("Spacecraft Constructor");
}

void SCEditor::SetupViewport()
{
  Renderer* renderer = GetSubsystem<Renderer>();
  renderer->SetHDRRendering(true);
  renderer->SetShadowMapSize(2048);
  renderer->SetDrawShadows(true);

  Engine* engine = GetSubsystem<Engine>();
  engine->SetMaxFps(0);

  // Set up a viewport to the Renderer subsystem so that the 3D scene can be seen
  SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
  renderer->SetViewport(0, viewport);

  // Add post-processing effects appropriate with the example scene
  ResourceCache* cache = GetSubsystem<ResourceCache>();
  SharedPtr<RenderPath> effectRenderPath = viewport->GetRenderPath()->Clone();
  //effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/GammaCorrection.xml"));
  effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/AutoExposure.xml"));
  effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/FXAA3.xml"));
  effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/BloomHDR.xml"));

  viewport->SetRenderPath(effectRenderPath);
}

void SCEditor::SubscribeToEvents()
{
  // Subscribe HandleUpdate() function for processing update events
  SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(SCEditor, HandleUpdate));
}

void SCEditor::MoveCamera(float timeStep)
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
    yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
    pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
    pitch_ = Clamp(pitch_, -90.0f, 90.0f);
    // Construct new orientation for the camera scene node from yaw and pitch. Roll is fixed to zero
    cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));
    cameraNode_->Translate((Vector3::LEFT * mouseMove.x_ + Vector3::UP * mouseMove.y_) * 0.01);
  }

  if (input->GetMouseButtonDown(MOUSEB_MIDDLE)) {
    cameraNode_->Translate((Vector3::LEFT * mouseMove.x_ + Vector3::UP * mouseMove.y_) * MOUSE_SENSITIVITY_UNITS);
  }
  cameraNode_->Translate((Vector3::FORWARD * input->GetMouseMoveWheel()) * MOUSE_WHEEL_SPEED);

  // Read WASD keys and move the camera scene node to the corresponding direction if they are pressed
  if (input->GetKeyDown(KEY_W))
    cameraNode_->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
  if (input->GetKeyDown(KEY_S))
    cameraNode_->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
  if (input->GetKeyDown(KEY_A))
    cameraNode_->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
  if (input->GetKeyDown(KEY_D))
    cameraNode_->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);
}

void SCEditor::AnimateObjects(float timeStep)
{
  URHO3D_PROFILE(AnimateObjects);

  time_ += timeStep * 100.0f;
}

void SCEditor::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
  using namespace Update;

  // Take the frame time step, which is stored as a float
  float timeStep = eventData[P_TIMESTEP].GetFloat();

  // Toggle animation with space
  Input* input = GetSubsystem<Input>();
  if (input->GetKeyPress(KEY_SPACE)) {
    animate_ = !animate_;
  }

  // Move the camera, scale movement with time step
  MoveCamera(timeStep);

  // Animate objects' if enabled
  if (animate_) {
    AnimateObjects(timeStep);
  }
}
