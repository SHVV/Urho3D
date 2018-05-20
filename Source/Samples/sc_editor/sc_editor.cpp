// SHW Spacecraft editor
//
// Main application class

#include "sc_editor.h"

// Includes from editor
#include "Core/MeshGenerator.h"

#include "Model/UnitModel.h"
#include "Model/ProceduralUnit.h"
#include "Model/MultiPointUnit.h"
#include "Model/NodeModel.h"
#include "Model/SceneModel.h"
#include "Model/DynamicModel.h"
#include "Model/BasePositioner.h"
#include "Model/BaseAttachableSurface.h"
#include "Model/SurfaceSurfacePositioner.h"
#include "Model/SurfaceSurfaceAutoLinkUnit.h"
#include "Model/SurfaceNodePositioner.h"
#include "Model/SurfaceMount.h"
#include "Model/Voxel1DAttachableSurface.h"

#include "Contexts/BaseContext.h"
#include "Contexts/NodesContext.h"
#include "Contexts/CreationContext.h"

#include "View/SceneView.h"
#include "View/EditorUI.h"
#include "View/CameraController.h"

#include "MeshGenerators/TestGenerator.h"
#include "MeshGenerators/BaseTruss.h"
#include "MeshGenerators/WingRadiator.h"
#include "MeshGenerators/BeamAdapter.h"
#include "MeshGenerators/ConnectionTruss.h"

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
  time_(0.0f),
  m_symmetry(1)
{
  m_exit_on_esc = false;
  // Register all editor factories and objects

  // Mesh generator subsystem
  auto generator = new MeshGenerator(context_);
  context_->RegisterSubsystem(generator);
  // Register generator functions
  generator->add_function(new TestGenerator());
  generator->add_function(new BaseTruss());
  generator->add_function(new WingRadiator());
  generator->add_function(new BeamAdapter());
  generator->add_function(new ConnectionTruss());
  // TODO:

  // Model
  UnitModel::RegisterObject(context);
  ProceduralUnit::RegisterObject(context);
  MultiPointUnit::RegisterObject(context);
  BasePositioner::RegisterObject(context);
  BaseAttachableSurface::RegisterObject(context);
  SurfaceSurfacePositioner::RegisterObject(context);
  SurfaceSurfaceAutoLinkUnit::RegisterObject(context);
  SurfaceNodePositioner::RegisterObject(context);
  SurfaceMount::RegisterObject(context);
  Voxel1DAttachableSurface::RegisterObject(context);


  // Components
  DynamicModel::RegisterObject(context);

  // View components
  //StructureView::RegisterObject(context);
}

void SCEditor::Setup()
{
  Sample::Setup();
  engineParameters_["WindowResizable"] = true;
  engineParameters_["RenderPath"] = "RenderPaths/PBRDeferredHWDepthSHW.xml";
  //engineParameters_["RenderPath"] = "RenderPaths/PBRDeferredHWDepth.xml";
}

void SCEditor::Start()
{
  // Execute base class startup
  Sample::Start();

  // Create the scene content
  CreateScene();

  // Create the UI content
  CreateInstructions();

  // Hook up to the frame update events
  SubscribeToEvents();

  // Set the mouse mode to use in the sample
  Sample::InitMouseMode(MM_FREE);
}

/// Returns scene model
SceneModel* SCEditor::model()
{
  return m_model;
}

/// Returns scene view
SceneView* SCEditor::view()
{
  return m_view;
}

/// Returns EditorUI
EditorUI* SCEditor::ui()
{
  return m_editor_ui;
}

/// Returns context switcher
ContextToolbar* SCEditor::context_toolbar()
{
  return m_editor_ui->context_toolbar();
}

/// Set current editor's context
void SCEditor::set_context(BaseContext* context)
{
  if (m_context) {
    m_context->deactivate();
  }
  m_context = context;
  if (m_context) {
    m_context->set_symmetry(m_symmetry);
    m_context->activate();
  }
}

/// Set symmetry
void SCEditor::set_symmetry(int value)
{
  if (value < 1) {
    value = 1;
  }
  if (m_symmetry != value) {
    m_symmetry = value;
    if (m_context) {
      m_context->set_symmetry(value);
    }
  }
}

