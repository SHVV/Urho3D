// SHW Spacecraft editor
//
// Base truss generation function.

#pragma once

// Editor includes
#include "../Core/MeshGenerationFunction.h"

// Urho3D includes

// Forward declaration

using namespace Urho3D;

class BaseTruss : public MeshGenerationFunction
{
public:
  /// Default constructor
  BaseTruss();

  /// Destructor
  virtual ~BaseTruss();

  /// Generate mesh geometry, basing on parameters
  virtual MeshGeometry* generate(const Parameters& parameters);

  /// Cell size parameter ID
  static ParameterID s_cell_size;

  /// Length in cell parameter ID
  static ParameterID s_cells;

  /// Truss segments parameter ID
  static ParameterID s_segments;

  // TODO: add other parameters

  /// Generator name
  static String s_name;

protected:

private:
};
