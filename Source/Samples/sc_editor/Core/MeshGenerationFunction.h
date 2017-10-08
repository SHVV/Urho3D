// SHW Spacecraft editor
//
// Class for mesh generation function.
// Gets Parameters as input and returns MeshGeometry.

#pragma once

// Editor includes
#include "Parameters.h"
#include "ParametersDescription.h"

// Urho3D includes
#include <Urho3D/Container/RefCounted.h>

// Forward declaration
class MeshGeometry; 
class MeshGenerator;
class ProceduralUnit;

using namespace Urho3D;

class MeshGenerationFunction : public RefCounted
{
public:
  /// Destructor
  virtual ~MeshGenerationFunction();

  /// Sets back pointer to generator
  void generator(MeshGenerator* generator);

  /// Returns function name
  const String& name() const;

  /// Returns default parameters
  const Parameters& default_parameters() const;

  // Parameters description
  const ParametersDescription& parameters_description() const;

  /// Generate mesh geometry, basing on parameters
  virtual MeshGeometry* generate(const Parameters& parameters) = 0;

  /// Update procedural unit guts
  virtual void update_unit(const Parameters& parameters, ProceduralUnit* unit);

protected:
  /// Constructor with parameters.
  MeshGenerationFunction(
    String name
  );

  /// Set default parameters
  void set_default_parameters(const Parameters& default_parameters);

  /// Add parameter
  void add_parameter(
    const ParameterID& id,
    const Variant& default_value,
    const int& flags,
    const String& name,
    const String& tooltip,
    const Variant& min = Variant(),
    const Variant& max = Variant()
  );

  /// Returns default parameters
  Parameters& access_default_parameters();

  // Parameters description
  ParametersDescription& access_parameters_description();

  /// Get generator
  MeshGenerator* generator();

private:
  /// Default parameters of generation function
  Parameters m_default_parameters;
  /// Parameters description
  ParametersDescription m_parameters_description;

  /// Name of generation function
  String m_name;
  /// Back pointer to mesh generator. Can be used for calling other functions
  WeakPtr<MeshGenerator> m_generator;
};
