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

  /// Set scene model
  void set_model(SceneModel* model);

  /// Update view scene completely
  void full_update();

private:
  /// New component event handler, listens for new units in scene model
  void on_component_add(StringHash eventType, VariantMap& eventData);
  /// Delete component event handler, listens for deleted units in scene model
  void on_component_remove(StringHash eventType, VariantMap& eventData);

  /// Choose correct view for newly created component
  void dispatch_new_unit(UnitModel* unit);

  /// Urho scene, used for view
  SharedPtr<Scene> m_urho_scene;
  /// Scene model, that we are viewing
  SharedPtr<SceneModel> m_model;
  /// Structure view
  SharedPtr<StructureView> m_structure_view;
};
