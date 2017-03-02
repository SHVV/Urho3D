// SHW Spacecraft editor
//
// Main application class

#include "sc_editor.h"

// Includes from editor
#include "Model/UnitModel.h"
#include "Model/NodeModel.h"
#include "Model/SceneModel.h"

#include "Contexts/BaseContext.h"
#include "Contexts/NodesContext.h"

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

void piramid(
  int n, float scale, const Vector3& pos,
  const Vector<Vector3>& base, MeshGeometry* mesh
)
{
  if (n) {
    // Recursion
    float new_scale = scale * 0.5;
    int new_n = n - 1;
    for (int i = 0; i < 4; ++i) {
      Vector3 new_pos = pos + base[i] * new_scale;
      piramid(new_n, new_scale, new_pos, base, mesh);
    }
  } else {
    // Terminal
    int indexies[4];
    // Vertecies
    for (int i = 0; i < 4; ++i) {
      indexies[i] = mesh->add(base[i] * scale + pos, 0.15 * scale);
    }
    // Edges
    for (int i = 0; i < 4; ++i) {
      for (int j = i + 1; j < 4; ++j) {
        mesh->add(indexies[i], indexies[j]);
      }
    }
    // Polygons
    mesh->add(indexies[0], indexies[1], indexies[2]);
    mesh->add(indexies[0], indexies[3], indexies[1]);
    mesh->add(indexies[1], indexies[3], indexies[2]);
    mesh->add(indexies[0], indexies[2], indexies[3]);
  }
}

void piramid(int level, float size, MeshGeometry* mesh)
{
  // Base
  Vector<Vector3> base;
  base.Push(Vector3(1, 1, -1));
  base.Push(Vector3(-1, -1, -1));
  base.Push(Vector3(-1, 1, 1));
  base.Push(Vector3(1, -1, 1));

  piramid(level, size, Vector3(0, 0, 0), base, mesh);
}

void SCEditor::CreateScene()
{
  // Create Model-View
  m_view = new SceneView(context_);
  //m_view->set_model(m_model);
  scene_ = m_view->scene();
  m_model = new SceneModel(context_, scene_->CreateChild("Units root"));

  // Create test context
  m_context = new NodesContext(context_, m_model, m_view);

  // Create test guts
  for (int i = 0; i < 6; ++i) {
    float s, c;
    SinCos(i * 360.0 / 6, s, c);
    UnitModel* test_unit = m_model->create_unit(
      UnitModel::GetTypeStatic(),
      Vector3(20 * s, 20 * c, 0)
    );
    MeshGeometry* mesh = test_unit->rendering_mesh();
    if (mesh) {
      piramid(2, 5, mesh);
    }
  }
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
    m_context->on_mouse_move(
      eventData[P_X].GetInt(),
      eventData[P_Y].GetInt()
    );
  }
}

/// Handle the Mouse button down.
void SCEditor::HandleMouseDown(StringHash eventType, VariantMap& eventData)
{
  using namespace MouseButtonDown;
  if (m_context && is_mouse_free() && eventData[P_BUTTON].GetInt() == MOUSEB_LEFT) {
    m_context->on_mouse_down();
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
  return ui->GetCursor()->IsVisible() && !ui->GetElementAt(pos, true);
}
