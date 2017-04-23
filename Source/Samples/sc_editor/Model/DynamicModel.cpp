// SHW Spacecraft editor
//
// Dynamic Model component.
// Component for visualizing our editable or procedurally generated mesh.
// Allows to assign materials with custom types to special named slots.

#include "DynamicModel.h"

#include "../Core/MeshGeometry.h"
#include "../Core/MeshBuffer.h"
#include "../Core/MeshGenerator.h"
#include "SceneModel.h"

#include <Urho3D\Core\Context.h>
#include <Urho3D\Scene\Node.h>

using namespace Urho3D;

extern const char* EDITOR_CATEGORY;
StringHash s_default_material = "default";

/// Register object attributes.
void DynamicModel::RegisterObject(Context* context)
{
  context->RegisterFactory<DynamicModel>(EDITOR_CATEGORY);

  URHO3D_COPY_BASE_ATTRIBUTES(StaticModel);
  // TODO: add Attributes
  //URHO3D_ENUM_ATTRIBUTE("State", m_state, state_names, usNONE, AM_NET | AM_NOEDIT);
}

/// Construct.
DynamicModel::DynamicModel(Context* context)
  : StaticModel(context)
{
  SetCastShadows(true);
}

/// Destructor
DynamicModel::~DynamicModel()
{
  if (m_mesh_buffer.NotNull()) {
    m_mesh_buffer->remove_notification_receiver(this);
  }
}

/// Set mesh rendering buffer
void DynamicModel::mesh_buffer(MeshBuffer* mesh)
{
  if (m_mesh_buffer.NotNull()) {
    m_mesh_buffer->remove_notification_receiver(this);
  }
  m_mesh_buffer = mesh;
  if (m_mesh_buffer.NotNull()) {
    m_mesh_buffer->add_notification_receiver(this);
  }
  update_model();
}

/// Get mesh geometry
const MeshGeometry* DynamicModel::mesh_geometry() const
{
  return m_mesh_buffer.NotNull() ? m_mesh_buffer->mesh_geometry() : nullptr;
}

/// Get mesh rendering buffer
const MeshBuffer* DynamicModel::mesh_buffer() const
{
  return m_mesh_buffer;
}

/// Get mesh rendering buffer
MeshBuffer* DynamicModel::access_rendering_buffer()
{
  return m_mesh_buffer;
}

/// Get scene model
SceneModel* DynamicModel::scene_model()
{
  // TODO: decide how to get scene this
  return SceneModel::get();
}

///// Creates all necessary components
//void DynamicModel::create_guts()
//{
//  m_model = node_->CreateComponent<StaticModel>();
//  m_model->SetTemporary(true);
//  m_model->SetCastShadows(true);
//}
//
///// Handle node being assigned.
//void DynamicModel::OnNodeSet(Node* node)
//{
//  if (node) {
//    create_guts();
//  }
//}

/// Get material name
StringHash DynamicModel::material_name(
  const Vector<StringHash>& material_list, int id
)
{
  return id >= material_list.Size() ? s_default_material : material_list[id];
}

/// Update model and reassign materials
void DynamicModel::update_model()
{
  if (m_mesh_buffer) {
    // Update model
    Model* model = m_mesh_buffer->model();
    SetModel(model);
    SetEnabled(true);
    // Update all materials
    SceneModel* scene = scene_model();
    for (unsigned int i = 0; i < model->GetNumGeometries(); ++i) {
      auto& geometry_description = m_mesh_buffer->geometry_description(i);
      Material* material;
      switch (geometry_description.geometry_type) {
        case sotVERTEX: {
          material = scene->get_vertex_material(
            material_name(m_vertex_materials, geometry_description.material)
          );
          break;
        }
        case sotEDGE: {
          material = scene->get_edge_material(
            material_name(m_edge_materials, geometry_description.material)
          );
          break;
        }
        case sotPOLYGON: {
          material = scene->get_polygon_material(
            material_name(m_polygon_materials, geometry_description.material)
          );
          break;
        }
        default: {
          assert(false);
          break;
        }
      }
      SetMaterial(i, material);
    }
  } else {
    SetEnabled(false);
  }
}

/// Mesh geometry notifications
void DynamicModel::on_update()
{
  update_model();
}

///// Notify all subscribers, that attribute of model has been changed
//void DynamicModel::notify_attribute_changed()
//{
//  // TODO: test this
//  using namespace ModelAttributeChanged;
//
//  VariantMap& eventData = GetEventDataMap();
//  eventData[P_COMP] = this;
//  // TODO: add attribute's categories
//
//  SendEvent(E_MODEL_ATTRIBUTE_CHANGED, eventData);
//  MarkNetworkUpdate();
//}
