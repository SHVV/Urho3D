// SHW Spacecraft editor
//
// Test mesh generation function.

#pragma once

// Editor includes
#include "../Core/MeshGenerationFunction.h"

// Urho3D includes

// Forward declaration

using namespace Urho3D;

class TestGenerator : public MeshGenerationFunction
{
public:
  /// Default constructor
  TestGenerator();

  /// Destructor
  virtual ~TestGenerator();

  /// Generate mesh geometry, basing on parameters
  virtual MeshGeometry* generate(const Parameters& parameters);

  static String s_name();

protected:

private:
};
