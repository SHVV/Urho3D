// SHW Spacecraft editor
//
// Nodes manipulation context class, used for move, rotate ad clone nodes

#include "NodesContext.h"

// Includes from Editor
#include "../Core/MeshGenerator.h"
#include "../Model/NodeModel.h"
#include "../Model/SceneModel.h"
#include "../View/SceneView.h"

// Includes from Urho3D
#include "Urho3D/Graphics/Graphics.h"
#include "Urho3D/Graphics/Camera.h"
#include "Urho3D/Graphics/Octree.h"
#include "Urho3D/Graphics/Material.h"
#include "Urho3D/Graphics/StaticModel.h"
#include "Urho3D/Scene/Node.h"
#include "Urho3D/Scene/Scene.h"
#include "Urho3D/UI/UI.h"
#include "Urho3D/Input/Input.h"
#include "Urho3D/Resource/ResourceCache.h"

using namespace Urho3D;

/// Construct.
NodesContext::NodesContext(Context* context, SceneModel* model, SceneView* view)
: BaseContext(context, model, view),
  m_focus_part(nullptr),
  m_active_part(nullptr)
{
  ResourceCache* cache = GetSubsystem<ResourceCache>();

  m_gizmo = m_view->scene()->CreateChild("NodesGimbal", LOCAL);
  m_gizmo->SetEnabled(false);

  auto generator = GetSubsystem<MeshGenerator>();

  Polyline2 profile;
  PolylineSegment seg;
  seg.m_lateral_material_id = -1;
  seg.m_longitudal_material_id = -1;
  seg.m_node_material_id = -1;
  seg.m_plate_material_id = 0;
  seg.m_node_radius = 0.0;
  seg.m_smooth_vertex = true;
  seg.m_smooth_segment = true;

  // Arrow profile
  float line_r = 0.01;
  float arrow_r = 0.05;
  seg.m_pos = Vector2(-1, 0);
  profile.segments().Push(seg);
  seg.m_pos = Vector2(-1, line_r);
  profile.segments().Push(seg);
  seg.m_pos = Vector2(1, line_r);
  profile.segments().Push(seg);
  seg.m_pos = Vector2(1 - arrow_r * 5, arrow_r);
  profile.segments().Push(seg);
  seg.m_pos = Vector2(1, 0);
  profile.segments().Push(seg);

  // Liear axis
  SharedPtr<MeshGeometry> mesh(generator->lathe(profile, 12, ttQUAD));

  // Ring profile
  float thickness = 0.005;
  float radius = 0.6;
  float width = 0.02;
  Polyline2 profile_ring;
  seg.m_pos = Vector2(0, radius);
  profile_ring.segments().Push(seg);
  seg.m_pos = Vector2(-thickness, radius + width);
  profile_ring.segments().Push(seg);
  seg.m_pos = Vector2(thickness, radius + width);
  profile_ring.segments().Push(seg);
  seg.m_pos = Vector2(0, radius);
  profile_ring.segments().Push(seg);

  // Liear axis
  SharedPtr<MeshGeometry> mesh_ring(generator->lathe(profile_ring, 64, ttQUAD));
  {
    SharedPtr<MeshBuffer> mesh_buffer(new MeshBuffer(context));
    mesh_buffer->set_mesh_geometry(mesh);
    GizmoPart axis;
    axis.m_component = m_gizmo->CreateComponent<StaticModel>();
    axis.m_component->SetOccludee(false);
    axis.m_component->SetModel(mesh_buffer->model());
    axis.m_component->SetMaterial(0, cache->GetResource<Material>("Materials/Editor/BlueUnlit.xml"));
    axis.m_axis = aZ;
    axis.m_rotation = false;
    m_gizmo_parts.Push(axis);

    SharedPtr<MeshBuffer> mesh_buffer_ring(new MeshBuffer(context));
    mesh_buffer_ring->set_mesh_geometry(mesh_ring);
    axis.m_component = m_gizmo->CreateComponent<StaticModel>();
    axis.m_component->SetOccludee(false);
    axis.m_component->SetModel(mesh_buffer_ring->model());
    axis.m_component->SetMaterial(0, cache->GetResource<Material>("Materials/Editor/BlueUnlit.xml"));
    axis.m_axis = aZ;
    axis.m_rotation = true;
    m_gizmo_parts.Push(axis);
  }
  {
    Matrix3x4 tr(
      Vector3(),
      Quaternion(-90, 0, 0),
      1.0
    );

    mesh->transform(tr);
    SharedPtr<MeshBuffer> mesh_buffer(new MeshBuffer(context));
    mesh_buffer->set_mesh_geometry(mesh);
    GizmoPart axis;
    axis.m_component = m_gizmo->CreateComponent<StaticModel>();
    axis.m_component->SetOccludee(false);
    axis.m_component->SetModel(mesh_buffer->model());
    axis.m_component->SetMaterial(0, cache->GetResource<Material>("Materials/Editor/GreenUnlit.xml"));
    axis.m_axis = aY;
    axis.m_rotation = false;
    m_gizmo_parts.Push(axis);

    mesh_ring->transform(tr);
    SharedPtr<MeshBuffer> mesh_buffer_ring(new MeshBuffer(context));
    mesh_buffer_ring->set_mesh_geometry(mesh_ring);
    axis.m_component = m_gizmo->CreateComponent<StaticModel>();
    axis.m_component->SetOccludee(false);
    axis.m_component->SetModel(mesh_buffer_ring->model());
    axis.m_component->SetMaterial(0, cache->GetResource<Material>("Materials/Editor/GreenUnlit.xml"));
    axis.m_axis = aY;
    axis.m_rotation = true;
    m_gizmo_parts.Push(axis);
  }
  {
    Matrix3x4 tr(
      Vector3(),
      Quaternion(90, 90, 0),
      1.0
    );
    mesh->transform(tr);
    SharedPtr<MeshBuffer> mesh_buffer(new MeshBuffer(context));
    mesh_buffer->set_mesh_geometry(mesh);
    GizmoPart axis;
    axis.m_component = m_gizmo->CreateComponent<StaticModel>();
    axis.m_component->SetOccludee(false);
    axis.m_component->SetModel(mesh_buffer->model());
    axis.m_component->SetMaterial(0, cache->GetResource<Material>("Materials/Editor/RedUnlit.xml"));
    axis.m_axis = aX;
    axis.m_rotation = false;
    m_gizmo_parts.Push(axis);

    mesh_ring->transform(tr);
    SharedPtr<MeshBuffer> mesh_buffer_ring(new MeshBuffer(context));
    mesh_buffer_ring->set_mesh_geometry(mesh_ring);
    axis.m_component = m_gizmo->CreateComponent<StaticModel>();
    axis.m_component->SetOccludee(false);
    axis.m_component->SetModel(mesh_buffer_ring->model());
    axis.m_component->SetMaterial(0, cache->GetResource<Material>("Materials/Editor/RedUnlit.xml"));
    axis.m_axis = aX;
    axis.m_rotation = true;
    m_gizmo_parts.Push(axis);
  }

  /*gizmo->SetModel(cache->GetResource<Model>("Models/Editor/Axes.mdl"));
  gizmo->SetMaterial(0, cache->GetResource<Material>("Materials/Editor/RedUnlit.xml"));
  gizmo->SetMaterial(1, cache->GetResource<Material>("Materials/Editor/GreenUnlit.xml"));
  gizmo->SetMaterial(2, cache->GetResource<Material>("Materials/Editor/BlueUnlit.xml"));*/
  //gizmo.viewMask = 0x80000000; // Editor raycasts use viewmask 0x7fffffff

}

