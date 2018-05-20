// SHW Spacecraft editor
//
// Basic procedural unit. Contains link to function for generating guts.

#pragma once

#include "../Model/ProceduralUnit.h"

using namespace Urho3D;

class MultiPointUnit : public ProceduralUnit {
  // Enable type information.
  URHO3D_OBJECT(MultiPointUnit, ProceduralUnit);
public:

  /// Construct.
  MultiPointUnit(Context* context);
  
  /// Destructor
  ~MultiPointUnit();

  /// Register object attributes.
  static void RegisterObject(Context* context);

  /// Sets i'th reference node
  /// \return true, if acceps more nodes
  virtual bool set_reference_node(int i, Node* node);

  /// Gets i'th reference node, if it exists
  virtual Node* get_reference_node(int i);

  /// Handle attribute write access.
  //virtual void OnSetAttribute(const AttributeInfo& attr, const Variant& src);
  /// Apply attribute changes that can not be applied immediately. Called after scene load or a network update.
  //virtual void ApplyAttributes() override;
  // TODO: add other component methods, that we need to override

  // TODO: ....

protected:
  // Existing overrides
  /// Handle node being assigned.
  virtual void OnNodeSet(Node* node) override;
  /// Handle scene node transform dirtied.
  virtual void OnMarkedDirty(Node* node) override;
  /// On node remove event
  void on_node_remove(StringHash eventType, VariantMap& eventData);
  // New interface functions
  /// Create or update all necessary components - override for derived classes
  virtual void update_guts_int() override;

private:
  /// Reference nodes for creating this one
  Vector<WeakPtr<Node>> m_reference_nodes;
};
