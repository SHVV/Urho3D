// SHW Spacecraft editor
//
// Unit Model class, represents basic class for every units in constructor

#pragma once

#include "../Core/MeshBuffer.h"
#include "../Core/Parameters.h"
#include "../Core/ParametersDescription.h"

#include <Urho3D/Scene/Component.h>

namespace Urho3D {
  class StaticModel;
}

using namespace Urho3D;

/// Particle effect finished.
URHO3D_EVENT(E_MODEL_ATTRIBUTE_CHANGED, ModelAttributeChanged)
{
  URHO3D_PARAM(P_COMP, Component);                    // Component pointer
  //URHO3D_PARAM(P_EFFECT, Effect);                // ParticleEffect pointer
}

class SceneModel;

class UnitModel : public Component {
  // Enable type information.
  URHO3D_OBJECT(UnitModel, Component);
public:

  /// Construct.
  UnitModel(Context* context);
  
  /// Destructor
  ~UnitModel();

  /// Register object attributes.
  static void RegisterObject(Context* context);

  /// Handle attribute write access.
  //virtual void OnSetAttribute(const AttributeInfo& attr, const Variant& src);
  /// Apply attribute changes that can not be applied immediately. Called after scene load or a network update.
  //virtual void ApplyAttributes() override;
  // TODO: add other component methods, that we need to override

  // GetDependencyNodes?

  // Parameters management
  /// Get all parameters of the component
  const Parameters& parameters() const;
  /// Set all parameters in one go
  void set_parameters(const Parameters& parameters);
  /// Set one parameter by index
  void set_parameter(ParameterID index, const Variant& parameter);
  /// Get all parameters description of the component
  virtual const ParametersDescription& parameters_description() const;
  // TODO: default parameters

  // Components tracking
  /// Gets or creates component by its type and automatically incremented index.
  Component* get_component(StringHash type, CreateMode mode = REPLICATED);
  /// Gets or creates component by its type and automatically incremented index.
  template <class T> T* get_component(CreateMode mode = REPLICATED);
  // TODO: Nodes tracking

  // TODO: Work with sub-objects ? to other

  // TODO: ....

protected:
  // New interface functions
  /// Called on setting parameters
  virtual void apply_parameters(int index = -1);
  /// Create or update all necessary components
  void update_guts();
  /// Create or update all necessary components - override for derived classes
  virtual void update_guts_int();

  /// Update all links due to movements
  //virtual void update_links();

  // Existing overrides
  /// Handle node being assigned.
  virtual void OnNodeSet(Node* node) override;
  /// Handle node (not only our) transform being dirtied.
  //virtual void OnMarkedDirty(Node* node) override;

  // Utilities
  /// Notify all subscribers, that attribute of model has been changed
  void notify_attribute_changed();
  /// Starts updating all guts. Resets tracking
  void start_updating();
  /// Finishes updating process. Removes all untouched tracked components
  void finish_updating();
  ///// Get scene model
  //SceneModel* scene_model();
private:

  // TODO: save this guts
  /// Parameters of unit
  Parameters m_parameters;
  /// All tracked components
  PODVector<Component*> m_tracked_components;
  /// Second list of components during update
  PODVector<Component*> m_update_components;

  /// Back pointer to scene
  //WeakPtr<SceneModel> m_scene;

  /// Property for material link serialization
  //unsigned int m_material_id;
  /// Movable flag. If true, it is possible to move node.
  //bool m_movable;
  /// State
  //State m_state;
};


template <class T> T* UnitModel::get_component(CreateMode mode)
{
  return static_cast<T*>(get_component(T::GetTypeStatic(), mode));
}