/// Destructor
NodesContext::~NodesContext()
{
}

/// Activates context and allows it to set up all its guts
void NodesContext::activate()
{

}

/// Deactivate context and remove all temporary objects
void NodesContext::deactivate()
{
  m_gizmo->SetEnabled(false);
}

/// Mouse button down handler
void NodesContext::on_mouse_down()
{
  if (m_focus_part) {
    m_active_part = m_focus_part;
    m_gizmo_pos = calculate_gizmo_point();
  }
}

/// Mouse button up handler
void NodesContext::on_mouse_up()
{
  Input* input = GetSubsystem<Input>();
  if (m_active_part) {
    //
    m_active_part = nullptr;

    // Undo support
    commit_transaction();
  } else {
    Node* node = get_unit_under_mouse();
    if (input->GetKeyDown(KEY_CTRL)) {
      if (node) {
        auto selected = m_view->selected();
        if (selected.Find(node) != selected.End()) {
          m_view->deselect(node);
        } else {
          m_view->select(node);
        }
      }
    } else {
      m_view->clear_selection();
      if (node) {
        m_view->select(node);
      }
    }
    auto selected = m_view->selected();
    if (selected.Size()) {
      //m_gizmo->SetPosition(selected.Back()->GetPosition());
      m_gizmo->SetEnabled(true);
    } else {
      m_gizmo->SetEnabled(false);
    }
  }
}

