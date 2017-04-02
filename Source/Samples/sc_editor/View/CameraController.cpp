// SHW Spacecraft editor
//
// Default camera controller for editor

#include "CameraController.h"

// Includes from Editor
#include "../Model/NodeModel.h"
#include "../Model/SceneModel.h"
#include "../View/SceneView.h"

// Includes from Urho3D
#include "Urho3D/Graphics/Graphics.h"
#include "Urho3D/Graphics/Camera.h"
#include "Urho3D/Graphics/Octree.h"
#include "Urho3D/Scene/Node.h"
#include "Urho3D/Scene/Scene.h"
#include "Urho3D/UI/UI.h"
#include "Urho3D/Input/Input.h"

using namespace Urho3D;

/// Construct.
CameraController::CameraController(Context* context, SceneView* view)
: Object(context),
  m_view(view)
{
}

/// Destructor
CameraController::~CameraController()
{

}

/// Mouse button down handler
void CameraController::on_mouse_down(VariantMap& event_data)
{
  // On mouse down just update camera target
  m_target = m_view->camera()->GetWorldPosition();

  Camera* camera = m_view->camera()->GetComponent<Camera>();
  PODVector<Drawable*> results;
  FrustumOctreeQuery query(results, camera->GetFrustum());
  m_view->scene()->GetComponent<Octree>()->GetDrawables(query);
  BoundingBox visible_scene_box;
  for (int i = 0; i < results.Size(); ++i) {
    visible_scene_box.Merge(results[i]->GetWorldBoundingBox());
  };
  // TODO: build intesection between world BB and frustrum
  if (visible_scene_box.Defined()) {
    Vector3 potential_target = visible_scene_box.Center();
    Node* node = get_unit_under_mouse();
    if (node) {
      //auto drawable = node->GetDerivedComponent<Drawable>();
      Vector3 node_center = node->GetWorldTransform().Translation();

      // Check camera distance
      // TODO: check angular size instead of distance
      float scene_dist = (m_target - potential_target).Length();
      float node_dist = (m_target - node_center).Length();
      if (1.3 * node_dist < scene_dist) {
        potential_target = node_center;
      }
    }

    Ray ray = calculate_ray();
    Vector3 projection = ray.Project(potential_target);
    if (ray.direction_.DotProduct(projection - ray.origin_) > 0) {
      m_target = projection;
    }
  }
}

/// Update controller on each frame
void CameraController::update(float dt)
{
  // Do not move if the UI has a focused element (the console)
  if (GetSubsystem<UI>()->GetFocusElement())
    return;

  Input* input = GetSubsystem<Input>();
  Node* camera = m_view->camera();

  // Mouse sensitivity as degrees per pixel
  const float MOUSE_SENSITIVITY = 0.1f;

  // Scale speeds, based on target distance, or distance from origin
  float distance = (m_target - camera->GetWorldPosition()).Length();
  float origin_distance = camera->GetWorldPosition().Length();
  float mult = ((distance < 1) ? origin_distance : distance) / 50;

  // Clamp to some minimum values, if distance is zero
  if (mult < 0.1) {
    mult = 0.1;
  }

  // Speed up on pressing Shift
  if (input->GetKeyDown(KEY_SHIFT)) {
    mult *= 3;
  }

  // Mouse sensitivity as units per pixel
  const float MOUSE_SENSITIVITY_UNITS = 0.02f * mult;
  // Mouse wheel sensitivity as units per pixel
  const float MOUSE_WHEEL_SPEED = 1.5f * mult;

  // Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
  IntVector2 mouseMove = input->GetMouseMove();
  if (mouseMove.Length() > 0) {
    if (input->GetMouseButtonDown(MOUSEB_RIGHT)) {
      Vector3 angles = camera->GetRotation().EulerAngles();
      float yaw = angles.y_ + MOUSE_SENSITIVITY * mouseMove.x_;
      float pitch = angles.x_ + MOUSE_SENSITIVITY * mouseMove.y_;
      pitch = Clamp(pitch, -80.0f, 80.0f);
      //Vector3 proj = ray.Project(m_target);
      // Construct new orientation for the camera scene node from yaw and pitch. Roll is fixed to zero
      camera->SetRotation(Quaternion(pitch, yaw, 0.0f));
      Ray ray = calculate_ray();
      Vector3 new_pos = ray.direction_ * -distance + m_target;
      camera->Translate(new_pos - camera->GetWorldPosition(), TS_WORLD);

      //Vector3 trans = Quaternion(pitch, yaw, 0.0f) * Vector3(0, 0, -distance) + m_target - camera->GetWorldPosition();
      //camera->Translate(trans);
      //camera->LookAt(m_target);
      //cameraNode_->Translate((Vector3::LEFT * mouseMove.x_ + Vector3::UP * mouseMove.y_) * 0.01);
    }

    if (input->GetMouseButtonDown(MOUSEB_MIDDLE)) {
      camera->Translate((Vector3::LEFT * mouseMove.x_ + Vector3::UP * mouseMove.y_) * MOUSE_SENSITIVITY_UNITS);
    }
  }
  camera->Translate((Vector3::FORWARD * input->GetMouseMoveWheel()) * MOUSE_WHEEL_SPEED);

  /*// Read WASD keys and move the camera scene node to the corresponding direction if they are pressed
  // Movement speed as world units per second
  const float MOVE_SPEED = 20.0f;
  if (input->GetKeyDown(KEY_W))
    camera->Translate(Vector3::FORWARD * MOVE_SPEED * dt);
  if (input->GetKeyDown(KEY_S))
    camera->Translate(Vector3::BACK * MOVE_SPEED * dt);
  if (input->GetKeyDown(KEY_A))
    camera->Translate(Vector3::LEFT * MOVE_SPEED * dt);
  if (input->GetKeyDown(KEY_D))
    camera->Translate(Vector3::RIGHT * MOVE_SPEED * dt);*/
}

/// Get unit under mouse cursor
Node* CameraController::get_unit_under_mouse()
{
  Ray ray = calculate_ray();

  PODVector<RayQueryResult> results;
  RayOctreeQuery query(results, ray, RAY_OBB, 2000, DRAWABLE_GEOMETRY);
  m_view->scene()->GetComponent<Octree>()->Raycast(query);

  float t = M_INFINITY;
  Node* res = nullptr;
  for (int i = 0; i < results.Size(); ++i){
    RayQueryResult& result = results[i];
    Node* node = result.drawable_->GetNode();
    UnitModel* unit = node->GetComponent<UnitModel>();
    if (unit) {
      MeshGeometry* mesh = unit->structure_mesh();
      if (mesh) {
        Matrix3x4 inverse(node->GetWorldTransform().Inverse());
        Ray local_ray = ray.Transformed(inverse);

        SubObjectType sub_type;
        if (mesh->raycast(local_ray, sub_type, sotPOLYGON, true, t) >= 0) {
          res = node;
        }
      }
    }
  }
  return res;
}

/// Get ray from current mouse position and camera
Ray CameraController::calculate_ray()
{
  Graphics* graphics = GetSubsystem<Graphics>();
  Camera* camera = m_view->camera()->GetComponent<Camera>();
  return camera->GetScreenRay(0.5, 0.5);
}
