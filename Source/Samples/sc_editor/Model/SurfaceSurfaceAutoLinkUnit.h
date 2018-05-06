// SHW Spacecraft editor
//
// Unit for automatic Surface-Surface truss connection.

#pragma once

#include "UnitModel.h"
#include "BaseAttachableSurface.h"

using namespace Urho3D;

class SurfaceSurfaceAutoLinkUnit : public UnitModel {
  // Enable type information.
  URHO3D_OBJECT(SurfaceSurfaceAutoLinkUnit, UnitModel);
public:

  /// Construct.
  SurfaceSurfaceAutoLinkUnit(Context* context);
  
  /// Destructor
  ~SurfaceSurfaceAutoLinkUnit();

  /// Register object attributes.
  static void RegisterObject(Context* context);

  /// Set nodes to connect
  void set_nodes(Node* attached_node, Node* parent_node);

  /// Handle attribute write access.
  //virtual void OnSetAttribute(const AttributeInfo& attr, const Variant& src);
  /// Apply attribute changes that can not be applied immediately. Called after scene load or a network update.
  //virtual void ApplyAttributes() override;
  // TODO: add other component methods, that we need to override

  // TODO: add automatic remove on one of node delete
  // TODO: ....

protected:
  // New interface functions
  /// Called on setting parameters
  //virtual void apply_parameters(int index = -1);
  /// Create or update all necessary components - override for derived classes
  virtual void update_guts_int() override;
  /// Prepare everything before rendering
  void on_update_views(StringHash eventType, VariantMap& eventData);
  /// On surface changed
  void on_surface_changed(StringHash eventType, VariantMap& eventData);
  /// Handle scene node transform dirtied.
  virtual void OnMarkedDirty(Node* node) override;

  // Attachable surfaces.
  BaseAttachableSurface* get_attached_surface();
  BaseAttachableSurface* get_parent_surface();

private:
  // Connected nodes.
  WeakPtr<Node> m_attached_node;
  WeakPtr<Node> m_parent_node;

  // IDs of the nodes for serialization.
  //unsigned m_attached_node_id;
  //unsigned m_parent_node_id;

  bool m_truss_dirty = true;
};
