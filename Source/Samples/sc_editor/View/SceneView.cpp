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
#include "Urho3D\Graphics\Light.h"
#include "Urho3D\Graphics\Material.h"
#include "Urho3D\Graphics\Model.h"
#include "Urho3D\Graphics\Octree.h"
#include "Urho3D\Graphics\Skybox.h"
#include "Urho3D\Graphics\TextureCube.h"
#include "Urho3D\Graphics\Zone.h"
#include "Urho3D\Resource\ResourceCache.h"
#include "Urho3D\Scene\Node.h"
#include "Urho3D\Scene\Scene.h"
#include "Urho3D\Scene\SceneEvents.h"


using namespace Urho3D;

// Construct.
SceneView::SceneView(Context* context)
: Object(context)
{
  m_urho_scene = new Scene(context);
  Node* hull_node = m_urho_scene->CreateChild("Hull View", LOCAL);
  m_structure_view = hull_node->CreateComponent<StructureView>();

  //m_scene_root = m_urho_scene->CreateChild("Units root");
  // TODO: Create some helper structures
  // TODO: Setup environment
  ResourceCache* cache = GetSubsystem<ResourceCache>();

  // Create the Octree component to the scene so that drawable objects can be rendered. Use default volume
  // (-1000, -1000, -1000) to (1000, 1000, 1000)
  m_urho_scene->CreateComponent<Octree>();

  // Create a Zone for ambient light & fog control
  Node* zoneNode = m_urho_scene->CreateChild("Zone", LOCAL);
  Zone* zone = zoneNode->CreateComponent<Zone>();
  zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
  zone->SetFogColor(Color(0.2f, 0.2f, 0.2f));
  zone->SetFogStart(200.0f);
  zone->SetFogEnd(300.0f);
  zone->SetZoneTexture(cache->GetResource<TextureCube>("Textures/Spacebox.xml"));

  // Create a directional light
  Node* lightNode = m_urho_scene->CreateChild("DirectionalLight", LOCAL);
  lightNode->SetDirection(Vector3(-0.2f, -0.6f, -0.8f)); // The direction vector does not need to be normalized
  Light* light = lightNode->CreateComponent<Light>();
  light->SetLightType(LIGHT_DIRECTIONAL);
  light->SetColor(Color(1.0f, 0.95f, 0.8f));
  light->SetBrightness(7.0f);
  light->SetShadowBias(BiasParameters(0.0001f, 0.5f));
  light->SetShadowCascade(CascadeParameters(2.0f, 10.0f, 50.0f, 250.0f, 0.9f));
  //light->SetSpecularIntensity(1.5f);
  light->SetCastShadows(true);

  // Create skybox. 
  Node* skyNode = m_urho_scene->CreateChild("Sky", LOCAL);
  //skyNode->SetScale(500.0f); // The scale actually does not matter
  Skybox* skybox = skyNode->CreateComponent<Skybox>();
  skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
  skybox->SetMaterial(cache->GetResource<Material>("Materials/Spacebox.xml"));
}

// Destructor
SceneView::~SceneView()
{

}

// Get Urho3D scene behind
Scene* SceneView::scene()
{
  return m_urho_scene;
}

// Set scene model
void SceneView::set_model(SceneModel* model)
{
  if (!m_model) {
    UnsubscribeFromAllEvents();
  }
  m_model = model;
  SubscribeToEvent(
    model->scene(), 
    E_COMPONENTADDED, 
    URHO3D_HANDLER(SceneView, on_component_add)
  );
  SubscribeToEvent(
    model->scene(),
    E_COMPONENTREMOVED,
    URHO3D_HANDLER(SceneView, on_component_remove)
  );
  full_update();
}

// Update view scene completely
void SceneView::full_update()
{
  // TODO: go through all model scene nodes and create corresponding views
}

// New component event handler, listens for new units in scene model
void SceneView::on_component_add(StringHash eventType, VariantMap& eventData)
{
  Component* component = static_cast<Component*>(eventData[ComponentAdded::P_COMPONENT].GetPtr());
  if (component->IsInstanceOf<UnitModel>()) {
    dispatch_new_unit(static_cast<UnitModel*>(component));
  }
}

// Delete component event handler, listens for deleted units in scene model
void SceneView::on_component_remove(StringHash eventType, VariantMap& eventData)
{
  // TODO: implement
}

// Choose correct view for newly created component
void SceneView::dispatch_new_unit(UnitModel* unit)
{
  StringHash type = unit->GetType();
  if (NodeModel::GetTypeStatic() == type) {
    m_structure_view->add_node(static_cast<NodeModel*>(unit));
  }
}
