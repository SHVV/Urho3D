// SHW Spacecraft editor
//
// Main Model class, that holds complete scene

#include "SceneModel.h"

// Includes from Editor
#include "../Model/NodeModel.h"

// Includes from Urho3D
#include "Urho3D\Scene\Node.h"
#include "Urho3D\Scene\Scene.h"

using namespace Urho3D;

const char* EDITOR_CATEGORY = "SC_Editor";

// Construct.
SceneModel::SceneModel(Context* context)
  : Object(context)
{
  m_scene = new Scene(context);
  m_scene_root = m_scene->CreateChild("Units root");

  // TODO: Setup Helpers for queries (octree or physics world)
  // TODO: Setup material storage
  // TODO: Setup shared properties storage
}

// Destructor
SceneModel::~SceneModel()
{
}

// Returns pointer to the Urho scene behind
Scene* SceneModel::scene()
{
  return m_scene;
}

// Add node (ball) into the scene
NodeModel* SceneModel::create_node(const Vector3& position, Node* parent)
{
  Node* actual_parent = parent ? parent : m_scene_root;

  Vector3 local_position = actual_parent->WorldToLocal(position);
  Node* new_node = actual_parent->CreateChild("node");
  new_node->SetPosition(local_position);
  NodeModel* new_joint = new_node->CreateComponent<NodeModel>();

  // TODO: Setup material and other guts
  // TODO: may be fire event here 

  return new_joint;
}
