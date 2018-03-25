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
NodesContext::NodesContext(Context* context)
: BaseContext(context),
  m_focus_part(nullptr),
  m_active_part(nullptr)
{
}

/// Destructor
NodesContext::~NodesContext()
{
}

/// Initialize context
void NodesContext::initialize()
{
  ResourceCache* cache = GetSubsystem<ResourceCache>();

  m_gizmo = view()->scene()->CreateChild("NodesGimbal", LOCAL);
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

  // Linear axis
  SharedPtr<MeshGeometry> mesh_ring(generator->lathe(profile_ring, 64, ttQUAD));
  {
    SharedPtr<MeshBuffer> mesh_buffer(new MeshBuffer(context_));
    mesh_buffer->set_mesh_geometry(mesh);
    GizmoPart axis;
    axis.m_component = m_gizmo->CreateComponent<StaticModel>();
    axis.m_component->SetOccludee(false);
    axis.m_component->SetModel(mesh_buffer->model());
    axis.m_component->SetMaterial(0, cache->GetResource<Material>("Materials/Editor/BlueUnlit.xml"));
    axis.m_axis = Axis::Z;
    axis.m_rotation = false;
    m_gizmo_parts.Push(axis);

    SharedPtr<MeshBuffer> mesh_buffer_ring(new MeshBuffer(context_));
    mesh_buffer_ring->set_mesh_geometry(mesh_ring);
    axis.m_component = m_gizmo->CreateComponent<StaticModel>();
    axis.m_component->SetOccludee(false);
    axis.m_component->SetModel(mesh_buffer_ring->model());
    axis.m_component->SetMaterial(0, cache->GetResource<Material>("Materials/Editor/BlueUnlit.xml"));
    axis.m_axis = Axis::Z;
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
    SharedPtr<MeshBuffer> mesh_buffer(new MeshBuffer(context_));
    mesh_buffer->set_mesh_geometry(mesh);
    GizmoPart axis;
    axis.m_component = m_gizmo->CreateComponent<StaticModel>();
    axis.m_component->SetOccludee(false);
    axis.m_component->SetModel(mesh_buffer->model());
    axis.m_component->SetMaterial(0, cache->GetResource<Material>("Materials/Editor/GreenUnlit.xml"));
    axis.m_axis = Axis::Y;
    axis.m_rotation = false;
    m_gizmo_parts.Push(axis);

    mesh_ring->transform(tr);
    SharedPtr<MeshBuffer> mesh_buffer_ring(new MeshBuffer(context_));
    mesh_buffer_ring->set_mesh_geometry(mesh_ring);
    axis.m_component = m_gizmo->CreateComponent<StaticModel>();
    axis.m_component->SetOccludee(false);
    axis.m_component->SetModel(mesh_buffer_ring->model());
    axis.m_component->SetMaterial(0, cache->GetResource<Material>("Materials/Editor/GreenUnlit.xml"));
    axis.m_axis = Axis::Y;
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
    SharedPtr<MeshBuffer> mesh_buffer(new MeshBuffer(context_));
    mesh_buffer->set_mesh_geometry(mesh);
    GizmoPart axis;
    axis.m_component = m_gizmo->CreateComponent<StaticModel>();
    axis.m_component->SetOccludee(false);
    axis.m_component->SetModel(mesh_buffer->model());
    axis.m_component->SetMaterial(0, cache->GetResource<Material>("Materials/Editor/RedUnlit.xml"));
    axis.m_axis = Axis::X;
    axis.m_rotation = false;
    m_gizmo_parts.Push(axis);

    mesh_ring->transform(tr);
    SharedPtr<MeshBuffer> mesh_buffer_ring(new MeshBuffer(context_));
    mesh_buffer_ring->set_mesh_geometry(mesh_ring);
    axis.m_component = m_gizmo->CreateComponent<StaticModel>();
    axis.m_component->SetOccludee(false);
    axis.m_component->SetModel(mesh_buffer_ring->model());
    axis.m_component->SetMaterial(0, cache->GetResource<Material>("Materials/Editor/RedUnlit.xml"));
    axis.m_axis = Axis::X;
    axis.m_rotation = true;
    m_gizmo_parts.Push(axis);
  }

  /*gizmo->SetModel(cache->GetResource<Model>("Models/Editor/Axes.mdl"));
  gizmo->SetMaterial(0, cache->GetResource<Material>("Materials/Editor/RedUnlit.xml"));
  gizmo->SetMaterial(1, cache->GetResource<Material>("Materials/Editor/GreenUnlit.xml"));
  gizmo->SetMaterial(2, cache->GetResource<Material>("Materials/Editor/BlueUnlit.xml"));*/
  //gizmo.viewMask = 0x80000000; // Editor raycasts use viewmask 0x7fffffff
}

