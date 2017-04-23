// SHW Spacecraft editor
//
// Class for mesh generation function.
// Gets Parameters as input and returns MeshGeometry.

#pragma once

// Editor includes
#include "Parameters.h"

// Urho3D includes
#include <Urho3D/Container/RefCounted.h>

// Forward declaration
class MeshGeometry; 
class MeshGenerator;

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

  // TODO: Parameters description

  /// Generate mesh geometry, basing on parameters
  virtual MeshGeometry* generate(const Parameters& parameters) = 0;

protected:
  /// Constructor with parameters.
  MeshGenerationFunction(
    String name
  );

  /// Set default parameters
  void set_default_parameters(const Parameters& default_parameters);

  /// Get generator
  MeshGenerator* generator();

private:
  /// Default parameters of generation function
  Parameters m_default_parameters;
  /// Name of generation function
  String m_name;
  /// Back pointer to mesh generator. Can be used for calling other functions
  WeakPtr<MeshGenerator> m_generator;
};
