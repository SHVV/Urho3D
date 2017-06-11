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

  /// Tank radius parameter ID
  static ParameterID s_radius;

  /// Tank length parameter ID
  static ParameterID s_length;

  /// Tank cap parameter ID
  static ParameterID s_head;

  /// Tank conical parameter ID
  static ParameterID s_conical;

  /// Tank segments parameter ID
  static ParameterID s_segments;

  /// Generator name
  static String s_name;

protected:

private:
};