/// Mouse button move handler
void NodesContext::on_mouse_move(float x, float y)
{
  if (m_active_part) {
    // TODO: Snapping
    // TODO: Correct parentness support
    // TODO: Don't move non-movable nodes
    // TODO: Mirror symmetry support
    Vector3 cur_pos = calculate_gizmo_point();

    Vector4 axis;
    switch (m_active_part->m_axis) {
      case aX: axis = Vector4(1.0, 0.0, 0.0, 0.0); break;
      case aY: axis = Vector4(0.0, 1.0, 0.0, 0.0); break;
      case aZ: axis = Vector4(0.0, 0.0, 1.0, 0.0); break;
    }
    Vector3 axis3 = (m_gizmo->GetWorldTransform() * axis).Normalized();

    auto selected = m_view->selected();
    for (int i = 0; i < selected.Size(); ++i) {
      Node* node = selected[i];
      Vector3 node_pos = node->GetWorldPosition();
      // Calculate orientation
      Vector3 parent_pos = node->GetParent()->GetWorldPosition();
      Vector3 parent_z = node->GetParent()->GetWorldTransform() * Vector4(0, 0, 1, 0);
      // TODO: handle zero position
      Vector3 parent_y = (node_pos - parent_pos).Normalized();
      Vector3 parent_x = parent_y.CrossProduct(parent_z).Normalized();
      parent_y = parent_z.CrossProduct(parent_x).Normalized();

      Vector3 axis;
      switch (m_active_part->m_axis){
        case aX: axis = parent_x; break;
        case aY: axis = parent_y; break;
        case aZ: axis = parent_z; break;
      }

      if (m_active_part->m_rotation) {
        Vector3 a1 = (m_gizmo_pos - m_gizmo->GetWorldPosition()).Normalized();
        Vector3 a2 = (cur_pos - m_gizmo->GetWorldPosition()).Normalized();
        float sina = a1.CrossProduct(a2).DotProduct(axis3);
        float a = asin(sina) * 180 / M_PI;
        node->Rotate(Quaternion(a, axis), TS_WORLD);
      } else {
        // TODO: better calculation of X translate (add rotation)
        float distance = axis3.DotProduct(cur_pos - m_gizmo_pos);
        if (m_active_part->m_axis != aX) {
          node->Translate(distance * axis, TS_WORLD);
        } else {
          Vector3 local_pos = node_pos - parent_pos;
          float a = -distance / local_pos.Length();
          Quaternion tr(a *180 / M_PI, parent_z);
          Vector3 new_pos = tr * local_pos + parent_pos;
          node->Translate(new_pos - node_pos, TS_WORLD);
          node->Rotate(tr, TS_WORLD);
        }
      }
    }
    m_gizmo_pos = cur_pos;
  }
}