/// Activates context and allows it to set up all its guts
void NodesContext::activate()
{
  BaseContext::activate();
  update_gizmo();
}

/// Deactivate context and remove all temporary objects
void NodesContext::deactivate()
{
  m_gizmo->SetEnabled(false);
  m_focus_part = nullptr;
  //view()->clear_selection();
  BaseContext::deactivate();
}

/// Updates gizmo state
void NodesContext::update_gizmo()
{
  auto selected = view()->selected();
  if (selected.Size()) {
    m_gizmo->SetEnabled(true);
    // Calculate most restrictive set of available axis
    Axis linear_axis = Axis::XYZ;
    Axis rotation_axis = Axis::XYZ;
    for (int i = 0; i < selected.Size(); ++i) {
      Node* node = selected[i];
      BasePositioner* positioner = node->GetComponent<BasePositioner>();
      if (positioner) {
        linear_axis = (Axis)((int)linear_axis & (int)positioner->linear_axis());
        rotation_axis = (Axis)((int)rotation_axis & (int)positioner->rotation_axis());
      }
    }
    // And enable only allowed components
    for (int i = 0; i < m_gizmo_parts.Size(); ++i) {
      auto& axis = m_gizmo_parts[i];
      axis.m_component->SetEnabled(
        !!((int)axis.m_axis & int(axis.m_rotation ? rotation_axis : linear_axis))
      );
    }
  } else {
    m_gizmo->SetEnabled(false);
  }
}

/// Mouse button down handler
void NodesContext::on_mouse_down()
{
  if (m_focus_part) {
    m_active_part = m_focus_part;
    m_gizmo_pos = calculate_gizmo_point();

    // Copy all positions and rotations of selected nodes
    m_original_nodes_pos.Clear();
    m_original_nodes_rot.Clear();
    auto selected = view()->selected();
    for (int i = 0; i < selected.Size(); ++i) {
      Node* node = selected[i];
      m_original_nodes_pos.Push(node->GetWorldPosition());
      m_original_nodes_rot.Push(node->GetWorldRotation());
    }
  }
}

/// Mouse button up handler
void NodesContext::on_mouse_up()
{
  Input* input = GetSubsystem<Input>();
  if (m_active_part) {
    //
    m_active_part = nullptr;
    hide_tooltip();

    // Undo support
    commit_transaction();
  } else {
    Node* node = get_unit_under_mouse();
    // Go up by hierarhy until node with positioner will be found.
    BasePositioner* positioner;
    while (node && !(positioner = node->GetDerivedComponent<BasePositioner>())) {
      node = node->GetParent();
    }

    Vector<Node*> symmetry_nodes = get_symmety_nodes(node);
    if (input->GetKeyDown(KEY_CTRL)) {
      if (node) {
        auto selected = view()->selected();
        if (selected.Find(node) != selected.End()) {
          view()->deselect(symmetry_nodes);
        } else {
          view()->select(symmetry_nodes);
        }
      }
    } else {
      view()->clear_selection();
      if (node) {
        view()->select(symmetry_nodes);
      }
    }
  }
  update_gizmo();
}

