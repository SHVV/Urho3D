// SHW Spacecraft editor
//
// Node Model class, represents one sphere node for structures

#include "NodeModel.h"

#include <Urho3D\Core\Context.h>

using namespace Urho3D;

extern const char* EDITOR_CATEGORY;

/// Register object attributes.
void NodeModel::RegisterObject(Context* context)
{
  context->RegisterFactory<NodeModel>(EDITOR_CATEGORY);

  // TODO: add Attributes
}

/// Construct.
NodeModel::NodeModel(Context* context)
: UnitModel(context),
  m_radius(0.1f)
{

}

/// Destructor
NodeModel::~NodeModel()
{

}

// Set node radius
void NodeModel::set_radius(float radius)
{
  m_radius = radius;
}

float NodeModel::radius() const
{
  // TODO: calculate, based on length of connected beams
  // TODO: use golden ratio for quantizing radius (1 + sqrt(5)) / 2 = 1.6180339887498948482

  return m_radius;
}

/// Handle node being assigned.
void NodeModel::OnNodeSet(Node* node)
{
  // TODO: create physics representation
}

/// Handle node transform being dirtied.
void NodeModel::OnMarkedDirty(Node* node)
{
  // TODO: update physics representation
}
