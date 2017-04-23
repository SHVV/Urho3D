// SHW Spacecraft editor
//
// Class for mesh generation function.
// Gets Parameters as input and returns MeshGeometry.

#include "MeshGenerationFunction.h"

// Editor includes
#include "MeshGenerator.h"

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

/// Constructor with parameters.
MeshGenerationFunction::MeshGenerationFunction(String name)
: m_name(name)
{
}

/// Set default parameters
void MeshGenerationFunction::set_default_parameters(
  const Parameters& default_parameters
)
{
  m_default_parameters = default_parameters;
}

/// Get generator
MeshGenerator* MeshGenerationFunction::generator()
{
  return m_generator;
}
