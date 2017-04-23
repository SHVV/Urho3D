// SHW Spacecraft editor
//
// Main View class, that shows complete scene

#pragma once

#include <Urho3D/Core/Object.h>

// Editor predeclarations
class SceneModel;
class NodeModel;
class UnitModel;
class StructureView;

// Urho predeclarations
namespace Urho3D {
  class Context;
  class Scene;
  class Node;
}

using namespace Urho3D;

class SceneView : public Object {
  // Enable type information.
  URHO3D_OBJECT(SceneView, Object);
public:

  /// Construct.
  SceneView(Context* context);

  /// Destructor
  virtual ~SceneView();

  /// Get Urho3D scene behind
  Scene* scene();

  /// Get current camera node
  Node* camera();

  /// Selects node in view
  void select(Node* node);
  /// Deselects node in view
  void deselect(Node* node);
  /// Deselects all node in view
  void clear_selection();
  /// Returns selected nodes
  const PODVector<Node*>& selected();

  /// Set scene model
  //void set_model(SceneModel* model);

  ///// Update view scene completely
  //void full_update();

  /// Update each frame
  void update(float dt);
  /// Post render update
  void post_render();
private:
  /// Read input and move the camera.
  void move_camera(float dt);

  /// Set up a viewport for displaying the scene.
  void setup_viewport();

  ///// New component event handler, listens for new units in scene model
  //void on_component_add(StringHash eventType, VariantMap& eventData);
  ///// Delete component event handler, listens for deleted units in scene model
  //void on_component_remove(StringHash eventType, VariantMap& eventData);

  ///// Choose correct view for newly created component
  //void dispatch_new_unit(UnitModel* unit);

  /// Urho scene, used for view
  SharedPtr<Scene> m_urho_scene;
  /// Scene model, that we are viewing
  //SharedPtr<SceneModel> m_model;
  /// Structure view
  //SharedPtr<StructureView> m_structure_view;

  // TODO: Factor out camera controller
  /// Camera scene node.
  SharedPtr<Node> m_camera_node;
  /// Camera yaw angle.
  float m_yaw;
  /// Camera pitch angle.
  float m_pitch;

  /// Main axis
  SharedPtr<Node> m_main_axis;

  /// Selected nodes.
  // TODO: potentially dangerous, because could be deleted. 
  // It is better to replace them with WeakPtrs
  PODVector<Node*> m_selected;
};
