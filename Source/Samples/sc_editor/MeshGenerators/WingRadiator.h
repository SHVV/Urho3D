// SHW Spacecraft editor
//
// Wing radiator generation function.

#pragma once

// Editor includes
#include "../Core/MeshGenerationFunction.h"

// Urho3D includes

// Forward declaration

using namespace Urho3D;

class WingRadiator : public MeshGenerationFunction
{
public:
  /// Default constructor
  WingRadiator();

  /// Destructor
  virtual ~WingRadiator();

  /// Generate mesh geometry, basing on parameters
  virtual MeshGeometry* generate(const Parameters& parameters) override;

  /// Update procedural unit guts
  virtual void update_unit(
    const Parameters& parameters, 
    ProceduralUnit* unit
  ) override;

  /// Wing width parameter ID
  static ParameterID s_width;

  /// Wing length parameter ID
  static ParameterID s_length;

  /// Taper parameter ID
  static ParameterID s_taper;

  // TODO: add other parameters

  /// Generator name
  static String s_name;

protected:
  /// Calculate surface mount size, based on parameters.
  float mount_size(const Parameters& parameters);

private:
};
