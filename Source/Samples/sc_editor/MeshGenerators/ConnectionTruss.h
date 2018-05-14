// SHW Spacecraft editor
//
// Connection truss generation function.

#pragma once

// Editor includes
#include "../Core/MeshGenerationFunction.h"

// Urho3D includes

// Forward declaration

using namespace Urho3D;

class ConnectionTruss : public MeshGenerationFunction
{
public:
  /// Default constructor
  ConnectionTruss();

  /// Destructor
  virtual ~ConnectionTruss();

  /// Generate mesh geometry, basing on parameters
  virtual MeshGeometry* generate(const Parameters& parameters);

  /// Update procedural unit guts
  virtual void update_unit(
    const Parameters& parameters, 
    ProceduralUnit* unit
  ) override;

  /// Nodes list parameter ID
  static ParameterID s_node_ids;

  /// Length in meters parameter ID
  static ParameterID s_length;

  /// Size of connection nodes parameter ID
  static ParameterID s_cell_size;

  /// Truss segments parameter ID
  static ParameterID s_segments;

  /// Truss curvature parameter ID
  static ParameterID s_curvature;

  // TODO: add other parameters

  /// Generator name
  static String s_name;

protected:

private:
};
