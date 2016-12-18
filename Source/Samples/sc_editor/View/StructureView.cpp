// SHW Spacecraft editor
//
// Class for displaying structure hull

#include "StructureView.h"

// Includes from Editor
#include "../Model/NodeModel.h"

// Includes from Urho3D
#include "Urho3D\Core\CoreEvents.h"
#include "Urho3D\Core\Context.h"
#include "Urho3D\Graphics\Material.h"
#include "Urho3D\Graphics\Model.h"
#include "Urho3D\Graphics\StaticModel.h"
#include "Urho3D\Graphics\Geometry.h"
#include "Urho3D\Graphics\VertexBuffer.h"
#include "Urho3D\Graphics\IndexBuffer.h"
#include "Urho3D\Resource\ResourceCache.h"
#include "Urho3D\Scene\Node.h"

#include <memory>

using namespace Urho3D;

extern const char* EDITOR_CATEGORY;

// Construct.
StructureView::StructureView(Context* context)
: Component(context)
{
  SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(StructureView, on_update_views));
}

// Destructor
StructureView::~StructureView()
{

}

/// Register object attributes.
void StructureView::RegisterObject(Context* context)
{
  context->RegisterFactory<StructureView>(EDITOR_CATEGORY);
}

// Adds vew for Node model
void StructureView::add_node(NodeModel* joint)
{
  Node* node = joint->GetNode();
  // TODO: insert in free space
  m_nodes.Push(WeakPtr<NodeModel>(joint));
  m_nodes_map[node] = m_nodes.Size() - 1;

  // Subscribe at the end, because it will notify dirty immediatly
  node->AddListener(this);
}

/// Prepare everything before rendering
void StructureView::on_update_views(StringHash eventType, VariantMap& eventData)
{
  // TODO: Update everything here
  // - Create all models, if necessary
  // - setup buffers
  // - fill them
  // - split by material
  // - ...
  if (!m_dirty_nodes.Size()) {
    return;
  }

  // TODO: 
  // - Use correct radius
  // - Update only changed nodes in batches, possibly
  // - Add index buffer to skip hidden nodes
  // - Create buffer big enought, to contain many points
  // - Recreate buffer, if it is not big enough
  // - Get material from manager
  // - Do something with selection / highlighting
  // ...

  // Simple implementation - recreates everything
  auto vertex_data = std::make_unique<float[]>(m_nodes.Size() * (3 + 1));
  BoundingBox bb;
  for (int i = 0; i < m_nodes.Size(); ++i) {
    auto node = m_nodes[i].Lock();
    if (node) {
      Node* node_node = node->GetNode();
      Vector3 pos = node_node->GetWorldPosition();
      bb.Merge(pos);
      vertex_data[i * 4 + 0] = pos.x_;
      vertex_data[i * 4 + 1] = pos.y_;
      vertex_data[i * 4 + 2] = pos.z_;
      vertex_data[i * 4 + 3] = node->radius();
    }
  }

  m_vertex_buffer = new VertexBuffer(context_);
  PODVector<VertexElement> elements;
  elements.Push(VertexElement(TYPE_VECTOR3, SEM_POSITION));
  //elements.Push(VertexElement(TYPE_VECTOR3, SEM_NORMAL));
  elements.Push(VertexElement(TYPE_FLOAT, SEM_CUSTOM)); // Radius
  m_vertex_buffer->SetSize(m_nodes.Size(), elements);
  m_vertex_buffer->SetData(vertex_data.get());

  //m_index_buffer = new IndexBuffer(context_);
  SharedPtr<Model> balls_model(new Model(context_));
  SharedPtr<Geometry> balls_geom(new Geometry(context_));

  balls_geom->SetVertexBuffer(0, m_vertex_buffer);
  balls_geom->SetDrawRange(POINT_LIST, 0, 0, 0, m_nodes.Size());
  
  balls_model->SetNumGeometries(1);
  balls_model->SetGeometry(0, 0, balls_geom);
  balls_model->SetBoundingBox(bb);

  ResourceCache* cache = GetSubsystem<ResourceCache>();

  StaticModel* object = node_->CreateComponent<StaticModel>();
  object->SetModel(balls_model);
  object->SetMaterial(cache->GetResource<Material>("Materials/GS/GSPointsToSpheresMat.xml"));
  object->SetCastShadows(true);

  m_dirty_nodes.Clear();
}

/// Handle scene node transform dirtied.
void StructureView::OnMarkedDirty(Node* node)
{
  NodeModel* joint = node->GetComponent<NodeModel>();
  if (joint) {
    int index = m_nodes_map[node];
    m_dirty_nodes.Push(index);
  }
}
