// SHW Spacecraft editor
//
// Main Model class, that holds complete scene

#include "SceneModel.h"

// Includes from Editor
#include "../Model/NodeModel.h"

// Includes from Urho3D
#include "Urho3D\Graphics\Material.h"
#include "Urho3D\Resource\ResourceCache.h"
#include "Urho3D\Scene\Node.h"
#include "Urho3D\Scene\Scene.h"

using namespace Urho3D;

const char* EDITOR_CATEGORY = "SC_Editor";
static WeakPtr<SceneModel> s_scene_model;

// Construct.
SceneModel::SceneModel(Context* context, Node* model_root)
  : Object(context),
    m_scene_root(model_root)
{
  s_scene_model = this;

  // scene nodes and guts
  m_scene_root = model_root;
  m_scene = m_scene_root->GetScene();

  // Setup materials
  // TODO: use external storage for loading

  ResourceCache* cache = GetSubsystem<ResourceCache>();

  /// Default materials
  m_default_vertex_material = 
    cache->GetResource<Material>("Materials/GSPointsToSpheresMat.xml");
  m_default_edge_material =
    cache->GetResource<Material>("Materials/GSLinesToBeamsMat.xml");
  m_default_polygon_material = 
    //cache->GetResource<Material>("Materials/PBR/MetallicRough5.xml");
    //cache->GetResource<Material>("Materials/Stone.xml");
    //cache->GetResource<Material>("Materials/MetallicRough5SHW.xml");
    //cache->GetResource<Material>("Materials/Concrete.xml");
    //cache->GetResource<Material>("Materials/Plaster0.xml");
    //cache->GetResource<Material>("Materials/Plaster1.xml");
    //cache->GetResource<Material>("Materials/Paint.xml");
    cache->GetResource<Material>("Materials/SimplePanels.xml");

  /// Materials maps
  //m_vertex_materials;
  //m_edge_materials;
  //m_polygon_materials;

  //m_scene = new Scene(context);
  //m_scene_root = m_scene->CreateChild("Units root");

  // TODO: Setup Helpers for queries (octree or physics world)
  // TODO: Setup material storage
  // TODO: Setup shared properties storage
}

// Destructor
SceneModel::~SceneModel()
{
}

/// Returns global SceneModel
SceneModel* SceneModel::get()
{
  return s_scene_model;
}

// Returns pointer to the Urho scene behind
Scene* SceneModel::scene()
{
  return m_scene;
}

/// Returns material by name
Material* SceneModel::get_vertex_material(StringHash key)
{
  return get_material(key, m_vertex_materials, m_default_vertex_material);
}

Material* SceneModel::get_edge_material(StringHash key)
{
  return get_material(key, m_edge_materials, m_default_edge_material);
}

Material* SceneModel::get_polygon_material(StringHash key)
{
  return get_material(key, m_polygon_materials, m_default_polygon_material);
}

/// Get material from custom set
Material* SceneModel::get_material(
  StringHash key,
  const HashMap<StringHash, SharedPtr<Material>>& materials,
  Material* default_material
)
{
  SharedPtr<Material> result;
  if (!materials.TryGetValue(key, result)) {
    return default_material;
  }
  return result;
}

/// Add Unit
UnitModel* SceneModel::create_unit(
  StringHash class_name,
  const Vector3& position,
  const Vector3& normal,
  Node* parent
)
{
  Node* actual_parent = parent ? parent : m_scene_root;
  
  Vector3 local_position = actual_parent->WorldToLocal(position);
  Node* new_node = actual_parent->CreateChild("node");
  new_node->SetPosition(local_position);
  Component* new_unit = new_node->CreateComponent(class_name);
  
  if (new_unit->IsInstanceOf<UnitModel>()) {
    // Pass normal as reference direction
    return static_cast<UnitModel*>(new_unit);
  }
  
  return nullptr;
}

/// Delete unit
void SceneModel::delete_unit(Node* unit)
{
  unit->Remove();
  // TODO: probably need to deal with dependent nodes, 
  // TODO: probably move child nodes to parent or root
}

// Add node (ball) into the scene
//NodeModel* SceneModel::create_node(const Vector3& position, Node* parent)
//{
//  Node* actual_parent = parent ? parent : m_scene_root;
//
//  Vector3 local_position = actual_parent->WorldToLocal(position);
//  Node* new_node = actual_parent->CreateChild("node");
//  new_node->SetPosition(local_position);
//  NodeModel* new_joint = new_node->CreateComponent<NodeModel>();
//
//  // TODO: Setup material and other guts
//  // TODO: may be fire event here 
//
//  return new_joint;
//}