/// Mouse button move handler
void NodesContext::on_mouse_move(float x, float y)
{
  if (m_active_part) {
    // TODO: Correct parentness support
    // TODO: Don't move non-movable nodes
    // TODO: Mirror symmetry support
    Vector3 cur_pos = calculate_gizmo_point();

    Vector4 axis;
    switch (m_active_part->m_axis) {
      case Axis::X: axis = Vector4(1.0, 0.0, 0.0, 0.0); break;
      case Axis::Y: axis = Vector4(0.0, 1.0, 0.0, 0.0); break;
      case Axis::Z: axis = Vector4(0.0, 0.0, 1.0, 0.0); break;
    }
    Vector3 axis3 = (m_gizmo->GetWorldTransform() * axis).Normalized();
    char deg[3] = { 0xC2, 0xB0, 0x0 };

    auto selected = view()->selected();
    float min_size = node_size(selected.Back());
    float move_step = quantizing_step(min_size * 2);
    for (int i = 0; i < selected.Size(); ++i) {
      Node* node = selected[i];
      Vector3 node_pos = node->GetWorldPosition();
      // Calculate orientation
      Vector3 parent_pos = node->GetParent()->GetWorldPosition();
      Vector3 parent_z = node->GetParent()->GetWorldTransform() * Vector4(0, 0, 1, 0);
      Vector3 parent_y = (node_pos - parent_pos).Normalized();
      Vector3 parent_x = parent_y.CrossProduct(parent_z).Normalized();
      parent_y = parent_z.CrossProduct(parent_x).Normalized();

      Vector3 axis;
      switch (m_active_part->m_axis){
        case Axis::X: axis = parent_x; break;
        case Axis::Y: axis = parent_y; break;
        case Axis::Z: axis = parent_z; break;
      }

      if (axis.LengthSquared() > 0.9) {
        if (m_active_part->m_rotation) {
          Vector3 a1 = (m_gizmo_pos - m_gizmo->GetWorldPosition()).Normalized();
          Vector3 a2 = (cur_pos - m_gizmo->GetWorldPosition()).Normalized();
          float sina = a1.CrossProduct(a2).DotProduct(axis3);
          float cosa = a1.DotProduct(a2);
          float a = atan2(sina, cosa) * M_RADTODEG;
          a = snap_angle(a);
          if (0 == i) {
            set_tooltip(String(a) + deg);
          }
          node->SetWorldRotation(Quaternion(a, axis) * m_original_nodes_rot[i]);
          //node->Rotate(Quaternion(a, axis), TS_WORLD);
        } else {
          float distance = axis3.DotProduct(cur_pos - m_gizmo_pos);
          distance = round(distance / move_step) * move_step;
          if (m_active_part->m_axis != Axis::X) {
            if (0 == i) {
              set_tooltip(String(distance) + "m");
            }
            node->SetWorldPosition(m_original_nodes_pos[i] + distance * axis);
            //node->Translate(distance * axis, TS_WORLD);
          } else {
            Plane gizmo_plane(parent_z, m_gizmo_pos);
            Vector3 projected_cur_pos = gizmo_plane.Project(cur_pos);
            Vector3 projected_parent_pos = gizmo_plane.Project(parent_pos);

            Vector3 a1 = (m_gizmo_pos - projected_parent_pos).Normalized();
            Vector3 a2 = (projected_cur_pos - projected_parent_pos).Normalized();

            float sina = a1.CrossProduct(a2).DotProduct(parent_z);
            float cosa = a1.DotProduct(a2);
            float a = atan2(sina, cosa) * M_RADTODEG;
            Vector3 local_pos = m_original_nodes_pos[i] - parent_pos;
            a = snap_angle(a);
            if (0 == i) {
              set_tooltip(String(a) + deg);
            }
            Quaternion tr(a, parent_z);
            Vector3 new_pos = tr * local_pos + parent_pos;
            node->SetWorldPosition(new_pos);
            node->SetWorldRotation(tr * m_original_nodes_rot[i]);

            //node->Translate(new_pos - node_pos, TS_WORLD);
            //node->Rotate(tr, TS_WORLD);
          }
        }
      }
    }
    //m_gizmo_pos = cur_pos;
  }
}

