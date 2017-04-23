// SHW Spacecraft editor
//
// Class for dynamically work with geometric meshes (creates all buffers)

#include "MeshBuffer.h"

// Includes from Urho3D
#include "Urho3D\Core\CoreEvents.h"
#include "Urho3D\Core\Context.h"
#include "Urho3D\Graphics\Model.h"
#include "Urho3D\Graphics\Geometry.h"
#include "Urho3D\Graphics\VertexBuffer.h"
#include "Urho3D\Graphics\IndexBuffer.h"

#include <memory>

using namespace Urho3D;

// Construct.
MeshBuffer::MeshBuffer(Context* context)
  : Object(context),
    m_vertices_dirty(false),
    m_dirty(false),
    m_edges_dirty(false),
    m_polygons_dirty(false)

{
  SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(MeshBuffer, on_update_views));
}

/// Destructor
MeshBuffer::~MeshBuffer()
{
  if (m_mesh_geometry.NotNull()) {
    m_mesh_geometry->remove_notification_receiver(this);
  }
}

/// Set mesh geometry, that we must show
void MeshBuffer::set_mesh_geometry(MeshGeometry* mesh_geometry)
{
  if (m_mesh_geometry.NotNull()) {
    m_mesh_geometry->remove_notification_receiver(this);
  }
  m_mesh_geometry = mesh_geometry;
  m_mesh_geometry->add_notification_receiver(this);
  m_dirty = true;
}

/// Get mesh geometry, that we must show
MeshGeometry* MeshBuffer::mesh_geometry()
{
  return m_mesh_geometry;
}

/// Get Urho3D Model
Model* MeshBuffer::model()
{
  // Ensure, that everything is up-to-date
  update();
  return m_model;
}

/// Geometry material description
const MeshBuffer::GeometryDescription& MeshBuffer::geometry_description(
  int geometry_index
)
{
  // TODO: check input
  return m_geometries[geometry_index];
}

/// Prepare everything before rendering
void MeshBuffer::on_update_views(StringHash eventType, VariantMap& eventData)
{
  update();
}

/// Converts SubObjectType into PrimitiveType
static PrimitiveType convert(SubObjectType type)
{
  switch (type) {
    case sotVERTEX: return POINT_LIST;
    case sotEDGE: return LINE_LIST;
    case sotPOLYGON: return TRIANGLE_LIST;
    default: {
      // TODO: Assert
      return TRIANGLE_LIST;
    }
  }
}

///// Helper function for creating geometries from indexes
//void MeshBuffer::create_geometry(
//  const Vector<PODVector<int>>& indexies,
//  SubObjectType type,
//  PODVector<GeometryDescription>& geometries_description,
//  Vector<SharedPtr<Geometry>>& geometries
//)
//{
//  PrimitiveType urho_type = convert(type);
//  // Create index buffers and geometries
//  for (int i = 0; i < indexies.Size(); ++i) {
//    auto& index = indexies[i];
//    // All non-empty indexies
//    if (index.Size()) {
//      // Create index buffer
//      SharedPtr<IndexBuffer> index_buffer(new IndexBuffer(context_));
//      index_buffer->SetShadowed(false);
//      index_buffer->SetSize(index.Size(), true);
//      index_buffer->SetData(index.Buffer());
//
//      // Corresponding geometry
//      SharedPtr<Geometry> geometry(new Geometry(context_));
//      geometry->SetVertexBuffer(0, m_vertex_buffer);
//      geometry->SetIndexBuffer(index_buffer);
//      geometry->SetDrawRange(urho_type, 0, index.Size(), false);
//
//      // And push it into the list
//      geometries.Push(geometry);
//      geometries_description.Push({ type, i });
//    }
//  }
//}

/// Helper function for moving geometries from existing mesh
void MeshBuffer::move_geometry(
  SubObjectType type,
  PODVector<GeometryDescription>& geometries_description,
  Vector<SharedPtr<Geometry>>& geometries
)
{
  for (unsigned int i = 0; i < m_geometries.Size(); ++i) {
    auto& geometry_description = m_geometries[i];
    if (type == geometry_description.geometry_type) {
      geometries.Push(SharedPtr<Geometry>(m_model->GetGeometry(i, 0)));
      geometries_description.Push(geometry_description);
    }
  }
}

