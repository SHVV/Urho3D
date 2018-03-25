// SHW Spacecraft editor
//
// Base positioner component. Knows, how to insert unit into space or another unit.

#include "BasePositioner.h"

#include <Urho3D\Core\Context.h>
#include <Urho3D\Scene\Node.h>

using namespace Urho3D;

extern const char* EDITOR_CATEGORY;


/// Register object attributes.
void BasePositioner::RegisterObject(Context* context)
{
  context->RegisterFactory<BasePositioner>(EDITOR_CATEGORY);

  // TODO: add Attributes
  //URHO3D_ENUM_ATTRIBUTE("State", m_state, state_names, usNONE, AM_NET | AM_NOEDIT);
}

/// Construct.
BasePositioner::BasePositioner(Context* context)
: Component(context)
{
}

/// Destructor
BasePositioner::~BasePositioner()
{
}

/// Updates internal position representation, based on current node position.
void BasePositioner::update_internal_position()
{
  // Nothing to do in base positioner
}

/// Updates node position, based on reference and internal position.
void BasePositioner::update_node_position()
{
  // Nothing to do in base positioner
}

/// Get supported moves along local linear axis
Axis BasePositioner::linear_axis()
{
  if (is_on_z_axis()) {
    return Axis::Z;
  } else {
    return Axis::XYZ;
  }
}

/// Get supported rotations around local linear axis
Axis BasePositioner::rotation_axis()
{
  return linear_axis();
}

/// Returns default move space
MoveSpace BasePositioner::move_space()
{
  return MoveSpace::PARENT;
}

/// Handle node being assigned.
void BasePositioner::OnNodeSet(Node* node)
{
  if (node) {
    update_internal_position();
  }
}

/// Checks, is node lays on parent Z axis.
bool BasePositioner::is_on_z_axis()
{
  Vector3 node_pos = node_->GetWorldPosition();
  Node* parent = node_->GetParent();
  Vector3 parent_pos = parent->GetWorldPosition();
  Vector3 parent_z = parent->GetWorldTransform() * Vector4(0, 0, 1, 0);
  Vector3 parent_y = (node_pos - parent_pos).Normalized();
  Vector3 parent_x = parent_y.CrossProduct(parent_z).Normalized();

  return parent_x.LengthSquared() < 0.9;
}

/*/// Handle node (not only our) transform being dirtied.
void BasePositioner::OnMarkedDirty(Node* node)
{
update_links();
}*/
