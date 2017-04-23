// SHW Spacecraft editor
//
// Dynamic Model component.
// Component for visualizing our editable or procedurally generated mesh.
// Allows to assign materials with custom types to special named slots.

#pragma once

#include "../Core/MeshBuffer.h"

#include <Urho3D\Graphics\StaticModel.h>

using namespace Urho3D;

class MeshGeometry;
class SceneModel;

class DynamicModel : public StaticModel, public MeshBuffer::NotificationReceiver {
  // Enable type information.
  URHO3D_OBJECT(DynamicModel, StaticModel);
public:

  /// Construct.
  DynamicModel(Context* context);
  
  /// Destructor
  ~DynamicModel();

  /// Register object attributes.
  static void RegisterObject(Context* context);

  /// Handle attribute write access.
  //virtual void OnSetAttribute(const AttributeInfo& attr, const Variant& src);
  /// Apply attribute changes that can not be applied immediately. Called after scene load or a network update.
  //virtual void ApplyAttributes() override;
  // TODO: add other component methods, that we need to override

  // TODO: materials management
  /// Set material 
  //void set_material(MaterialModel* material);
  /// Get material
  //const MaterialModel* material() const;

  /// Set mesh rendering buffer
  void mesh_buffer(MeshBuffer* mesh);
  /// Get mesh rendering buffer
  const MeshBuffer* mesh_buffer() const;

  /// Get mesh geometry
  const MeshGeometry* mesh_geometry() const;

protected:
  // New interface functions
  ///// Creates all necessary components
  //virtual void create_guts();

  //// Existing overrides
  ///// Handle node being assigned.
  //virtual void OnNodeSet(Node* node) override;

  /// Mesh geometry notifications
  virtual void on_update() override;

  // Utilities
  /// Get mesh rendering buffer
  MeshBuffer* access_rendering_buffer();
  /// Update model and reassign materials
  void update_model();
  /// Get scene model
  SceneModel* scene_model();
  /// Get material name
  StringHash material_name(const Vector<StringHash>& material_list, int id);

private:
  /// Mesh guts
  SharedPtr<MeshBuffer> m_mesh_buffer;

  /// Materials lists
  // TODO: use string name as key
  Vector<StringHash> m_vertex_materials;
  Vector<StringHash> m_edge_materials;
  Vector<StringHash> m_polygon_materials;

  /// Cached viewing model
  //WeakPtr<StaticModel> m_model;
};
