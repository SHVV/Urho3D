// SHW Spacecraft editor
//
// Context for creating multiple points units like connection trusses

#include "MultiPointCreationContext.h"

// Includes from Editor
#include "../Model/DynamicModel.h"
#include "../Model/SceneModel.h"
#include "../Model/MultiPointUnit.h"
#include "../Model/BasePositioner.h"
#include "../Model/BaseAttachableSurface.h"
#include "../Model/SurfaceNodePositioner.h"
#include "../Model/SurfaceSurfacePositioner.h"
#include "../Model/SurfaceMount.h"
#include "../View/SceneView.h"

// Includes from Urho3D
#include "Urho3D/Core/Context.h"
#include "Urho3D/Graphics/Graphics.h"
#include "Urho3D/Graphics/Camera.h"
#include "Urho3D/Graphics/Octree.h"
#include "Urho3D/Scene/Node.h"
#include "Urho3D/Scene/Scene.h"
#include "Urho3D/UI/UI.h"
#include "Urho3D/Input/Input.h"

using namespace Urho3D;

/// Construct.
MultiPointCreationContext::MultiPointCreationContext(Context* context)
  : CreationContext(context)
{
}

/// Destructor
MultiPointCreationContext::~MultiPointCreationContext()
{
}

/// Create rollower
void MultiPointCreationContext::create_rollower()
{
  CreationContext::create_rollower();
  for (int i = 0; i < m_rollowers.Size(); ++i) {
    auto unit = dynamic_cast<MultiPointUnit*>(m_rollowers[i].Get());
    if (unit) {
      m_more_nodes = 
        unit->set_reference_node(m_state, model()->scene_root()->CreateChild());
    }
  }
  update_rollower_position();
}

/// Updates rollower position
void MultiPointCreationContext::update_rollower_position()
{
  if (m_rollowers.Size()) {
    // Symmetry
    Node* first_rollower_node = m_rollowers[0]->GetNode();
    Vector3 attach_position;
    Vector3 attach_normal;
    Node* unit_under_mouse = get_unit_under_mouse(&attach_position, &attach_normal);
    // TODO: use some more explicit way to detect possible attachments
    // If module has attable surface - use free positioner, or surface - surface
    if (unit_under_mouse) {
      // Attachment
      // TODO: choose attachment type externally
      StringHash positioner_type = SurfaceNodePositioner::GetTypeStatic();
      auto surface = unit_under_mouse->GetDerivedComponent<BaseAttachableSurface>();
      if (surface) {
        // Get symmetry nodes
        auto attach_nodes = get_symmety_nodes(unit_under_mouse);
        // If more than one unit
        if (attach_nodes.Size() > 1) {
          // Use underlaying nodes as a guide
          for (int i = 0; i < m_rollowers.Size(); ++i) {
            auto unit = dynamic_cast<MultiPointUnit*>(m_rollowers[i].Get());
            if (unit) {
              Node* rollower_node = unit->GetNode();
              Node* reference_node = unit->get_reference_node(m_state);
              if (reference_node && i < attach_nodes.Size()) {
                // Set new parent
                reference_node->SetParent(attach_nodes[i]);
                // Get positioner
                auto positioner =
                  get_or_create_positioner(reference_node, positioner_type);
                auto spec_positioner = dynamic_cast<SurfaceNodePositioner*>(positioner);
                if (spec_positioner) {
                  spec_positioner->set_sub_objects(
                    (int)SubObjectType::VERTEX | (int)SubObjectType::POLYGON
                  );
                }
                // and set position
                rollower_node->SetEnabledRecursive(
                  positioner->set_position(attach_position, attach_normal, m_orientation)
                );
              } else {
                rollower_node->SetEnabledRecursive(false);
              }
            }
          }
        } else {
          // for just one unit - use symmetry positions
          auto positions = get_symmetry_positions(attach_position);
          // and normals
          auto normals = get_symmetry_positions(attach_normal);
          for (int i = 0; i < m_rollowers.Size(); ++i) {
            auto unit = dynamic_cast<MultiPointUnit*>(m_rollowers[i].Get());
            if (unit) {
              Node* rollower_node = unit->GetNode();
              Node* reference_node = unit->get_reference_node(m_state);
              if (reference_node) {
                // Set new parent
                reference_node->SetParent(unit_under_mouse);
                // Get positioner
                auto positioner =
                  get_or_create_positioner(reference_node, positioner_type);
                auto spec_positioner = dynamic_cast<SurfaceNodePositioner*>(positioner);
                if (spec_positioner) {
                  spec_positioner->set_sub_objects(
                    (int)SubObjectType::VERTEX | (int)SubObjectType::POLYGON
                  );
                }
                // and set position
                rollower_node->SetEnabledRecursive(
                  positioner->set_position(positions[i], normals[i], m_orientation)
                );
              }
            }
          }
        }

        if (m_rollowers[0]->GetNode()->IsEnabled()) {
          set_tooltip("Attach to module");
        }
        return;
      }
    } else {
      // Unit without attachable surface cannot be placed in vaccum hide rollowers
      for (int i = 0; i < m_rollowers.Size(); ++i) {
        m_rollowers[i]->GetNode()->SetEnabledRecursive(false);
      }
      hide_tooltip();
    }
  }
}

/// Mouse button down handler
void MultiPointCreationContext::on_mouse_down()
{
}

/// Mouse button up handler
void MultiPointCreationContext::on_mouse_up()
{
  if (!m_more_nodes) {
    // TODO: factor out common code

    // Save parameters for late reusing
    if (m_rollowers.Size()) {
      Pair<StringHash, StringHash> key(unit_class(), function_name());
      m_latest_parameters[key] = m_rollowers[0]->parameters();
    }

    for (int i = 0; i < m_rollowers.Size(); ++i) {
      Node* node = m_rollowers[i]->GetNode();
      if (!node->IsEnabled()) {
        model()->delete_unit(node);
      }
    }

    m_rollowers.Clear();
    view()->clear_selection();
    commit_transaction();
    create_rollower();
  } else {
    // TODO: factor out common code
    ++m_state;
    for (int i = 0; i < m_rollowers.Size(); ++i) {
      auto unit = dynamic_cast<MultiPointUnit*>(m_rollowers[i].Get());
      if (unit) {
        m_more_nodes =
          unit->set_reference_node(m_state, model()->scene_root()->CreateChild());
      }
    }
  }
}

/// Mouse button move handler
void MultiPointCreationContext::on_mouse_move(float x, float y)
{
  update_rollower_position();
}
