// SHW Spacecraft editor
//
// Beam Model class, represents beam, connected two nodes

#pragma once

#include "../Model/UnitModel.h"

using namespace Urho3D;

class NodeModel;
class PlateModel;

class BeamModel : public UnitModel {
  // Enable type information.
  URHO3D_OBJECT(BeamModel, UnitModel);
public:

  /// Construct.
  BeamModel(Context* context);

  /// Destructor
  virtual ~BeamModel();

  /// Register object attributes.
  static void RegisterObject(Context* context);

  // TODO: add other component methods, that we need to override

  // Read only properties, calculated on-the fly
  /// Get beam length
  float length() const;
  /// Get beam radius
  float radius() const;

  /// Set beam first node 
  void set_node_1(NodeModel* node);
  /// Get beam first node
  NodeModel* node_1();
  /// Set beam second node 
  void set_node_2(NodeModel* node);
  /// Get beam second node
  NodeModel* node_2();

  /// Set this beam is secondary (it will no be used to calculate node radius)
  void set_secondary(bool flag);
  /// Get is beam secondary
  bool secondary();

  /// Add plate link, that touches this beam
  void add_plate(PlateModel* plate);
  /// Remove plate link
  void remove_plate(PlateModel* plate);

private:
  /// Radius of node.
  float m_radius;

  // TODO: Add normal.
  //Vector3 m_normal;

  /// Link to first node 
  SharedPtr<NodeModel> m_node_1;
  /// Property for node link serialization
  unsigned int m_node_1_id;

  /// Link to second node 
  SharedPtr<NodeModel> m_node_2;
  /// Property for node link serialization
  unsigned int m_node_2_id;

  /// List of all plates, that touches this beam
  Vector<WeakPtr<PlateModel>> m_plates;
};
