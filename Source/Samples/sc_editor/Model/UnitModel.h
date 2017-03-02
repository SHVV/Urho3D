// SHW Spacecraft editor
//
// Unit Model class, represents basic class for every units in constructor

#pragma once

#include "../Core/MeshBuffer.h"

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

class MeshGeometry;
class MaterialModel;
class SceneModel;

class UnitModel : public Component, public MeshBuffer::NotificationReceiver {
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

  /// Get structure model
  MeshGeometry* structure_mesh();

  /// Get rendering model
  MeshGeometry* rendering_mesh();

  /// Get mesh rendering buffer
  const MeshBuffer* rendering_buffer() const;

  // TODO: Physics model

  // TODO: links

  // TODO: update node position

  // TODO: materials management

  // TODO: attribute management

  // TODO: Work with sub-objects

  // TODO: Ray casts

  // TODO: ....

  //enum State {
  //  usNONE = 0,
  //  usHIGHLIGHTED,  // Unit highlighted by mouse hovering
  //  usSELECTED,     // Unit selected
  //  usHIDDEN        // Unit hidden
  //};

  ///// Set unit state
  //void set_state(State value);
  ///// Get unit state
  //State state();

protected:
  // New interface functions
  /// Creates all necessary components
  virtual void create_guts();
  /// Update all links due to movements
  virtual void update_links();

  // Existing overrides
  /// Handle node being assigned.
  virtual void OnNodeSet(Node* node) override;
  /// Handle node (not only our) transform being dirtied.
  virtual void OnMarkedDirty(Node* node) override;

  /// Mesh geometry notifications
  virtual void on_update() override;

  // Utilities
  /// Notify all subscribers, that attribute of model has been changed
  void notify_attribute_changed();
  /// Get mesh rendering buffer
  MeshBuffer* access_rendering_buffer();
  /// Update model and reassign materials
  void update_model();
  /// Get scene model
  SceneModel* scene_model();
  /// Get material name
  StringHash material_name(const Vector<StringHash>& material_list, int id);
private:
  // TODO: factor out
  /// Mesh guts
  SharedPtr<MeshGeometry> m_mesh_geometry;
  SharedPtr<MeshBuffer> m_mesh_buffer;

  /// Materials lists
  Vector<StringHash> m_vertex_materials;
  Vector<StringHash> m_edge_materials;
  Vector<StringHash> m_polygon_materials;

  /// Cached viewing model
  WeakPtr<StaticModel> m_model;

  /// Back pointer to scene
  //WeakPtr<SceneModel> m_scene;

  /// Property for material link serialization
  //unsigned int m_material_id;
  /// Movable flag. If true, it is possible to move node.
  //bool m_movable;
  /// State
  //State m_state;
};
