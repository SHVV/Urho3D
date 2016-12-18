// SHW Spacecraft editor
//
// Main Model class, that holds complete scene

#pragma once

#include <Urho3D/Core/Object.h>

extern const char* EDITOR_CATEGORY;

// Editor predeclarations
class NodeModel;

// Urho predeclarations
namespace Urho3D {
  class Scene;
  class Node;
}

using namespace Urho3D;

class SceneModel : public Object {
  // Enable type information.
  URHO3D_OBJECT(SceneModel, Object);
public:

  /// Construct.
  SceneModel(Context* context);

  /// Destructor
  virtual ~SceneModel();

  // Generic access

  /// Returns pointer to the Urho scene behind
  Scene* scene();

  /// Returns material list

  /// Returns Shared properties list

  // Add object functions

  /// Add node (ball) into the scene
  NodeModel* create_node(const Vector3& position, Node* parent = nullptr);

  /// Add beam into the scene

  /// Add plate into the scene

  /// Add module into the scene

  // Query objects functions

  /// Raycast (include / exclude invisible)

  /// Box select

  // Delete functions

  /// Delete unit (delete all depndents, if necessary)

  // Serialization, Undo/Redo

  /// Save scene

  /// Load scene

  /// Commit transaction

  /// Undo

  /// Redo

private:

  /// Urho Scene
  SharedPtr<Scene> m_scene;
  /// Actual scene root for all units
  SharedPtr<Node> m_scene_root;
};

// TODO:
// - ModuleModel - drawable component;
//   - NodeModel - component of node, that is child of Module
//   - BeamModel - component of node, that is child of Module
//   - PlateModel - the same
//     each of them can be visible / invisible
// - HullModel - derived from Module 
// - GridHullModel - derived from Hull
// - MoveableModel - movable part of some module
// - FunctionalModel - module, that can do something smart
//   - ConnectionModel - child component of functional module
// - InstanceStorage - holds all modules instance parameters
// - MaterialStorage - holds all materials parameters, changed in editor
// 
// - EditorView - main view for editing scene
// - HullView - renders all children like plates, nodes and beams
// - ModuleView - renders module geometry
//
// - model - view - updates
// - wrappers for serializing links may be need to add automatic event subscriptions
// - wrappers for automating fireing events on property change
// - threaded geometry generator.
// - check for recursive updates.- 
//
// - grid - off grid symmetry 
//
// - use network replication mechanism for undo/redo
// - use Node::GetVars() for storing variable list of attributes, for example material attrbutes and procedural parameters.
// - also, check VAR_VARIANTMAP attribute type.
// - use CreateChild, CreateComponent (LOCAL) for non replicable (non-undoable) components and nodes.
// - use AddListener to listen node dirty transformation updates
// - use Events for notifing view of property changes.
// 