/// Update context each frame
void NodesContext::update(float dt)
{
  UI* ui = GetSubsystem<UI>();
  if (m_gizmo->IsEnabled()) {
    // Update gizmo position
    auto selected = m_view->selected();
    if (selected.Size()) {
      Node* node = selected.Back();
      Vector3 node_pos = node->GetWorldPosition();
      m_gizmo->SetPosition(node_pos);

      // Calculate orientation
      Vector3 parent_pos = node->GetParent()->GetWorldPosition();
      Vector3 parent_z = node->GetParent()->GetWorldTransform() * Vector4(0, 0, 1, 0);
      // TODO: handle zero position
      Vector3 parent_y = (node_pos - parent_pos).Normalized();
      Vector3 parent_x = parent_y.CrossProduct(parent_z).Normalized();
      parent_y = parent_z.CrossProduct(parent_x).Normalized();
      m_gizmo->SetRotation(Quaternion(parent_x, parent_y, parent_z));
    }

    // Update gizmo scale
    Camera* camera = m_view->camera()->GetComponent<Camera>();
    float scale = 0.1 / camera->GetZoom();

    if (camera->IsOrthographic()) {
      scale *= camera->GetOrthoSize();
    } else{
      scale *= (camera->GetView() * m_gizmo->GetPosition()).z_;
    }
    m_gizmo->SetScale(Vector3(scale, scale, scale));

    if (!m_active_part) {
      // Trace gizmo parts
      Ray ray = calculate_ray();
      Matrix3x4 inverse(m_gizmo->GetWorldTransform().Inverse());
      Ray local_ray = ray.Transformed(inverse);

      float t = M_INFINITY;
      m_focus_part = nullptr;
      PODVector<RayQueryResult> results;
      RayOctreeQuery query(results, ray, RAY_OBB, 2000, DRAWABLE_GEOMETRY);
      for (int i = 0; i < m_gizmo_parts.Size(); ++i) {
        m_gizmo_parts[i].m_component->ProcessRayQuery(query, results);
        if (results.Size() && results[0].distance_ < t) {
          m_focus_part = &m_gizmo_parts[i];
          t = results[0].distance_;
        }
        results.Clear();
      }
      ResourceCache* cache = GetSubsystem<ResourceCache>();
      for (int i = 0; i < m_gizmo_parts.Size(); ++i) {
        auto& axis = m_gizmo_parts[i];
        if (m_focus_part != &axis) {
          switch (axis.m_axis) {
            case aX: axis.m_component->SetMaterial(0, cache->GetResource<Material>("Materials/Editor/RedUnlit.xml")); break;
            case aY: axis.m_component->SetMaterial(0, cache->GetResource<Material>("Materials/Editor/GreenUnlit.xml")); break;
            case aZ: axis.m_component->SetMaterial(0, cache->GetResource<Material>("Materials/Editor/BlueUnlit.xml")); break;
          }
        } else {
          switch (axis.m_axis) {
            case aX: axis.m_component->SetMaterial(0, cache->GetResource<Material>("Materials/Editor/BrightRedUnlit.xml")); break;
            case aY: axis.m_component->SetMaterial(0, cache->GetResource<Material>("Materials/Editor/BrightGreenUnlit.xml")); break;
            case aZ: axis.m_component->SetMaterial(0, cache->GetResource<Material>("Materials/Editor/BrightBlueUnlit.xml")); break;
          }
        }
      }
    }
  }

  if (!m_focus_part) {
    Node* node = get_unit_under_mouse();
    if (node) {
      ui->GetCursor()->SetShape(CS_ACCEPTDROP);
    } else {
      ui->GetCursor()->SetShape(CS_NORMAL);
    }
  } else {
    ui->GetCursor()->SetShape(CS_RESIZE_ALL);
  }
}

/// Calculates closest point for current gizmo part
Vector3 NodesContext::calculate_gizmo_point()
{
  if (m_active_part) {
    Ray ray = calculate_ray();
    Vector4 axis;
    switch (m_active_part->m_axis) {
      case aX: axis = Vector4(1.0, 0.0, 0.0, 0.0); break;
      case aY: axis = Vector4(0.0, 1.0, 0.0, 0.0); break;
      case aZ: axis = Vector4(0.0, 0.0, 1.0, 0.0); break;
    }
    if (m_active_part->m_rotation) {
      Plane plane(m_gizmo->GetWorldTransform() * axis, m_gizmo->GetWorldPosition());
      float t = ray.HitDistance(plane);
      if (t < M_INFINITY) {
        return ray.origin_ + ray.direction_ * t;
      }
    } else {
      Ray axis_ray(m_gizmo->GetWorldPosition(), m_gizmo->GetWorldTransform() * axis);
      return axis_ray.ClosestPoint(ray);
    }
  }
  return Vector3();
}
