// SHW Spacecraft editor
//
// Beam to surface mount adapter.

#pragma once

// Editor includes
#include "../Core/MeshGenerationFunction.h"

// Urho3D includes

// Forward declaration

using namespace Urho3D;

class BeamAdapter : public MeshGenerationFunction
{
public:
  /// Default constructor
  BeamAdapter();

  /// Destructor
  virtual ~BeamAdapter();

  /// Generate mesh geometry, basing on parameters
  virtual MeshGeometry* generate(const Parameters& parameters);

  /// Beam radius parameter ID
  static ParameterID s_beam_radius;

  /// Plate size parameter ID
  static ParameterID s_plate_size;

  /// Beam to mount node shift parameter ID
  static ParameterID s_shift;

  /// Generator name
  static String s_name;

protected:

private:
};