void SCEditor::CreateScene()
{
  // Create Model-View
  m_view = new SceneView(context_);
  //m_view->set_model(m_model);
  scene_ = m_view->scene();
  m_model = new SceneModel(context_, scene_->CreateChild("Units root"));

  // Create editor UI
  m_editor_ui = new EditorUI(context_, this);

  // Create default camera controller
  // TODO: probably we need to move this into view and allow to switch on demand
  m_camera_controller = new CameraController(context_, m_view);

  //Node* node1 = m_model->scene_root()->CreateChild();
  //node1->SetWorldPosition(Vector3(0, 0, 10));
  //Node* node2 = m_model->scene_root()->CreateChild();
  //node2->SetWorldPosition(Vector3(0, 100, 40));

  //MultiPointUnit* test_unit = static_cast<MultiPointUnit*>(m_model->create_unit(
  //  MultiPointUnit::GetTypeStatic(),
  //  Vector3(0, 0, 0)
  //));
  //test_unit->set_function_name(ConnectionTruss::s_name);
  //test_unit->set_reference_node(0, node1);
  //test_unit->set_reference_node(1, node2);

  //ProceduralUnit* test_unit = static_cast<ProceduralUnit*>(m_model->create_unit(
  //  ProceduralUnit::GetTypeStatic(),
  //  Vector3(0, 0, 0)
  //));
  //test_unit->set_function_name(BeamAdapter::s_name);

  // Create test context
  /*m_nodes_context = new NodesContext(context_, this);
  m_creation_context = new CreationContext(context_, this);
  //set_context(m_nodes_context);
  set_context(m_creation_context);
  Parameters context_parameters;
  context_parameters[s_unit_class] = ProceduralUnit::GetTypeStatic();
  context_parameters[s_function_name] = StringHash(TestGenerator::s_name());
  m_creation_context->set_parameters(context_parameters);*/
  // Create test guts
  //for (int i = 0; i < 6; ++i) {
  //  float s, c;
  //  SinCos(i * 360.0 / 6, s, c);
  //  ProceduralUnit* test_unit = static_cast<ProceduralUnit*>(m_model->create_unit(
  //    ProceduralUnit::GetTypeStatic(),
  //    Vector3(40 * s, 40 * c, 0)
  //  ));
  //  test_unit->set_function_name(TestGenerator::s_name());
  //}
}

void SCEditor::CreateInstructions()
{
  SetLogoVisible(false); // We need the full rendering window
  ResourceCache* cache = GetSubsystem<ResourceCache>();
  Graphics* graphics = GetSubsystem<Graphics>();

  graphics->SetWindowIcon(0);
  graphics->SetWindowTitle("Spacecraft Constructor");
}

void SCEditor::SubscribeToEvents()
{
  // Subscribe HandleUpdate() function for processing update events
  SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(SCEditor, HandleUpdate));

  SubscribeToEvent(E_MOUSEMOVE, URHO3D_HANDLER(SCEditor, HandleMouseMove));
  SubscribeToEvent(E_MOUSEBUTTONUP, URHO3D_HANDLER(SCEditor, HandleMouseUp));
  SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(SCEditor, HandleMouseDown));

  // Post render handler
  SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(SCEditor, PostRenderUpdate));
}


void SCEditor::AnimateObjects(float timeStep)
{
  URHO3D_PROFILE(AnimateObjects);

  time_ += timeStep * 100.0f;
}

/// Handle the Mouse move.
void SCEditor::HandleMouseMove(StringHash eventType, VariantMap& eventData)
{
  using namespace MouseMove;
  if (m_context && is_mouse_free()) {
    Input* input = GetSubsystem<Input>();
    if (!(input->GetMouseButtonDown(MOUSEB_MIDDLE) || input->GetMouseButtonDown(MOUSEB_RIGHT))) {
      m_context->on_mouse_move(
        eventData[P_X].GetInt(),
        eventData[P_Y].GetInt()
      );
    }
  }
}

/// Handle the Mouse button down.
void SCEditor::HandleMouseDown(StringHash eventType, VariantMap& eventData)
{
  using namespace MouseButtonDown;
  if (is_mouse_free()) {
    if (m_context && eventData[P_BUTTON].GetInt() == MOUSEB_LEFT) {
      m_context->on_mouse_down();
    } else if (m_camera_controller) {
      // TODO: hide cursor and fade or hide UI on camera control
      m_camera_controller->on_mouse_down(eventData);
    }
  }
}

/// Handle the Mouse button up.
void SCEditor::HandleMouseUp(StringHash eventType, VariantMap& eventData)
{
  using namespace MouseButtonUp;
  if (m_context && is_mouse_free() && eventData[P_BUTTON].GetInt() == MOUSEB_LEFT) {
    m_context->on_mouse_up();
  }
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

  // Update scene view
  m_view->update(timeStep);

  // Update current context
  if (m_context && is_mouse_free()) {
    m_context->update(timeStep);
  }

  if (m_camera_controller) {
    m_camera_controller->update(timeStep);
  }

  if (m_editor_ui) {
    m_editor_ui->update(timeStep);
  }

  // Animate objects' if enabled
  if (animate_) {
    AnimateObjects(timeStep);
  }
}

/// Handle the post rendering event.
void SCEditor::PostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
  // Render some debug geometry
  m_view->post_render();
}

/// Check, that mouse is not on the UI
bool SCEditor::is_mouse_free()
{
  UI* ui = GetSubsystem<UI>();
  IntVector2 pos = ui->GetCursorPosition();
  // Check the cursor is visible and there is no UI element in front of the cursor
  return ui->GetCursor()->IsVisible() && !ui->GetElementAt(pos, false);
}