/// Helper function for creating geometries from primitives
template<class T>
void MeshBuffer::create_geometry(
  const PODVector<T>& primitives,
  SubObjectType type,
  PODVector<GeometryDescription>& geometries_description,
  Vector<SharedPtr<Geometry>>& geometries,
  std::function<void(int, const T&, PODVector<int>&)> filler
)
{
  // Indexies by material ID
  Vector<PODVector<int>> indexies;
  for (unsigned int i = 0; i < primitives.Size(); ++i) {
    auto& primitive = primitives[i];
    // If primitive is not deleted and has assigned material
    if (!primitive.deleted && (primitive.material >= 0)) {
      // If indexies buffer is too small
      if (indexies.Size() <= (unsigned)primitive.material) {
        indexies.Resize(primitive.material + 1);
      }
      filler(i, primitive, indexies[primitive.material]);
    }
  }

  // Create index buffers and geometries
  PrimitiveType urho_type = convert(type);
  // Create index buffers and geometries
  for (unsigned int i = 0; i < indexies.Size(); ++i) {
    auto& index = indexies[i];
    // All non-empty indexies
    if (index.Size()) {
      // Create index buffer
      SharedPtr<IndexBuffer> index_buffer(new IndexBuffer(context_));
      index_buffer->SetShadowed(false);
      index_buffer->SetSize(index.Size(), true);
      index_buffer->SetData(index.Buffer());

      // Corresponding geometry
      SharedPtr<Geometry> geometry(new Geometry(context_));
      geometry->SetVertexBuffer(0, m_vertex_buffer);
      geometry->SetIndexBuffer(index_buffer);
      geometry->SetDrawRange(urho_type, 0, index.Size(), false);

      // And push it into the list
      geometries.Push(geometry);
      geometries_description.Push({ type, i });
    }
  }
}

/// Do actual update
bool MeshBuffer::update()
{
  if (!(m_dirty || m_edges_dirty || m_polygons_dirty || 
        m_vertices_dirty || m_dirty_vertices.Size())){
    return false;
  }

  if (m_mesh_geometry.Null()) {
    return false;
  }

  bool recreate_buffer = false;
  unsigned int vertices_count = m_mesh_geometry->vertices().Capacity();
  if (m_dirty || !m_vertex_buffer) {
    m_vertices_dirty = true;
    m_edges_dirty = true;
    m_polygons_dirty = true;
    recreate_buffer = true;
  } else {
    // Buffer is not big enough to receive all vertices
    recreate_buffer = vertices_count > m_vertex_buffer->GetVertexCount();
  }

  int begin = -1;
  int end = -1;
  if (recreate_buffer) {
    // Recreate buffer from scratch
    m_vertex_buffer = new VertexBuffer(context_);
    m_vertex_buffer->SetShadowed(false);
    PODVector<VertexElement> elements;
    elements.Push(VertexElement(TYPE_VECTOR3, SEM_POSITION));
    elements.Push(VertexElement(TYPE_VECTOR3, SEM_NORMAL));
    elements.Push(VertexElement(TYPE_FLOAT, SEM_CUSTOM)); // Radius
    m_vertex_buffer->SetSize(vertices_count, elements, true);
    begin = 0;
    end = m_mesh_geometry->vertices().Size() - 1;
  } else if (m_dirty_vertices.Size()) {
    // Update affected part only
    // TODO: use just min-max vertex number
    m_dirty_vertices.Sort();
    begin = m_dirty_vertices.Front();
    end = m_dirty_vertices.Back();
  }

  // Bounding box for entire model
  BoundingBox bb;
  if (m_model && !recreate_buffer) {
    bb = m_model->GetBoundingBox();
  }
  if (!m_model) {
    m_model = new Model(context_);
  }
  float max_r = 0;
  // Update vertices, that need to be updated
  if (begin >= 0) {
    auto& vertices = m_mesh_geometry->vertices();
    float* buffer = (float*)m_vertex_buffer->Lock(begin, end - begin + 1);
    // Assert end <= vertices.Size();
    for (int i = begin; i <= end; ++i) {
      auto& vertex = vertices[i];
      bb.Merge(vertex.position);
      *buffer = vertex.position.x_; ++buffer;
      *buffer = vertex.position.y_; ++buffer;
      *buffer = vertex.position.z_; ++buffer;
      *buffer = vertex.normal.x_; ++buffer;
      *buffer = vertex.normal.y_; ++buffer;
      *buffer = vertex.normal.z_; ++buffer;
      *buffer = vertex.radius; ++buffer;
      if (vertex.radius > max_r) {
        max_r = vertex.radius;
      }
    }
    m_vertex_buffer->Unlock();
  }
  bb.max_ += Vector3(max_r, max_r, max_r);
  bb.min_ -= Vector3(max_r, max_r, max_r);
  
  // New geometries description
  PODVector<GeometryDescription> geometries_description;
  Vector<SharedPtr<Geometry>> geometries;

  // Update vertices indexes and geometries
  if (m_vertices_dirty) {
    auto& vertices = m_mesh_geometry->vertices();
    create_geometry<MeshGeometry::Vertex>(
      vertices, sotVERTEX,
      geometries_description, geometries,
      [](int i, const MeshGeometry::Vertex& primitive, PODVector<int>& indexies){
        indexies.Push(i);
      }
    );

    // Mark updated
    m_vertices_dirty = false;
  } else {
    // Move everything from existing mesh
    move_geometry(sotVERTEX, geometries_description, geometries);
  }

  // Update edges indexes and geometries
  if (m_edges_dirty) {
    // Indexies by material ID
    create_geometry<MeshGeometry::Edge>(
      m_mesh_geometry->edges(), sotEDGE,
      geometries_description, geometries,
      [](int i, const MeshGeometry::Edge& primitive, PODVector<int>& indexies){
        indexies.Push(primitive.vertexes[0]);
        indexies.Push(primitive.vertexes[1]);
      }
    );

    // Mark updated
    m_edges_dirty = false;
  } else {
    // Move everything from existing mesh
    move_geometry(sotEDGE, geometries_description, geometries);
  }

  // Update polygons indexes and geometries
  if (m_polygons_dirty) {
    // Indexies by material ID
    create_geometry<MeshGeometry::Polygon>(
      m_mesh_geometry->polygons(), sotPOLYGON,
      geometries_description, geometries,
      [](int i, const MeshGeometry::Polygon& primitive, PODVector<int>& indexies){
        // TODO: deal with secondary edge
        indexies.Push(primitive.vertexes[0]);
        indexies.Push(primitive.vertexes[1]);
        indexies.Push(primitive.vertexes[2]);
        if (primitive.vertexes[3] >= 0) {
          indexies.Push(primitive.vertexes[0]);
          indexies.Push(primitive.vertexes[2]);
          indexies.Push(primitive.vertexes[3]);
        }
      }
    );

    // Mark updated
    m_polygons_dirty = false;
  } else {
    // Move everything from existing mesh
    move_geometry(sotPOLYGON, geometries_description, geometries);
  }
  // TODO: add quads support

  // Update model
  m_model->SetBoundingBox(bb);
  m_model->SetNumGeometries(geometries.Size());
  for (unsigned int i = 0; i < geometries.Size(); ++i) {
    m_model->SetGeometry(i, 0, geometries[i]);
  }

  // update geometries descriptions
  m_geometries = geometries_description;

  // Reset all dirty flags and sets
  m_dirty = false;
  m_dirty_vertices.Clear();

  send_update();

  return true;
///////////////
  // TODO: Update everything here
  // - Create all models, if necessary
  // - setup buffers
  // - fill them
  // - split by material
  // - ...
  /*if (!m_dirty_nodes.Size()) {
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
  SharedPtr<Geometry> balls_geom(new Geometry(context_));
  balls_geom->SetVertexBuffer(0, m_vertex_buffer);
  balls_geom->SetDrawRange(POINT_LIST, 0, 0, 0, m_nodes.Size());

  SharedPtr<Model> balls_model(new Model(context_));
  balls_model->SetNumGeometries(1);
  balls_model->SetGeometry(0, 0, balls_geom);
  balls_model->SetBoundingBox(bb);

  ResourceCache* cache = GetSubsystem<ResourceCache>();

  StaticModel* object = node_->CreateComponent<StaticModel>();
  object->SetModel(balls_model);
  object->SetMaterial(cache->GetResource<Material>("Materials/GS/GSPointsToSpheresMat.xml"));
  object->SetCastShadows(true);

  m_dirty_nodes.Clear();*/
}

