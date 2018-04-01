// SHW Spacecraft editor
//
// Class for mesh generation function.
// Gets Parameters as input and returns MeshGeometry.

#include "MeshGenerationFunction.h"

// Editor includes
#include "MeshGenerator.h"
#include "../Core/MeshGeometry.h"
#include "../Model/ProceduralUnit.h"
#include "../Model/DynamicModel.h"
#include "../Model/BaseAttachableSurface.h"

/// Destructor
MeshGenerationFunction::~MeshGenerationFunction()
{

}

/// Sets back pointer to generator
void MeshGenerationFunction::generator(MeshGenerator* generator)
{
  m_generator = generator;
}

/// Returns function name
const String& MeshGenerationFunction::name() const
{
  return m_name;
}

/// Returns default parameters
const Parameters& MeshGenerationFunction::default_parameters() const
{
  return m_default_parameters;
}

// Parameters description
const ParametersDescription& MeshGenerationFunction::parameters_description() const
{
  return m_parameters_description;
}

/// Returns default parameters
Parameters& MeshGenerationFunction::access_default_parameters()
{
  return m_default_parameters;
}

// Parameters description
ParametersDescription& MeshGenerationFunction::access_parameters_description()
{
  return m_parameters_description;
}

/// Constructor with parameters.
MeshGenerationFunction::MeshGenerationFunction(String name)
: m_name(name)
{
}

/// Update unit, by using provided parameters
void MeshGenerationFunction::update_unit(
  const Parameters& parameters, 
  ProceduralUnit* unit
)
{
  // Default implementation - just self dynamic model.
  MeshBuffer* mesh_buffer = m_generator->generate_buffer(m_name, parameters);
  if (mesh_buffer) {
    DynamicModel* dynamic_model = unit->get_component<DynamicModel>();
    dynamic_model->mesh_buffer(mesh_buffer);
    // If mesh contains attachable nodes - create base attachable surface component
    if (mesh_buffer->mesh_geometry()->vertices_by_flags(mgfATTACHABLE).Size() > 0){
      unit->get_component<BaseAttachableSurface>();
    }
  }
}

/// Set default parameters
void MeshGenerationFunction::set_default_parameters(
  const Parameters& default_parameters
)
{
  m_default_parameters = default_parameters;
}


/// Add parameter
void MeshGenerationFunction::add_parameter(
  const ParameterID& id,
  const Variant& default_value,
  const int& flags,
  const String& name,
  const String& tooltip,
  const Variant& min,
  const Variant& max
)
{
  m_default_parameters[id] = default_value;

  ParameterDescription description;
  description.m_flags = flags;
  description.m_name = name;
  description.m_tooltip = tooltip;
  description.m_min = min;
  description.m_max = max;
  m_parameters_description[id] = description;
  m_parameters_description.parameter_ids().Push(id);
}

/// Get generator
MeshGenerator* MeshGenerationFunction::generator()
{
  return m_generator;
}
