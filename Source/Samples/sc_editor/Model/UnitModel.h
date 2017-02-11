// SHW Spacecraft editor
//
// Unit Model class, represents basic class for every units in constructor

#pragma once

#include <Urho3D/Scene/Component.h>

using namespace Urho3D;

/// Particle effect finished.
URHO3D_EVENT(E_MODEL_ATTRIBUTE_CHANGED, ModelAttributeChanged)
{
  URHO3D_PARAM(P_COMP, Component);                    // Component pointer
  //URHO3D_PARAM(P_EFFECT, Effect);                // ParticleEffect pointer
}

class MaterialModel;

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

  /// Set material 
  //void set_material(MaterialModel* material);
  /// Get material
  //const MaterialModel* material() const;

  // TODO: add more smart restrictions on moving node
  /// Set movable flag. If true, it is possible to move node
  //void set_movable(bool value);
  /// Get movable flag
  //bool movable() const;

  enum State {
    usNONE = 0,
    usHIGHLIGHTED,  // Unit highlighted by mouse hovering
    usSELECTED,     // Unit selected
    usHIDDEN        // Unit hidden
  };

  /// Set unit state
  void set_state(State value);
  /// Get unit state
  State state();

protected:
  /// Notify all subscribers, that attribute of model has been changed
  void notify_attribute_changed();

private:

  /// Link to material 
  //SharedPtr<MaterialModel> m_material;
  /// Property for material link serialization
  //unsigned int m_material_id;
  /// Movable flag. If true, it is possible to move node.
  //bool m_movable;
  /// State
  State m_state;
};
