// SHW Spacecraft editor
//
// Unit Model class, represents basic class for every units in constructor

#include "UnitModel.h"

#include "../Core/MeshGeometry.h"
#include "../Core/MeshBuffer.h"
#include "SceneModel.h"

#include <Urho3D\Core\Context.h>
#include <Urho3D\Scene\Node.h>
#include <Urho3D\Graphics\StaticModel.h>

using namespace Urho3D;

extern const char* EDITOR_CATEGORY;
StringHash s_default_material = "default";

//static const char* state_names[] =
//{
//  "None",
//  "Highlighted",
//  "Selected",
//  "Hidden",
//  0
//};

/// Register object attributes.
void UnitModel::RegisterObject(Context* context)
{
  context->RegisterFactory<UnitModel>(EDITOR_CATEGORY);

  // TODO: add Attributes
  //URHO3D_ENUM_ATTRIBUTE("State", m_state, state_names, usNONE, AM_NET | AM_NOEDIT);
}

/// Construct.
UnitModel::UnitModel(Context* context)
  : Component(context)
{
  m_mesh_geometry = new MeshGeometry(context);
  m_mesh_buffer = new MeshBuffer(context);
  m_mesh_buffer->set_mesh_geometry(m_mesh_geometry);
  m_mesh_buffer->add_notification_receiver(this);
}

/// Destructor
UnitModel::~UnitModel()
{
  if (!m_mesh_buffer) {
    m_mesh_buffer->remove_notification_receiver(this);
  }
}

/// Get structure model
MeshGeometry* UnitModel::structure_mesh()
{
  // TODO: move from here
  return m_mesh_geometry;
}

/// Get rendering model
MeshGeometry* UnitModel::rendering_mesh()
{
  // TODO: move from here
  return m_mesh_geometry;
}

/// Get mesh rendering buffer
const MeshBuffer* UnitModel::rendering_buffer() const
{
  // TODO: factor out
  return m_mesh_buffer;
}

/// Get mesh rendering buffer
MeshBuffer* UnitModel::access_rendering_buffer()
{
  // TODO: factor out
  return m_mesh_buffer;
}

/// Get scene model
SceneModel* UnitModel::scene_model()
{
  // TODO: decide how to get scene this
  return SceneModel::get();
}

/// Creates all necessary components
void UnitModel::create_guts()
{
  m_model = node_->CreateComponent<StaticModel>();
  m_model->SetTemporary(true);
  m_model->SetCastShadows(true);
}

/// Update all links due to movements
void UnitModel::update_links()
{
  // TODO: update all existing links
}

/// Handle node being assigned.
void UnitModel::OnNodeSet(Node* node)
{
  if (node) {
    create_guts();
  }
}

/// Handle node (not only our) transform being dirtied.
void UnitModel::OnMarkedDirty(Node* node)
{
  update_links();
}

/// Get material name
StringHash UnitModel::material_name(
  const Vector<StringHash>& material_list, int id
)
{
  return id >= material_list.Size() ? s_default_material : material_list[id];
}

/// Update model and reassign materials
void UnitModel::update_model()
{
  if (m_mesh_buffer) {
    // Update model
    Model* model = m_mesh_buffer->model();
    m_model->SetModel(model);
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
          // TODO: Assert
          break;
        }
      }
      m_model->SetMaterial(i, material);
    }
  }
}

/// Mesh geometry notifications
void UnitModel::on_update()
{
  update_model();
}

///// Set unit state
//void UnitModel::set_state(State value)
//{
//  if (m_state != value) {
//    m_state = value;
//    notify_attribute_changed();
//  }
//}
//
///// Get unit state
//UnitModel::State UnitModel::state()
//{
//  return m_state;
//}

/// Notify all subscribers, that attribute of model has been changed
void UnitModel::notify_attribute_changed()
{
  // TODO: test this
  using namespace ModelAttributeChanged;

  VariantMap& eventData = GetEventDataMap();
  eventData[P_COMP] = this;
  // TODO: add attribute's categories

  SendEvent(E_MODEL_ATTRIBUTE_CHANGED, eventData);
  MarkNetworkUpdate();
}