/// Update context each frame
void NodesContext::update(float dt)
{
  UI* ui = GetSubsystem<UI>();
  if (m_gizmo->IsEnabled()) {
    // Update gizmo position
    auto selected = view()->selected();
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
    Camera* camera = view()->camera()->GetComponent<Camera>();
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
        if (m_gizmo_parts[i].m_component->IsEnabled()) {
          m_gizmo_parts[i].m_component->ProcessRayQuery(query, results);
          if (results.Size() && results[0].distance_ < t) {
            m_focus_part = &m_gizmo_parts[i];
            t = results[0].distance_;
          }
          results.Clear();
        }
      }
      ResourceCache* cache = GetSubsystem<ResourceCache>();
      for (int i = 0; i < m_gizmo_parts.Size(); ++i) {
        auto& axis = m_gizmo_parts[i];
        if (m_focus_part != &axis) {
          switch (axis.m_axis) {
            case Axis::X: axis.m_component->SetMaterial(0, cache->GetResource<Material>("Materials/Editor/RedUnlit.xml")); break;
            case Axis::Y: axis.m_component->SetMaterial(0, cache->GetResource<Material>("Materials/Editor/GreenUnlit.xml")); break;
            case Axis::Z: axis.m_component->SetMaterial(0, cache->GetResource<Material>("Materials/Editor/BlueUnlit.xml")); break;
          }
        } else {
          switch (axis.m_axis) {
            case Axis::X: axis.m_component->SetMaterial(0, cache->GetResource<Material>("Materials/Editor/BrightRedUnlit.xml")); break;
            case Axis::Y: axis.m_component->SetMaterial(0, cache->GetResource<Material>("Materials/Editor/BrightGreenUnlit.xml")); break;
            case Axis::Z: axis.m_component->SetMaterial(0, cache->GetResource<Material>("Materials/Editor/BrightBlueUnlit.xml")); break;
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

  auto& selected = view()->selected();
  if (selected.Size()) {
    Input* input = GetSubsystem<Input>();
    if (input->GetKeyPress(KEY_DELETE)) {
      for (int i = 0; i < selected.Size(); ++i) {
        // TODO: deal with multi node units and child-parent relations
        model()->delete_unit(selected[i]);
      }
      view()->clear_selection();
      m_gizmo->SetEnabled(false);
      m_focus_part = nullptr;

      // Undo support
      commit_transaction();
    }
  }
  // Testing procedural units update
  //auto& selected = view()->selected();
  //if (selected.Size()) {
  //  Input* input = GetSubsystem<Input>();
  //  int delta = 0;
  //  if (input->GetKeyPress(KEY_KP_PLUS)) {
  //    delta = 1;
  //  }
  //  if (input->GetKeyPress(KEY_KP_MINUS)) {
  //    delta = -1;
  //  }
  //  if (delta) {
  //    for (int i = 0; i < selected.Size(); ++i) {
  //      Node* node = selected[i];
  //      UnitModel* unit = node->GetDerivedComponent<UnitModel>();
  //      if (unit) {
  //        unsigned int val = unit->parameters().parameters_vector()[0].GetUInt();
  //        unit->set_parameter(0, val + delta);
  //      }
  //    }
  //  }
  //}
}

/// Calculates closest point for current gizmo part
Vector3 NodesContext::calculate_gizmo_point()
{
  if (m_active_part) {
    Ray ray = calculate_ray();
    Vector4 axis;
    switch (m_active_part->m_axis) {
      case Axis::X: axis = Vector4(1.0, 0.0, 0.0, 0.0); break;
      case Axis::Y: axis = Vector4(0.0, 1.0, 0.0, 0.0); break;
      case Axis::Z: axis = Vector4(0.0, 0.0, 1.0, 0.0); break;
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
