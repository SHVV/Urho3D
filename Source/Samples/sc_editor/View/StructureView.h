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

  /// Vertex buffer for storing all nodes (balls, joints)
  SharedPtr<VertexBuffer> m_vertex_buffer;
  /// Index buffer for nodes
  // TODO: use separate buffers for different patches
  SharedPtr<IndexBuffer> m_index_buffer;
  /// Map Node -> vertex buffer index
  HashMap<Node*, int> m_nodes_map;
  /// Vector with all nodes in buffer
  Vector<WeakPtr<NodeModel>> m_nodes;
  // TODO: Add vector of free space for buffer reusing
  /// List of dirty nodes
  PODVector<int> m_dirty_nodes;
};
