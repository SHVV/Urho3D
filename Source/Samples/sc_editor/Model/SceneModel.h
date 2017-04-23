// SHW Spacecraft editor
//
// Main Model class, that holds complete scene

#pragma once

#include <Urho3D/Core/Object.h>

extern const char* EDITOR_CATEGORY;

// Editor predeclarations
class NodeModel;
class UnitModel;

// Urho predeclarations
namespace Urho3D {
  class Scene;
  class Node;
  class Material;
}

using namespace Urho3D;

class SceneModel : public Object {
  // Enable type information.
  URHO3D_OBJECT(SceneModel, Object);
public:

  /// Construct.
  SceneModel(Context* context, Node* model_root);

  /// Destructor
  virtual ~SceneModel();

  // Generic access
  // TODO: refactor
  /// Returns global SceneModel
  static SceneModel* get();

  /// Returns pointer to the Urho scene behind
  Scene* scene();

  /// Returns material by name
  Material* get_vertex_material(StringHash);
  Material* get_edge_material(StringHash);
  Material* get_polygon_material(StringHash);

  /// Returns Shared properties list

  // Add object functions
  /// Add Unit
  UnitModel* create_unit(
    StringHash class_name,
    const Vector3& position = Vector3(0, 0, 0),
    const Vector3& normal = Vector3(0, 0, 1),
    Node* parent = nullptr
  );

  /// Add node (ball) into the scene
  //NodeModel* create_node(const Vector3& position, Node* parent = nullptr);

  /// Add beam into the scene

  /// Add plate into the scene

  /// Add module into the scene

  // Query objects functions

  /// Raycast (include / exclude invisible)

  /// Box select

  // Delete functions
  /// Delete unit
  void delete_unit(Node* unit);

  /// Delete unit (delete all depedents, if necessary)

  // Serialization, Undo/Redo

  /// Save scene

  /// Load scene

  /// Commit transaction

  /// Rollback transaction

  /// Undo

  /// Redo

private:
  /// Get material from custom set
  Material* get_material(
    StringHash key, 
    const HashMap<StringHash, SharedPtr<Material>>& materials,
    Material* default_material
  );

  /// Urho Scene
  WeakPtr<Scene> m_scene;
  /// Actual scene root for all units
  WeakPtr<Node> m_scene_root;

  /// Materials maps
  HashMap<StringHash, SharedPtr<Material>> m_vertex_materials;
  HashMap<StringHash, SharedPtr<Material>> m_edge_materials;
  HashMap<StringHash, SharedPtr<Material>> m_polygon_materials;

  /// Default materials
  SharedPtr<Material> m_default_vertex_material;
  SharedPtr<Material> m_default_edge_material;
  SharedPtr<Material> m_default_polygon_material;
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
