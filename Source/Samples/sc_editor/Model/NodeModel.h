// SHW Spacecraft editor
//
// Node Model class, represents one sphere node for structures

#pragma once

#include "../Model/UnitModel.h"

using namespace Urho3D;

class BeamModel;
class PlateModel;

class NodeModel : public UnitModel {
  // Enable type information.
  URHO3D_OBJECT(NodeModel, UnitModel);
public:

  /// Construct.
  NodeModel(Context* context);

  /// Destructor
  virtual ~NodeModel();

  /// Register object attributes.
  static void RegisterObject(Context* context);

  /// Handle attribute write access.
  //virtual void OnSetAttribute(const AttributeInfo& attr, const Variant& src);
  /// Apply attribute changes that can not be applied immediately. Called after scene load or a network update.
  //virtual void ApplyAttributes();
  // TODO: add other component methods, that we need to override
  
  /// Set node radius
  void set_radius(float radius);
  /// Get node radius
  float radius() const;

  /// Add beam, that is linked to this node
  //void add_beam(BeamModel* beam);
  /// Remove beam link
  //void remove_beam(BeamModel* beam);
  /// Get all beams, connected to this node
  //const Vector<WeakPtr<BeamModel>>& beams() const;

  /// Add plate link, that touches this node
  //void add_plate(PlateModel* plate);
  /// Remove plate link
  //void remove_plate(PlateModel* plate);
  /// Get All plates, touched this node
  //const Vector<WeakPtr<PlateModel>> plates() const;

protected:
  /// Handle node being assigned.
  virtual void OnNodeSet(Node* node) override;
  /// Handle scene being assigned.
  //virtual void OnSceneSet(Scene* scene);
  /// Handle node transform being dirtied.
  virtual void OnMarkedDirty(Node* node) override;

private:
  /// Radius of node.
  float m_radius; // TODO: Should be calculated automatically

  // TODO: Add normal.
  //Vector3 m_normal;

  /// List of beam links to this node (weak back ptrs)
  //Vector<WeakPtr<BeamModel>> m_beams;

  /// List of all plates, that touches this node (weak back ptrs)
  //Vector<WeakPtr<PlateModel>> m_plates;
};