// Mesh geometry notifications
void MeshBuffer::on_update(MeshGeometry::UpdateType type, int i)
{
  switch (type) {
    case MeshGeometry::VERTEX_ADDED:
      m_dirty_vertices.Insert(i);
      m_vertices_dirty = true;
      break;

    case MeshGeometry::VERTEX_CHANGED:
      m_dirty_vertices.Insert(i);
      break;

    case MeshGeometry::VERTEX_REMOVED:
    case MeshGeometry::VERTEX_PAINTED:
      m_vertices_dirty = true;
      break;

    case MeshGeometry::EDGE_ADDED:
    case MeshGeometry::EDGE_REMOVED:
    case MeshGeometry::EDGE_PAINTED:
      m_edges_dirty = true;
      break;

    case MeshGeometry::POLYGON_ADDED:
    case MeshGeometry::POLYGON_REMOVED:
    case MeshGeometry::POLYGON_PAINTED:
      m_polygons_dirty = true;
      break;

    case MeshGeometry::GLOBAL_UPDATE:
      m_dirty = true;
      break;

    default:
      // TODO: asserts
      break;
  }
}

/// Add notification receiver
void MeshBuffer::add_notification_receiver(NotificationReceiver* target)
{
  m_notification_receivers.Push(target);
}

/// Remove notofication receiver
void MeshBuffer::remove_notification_receiver(NotificationReceiver* target)
{
  m_notification_receivers.Remove(target);
}

/// Send update to all subscribers
void MeshBuffer::send_update()
{
  for (int i = 0; i < m_notification_receivers.Size(); ++i) {
    m_notification_receivers[i]->on_update();
  }
}
