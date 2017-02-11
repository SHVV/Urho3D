// SHW Spacecraft editor
//
// Class for displaying structure hull

#pragma once

#include <Urho3D/Scene/Component.h>

// Editor predeclarations
class NodeModel;

// Urho predeclarations
namespace Urho3D {
  class Context;
  class Node;
  class Scene;
  class VertexBuffer;
  class IndexBuffer;
}

using namespace Urho3D;

class StructureView : public Component {
  // Enable type information.
  URHO3D_OBJECT(StructureView, Component);
public:

  /// Construct.
  StructureView(Context* context);

  /// Destructor
  virtual ~StructureView();

  /// Register object attributes.
  static void RegisterObject(Context* context);

  /// Adds vew for Node model
  void add_node(NodeModel* joint);

private:
  /// Prepare everything before rendering
  void on_update_views(StringHash eventType, VariantMap& eventData);

  /// Handle scene node transform dirtied.
  virtual void OnMarkedDirty(Node* node) override;
  /// Handle node model attribute changed event
  void on_node_attribute_changed(StringHash eventType, VariantMap& eventData);
  /// Mark node model dirty
  void mark_dirty(NodeModel* node);

  // TODO: use separate buffers for different patches

  /// Buffers
  /// Vertex buffer for storing all nodes (balls, joints)
  SharedPtr<VertexBuffer> m_vertex_buffer;
  // TODO: use separate index buffers for each material
  /// Index buffer for visible nodes
  SharedPtr<IndexBuffer> m_nodes_index_buffer;
  /// Index buffer for highlighted nodes
  SharedPtr<IndexBuffer> m_highlighted_nodes_index_buffer;
  /// Index buffer for selected nodes
  SharedPtr<IndexBuffer> m_selected_nodes_index_buffer;

  /// Map NodeModel -> vertex buffer index
  HashMap<NodeModel*, int> m_nodes_map;
  /// Vector with all nodes in buffer
  Vector<WeakPtr<NodeModel>> m_nodes;

  // TODO: Add vector of free space for buffer reusing

  /// Set of dirty nodes
  HashSet<int> m_dirty_nodes;
  /// Set of nodes to delete
  HashSet<int> m_deleted_nodes;
  /// Flag, that nodes index is dirty
  bool m_nodes_index_dirty;
};
