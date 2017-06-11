// SHW Spacecraft editor
//
// Basic procedural unit. Contains link to function for generating guts.
// Default implementation - generate procedural geometry model

#include "ProceduralUnit.h"

#include "../Core/MeshGeometry.h"
#include "../Core/MeshBuffer.h"
#include "../Core/MeshGenerator.h"

#include "DynamicModel.h"

#include <Urho3D\Core\Context.h>

using namespace Urho3D;

extern const char* EDITOR_CATEGORY;

/// Register object attributes.
void ProceduralUnit::RegisterObject(Context* context)
{
  context->RegisterFactory<ProceduralUnit>(EDITOR_CATEGORY);

  URHO3D_COPY_BASE_ATTRIBUTES(UnitModel);
}

/// Construct.
ProceduralUnit::ProceduralUnit(Context* context)
  : UnitModel(context)
{
}

/// Destructor
ProceduralUnit::~ProceduralUnit()
{
}

/// Get all parameters description of the component
const ParametersDescription& ProceduralUnit::parameters_description() const
{
  auto generator = GetSubsystem<MeshGenerator>();
  return generator->parameters_description(m_function);
}

/// Set function name
void ProceduralUnit::set_function_name(StringHash name)
{
  m_function = name;
  auto generator = GetSubsystem<MeshGenerator>();
  set_parameters(generator->default_parameters(m_function));
}

/// Set function name
StringHash ProceduralUnit::function_name()
{
  return m_function;
}

/// Create or update all necessary components - override for derived classes
void ProceduralUnit::update_guts_int()
{
  if (m_function) {
    auto generator = GetSubsystem<MeshGenerator>();
    MeshBuffer* mesh_buffer = generator->generate_buffer(m_function, parameters());
    if (mesh_buffer) {
      DynamicModel* dynamic_model = get_component<DynamicModel>();
      dynamic_model->mesh_buffer(mesh_buffer);
    }
  }
}
