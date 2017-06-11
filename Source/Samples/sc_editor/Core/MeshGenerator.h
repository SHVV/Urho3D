// SHW Spacecraft editor
//
// Class for generating meshes.

#pragma once

// Editor includes
#include "MeshGeometry.h"
#include "MeshBuffer.h"
#include "Polyline2.h"
#include "Parameters.h"
#include "ParametersDescription.h"

// Urho3D includes
#include <Urho3D/Core/Object.h>

class MeshGenerationFunction;

using namespace Urho3D;

enum TriangulationType {
  ttQUAD,
  ttTRIANGLE,
  ttSECTOR,
  ttSEGMENT,
  ttDIAMOND,
};

class MeshGenerator : public Object
{
  URHO3D_OBJECT(MeshGenerator, Object);

public:
  /// Default constructor.
  MeshGenerator(Context* context);

  /// Destructor
  virtual ~MeshGenerator();

  /// Returns default parameters of function
  const Parameters& default_parameters(StringHash name) const;

  /// Get all parameters description of function
  const ParametersDescription& parameters_description(StringHash name) const;

  /// Generate mesh and return buffer for visualizing it.
  // TODO: make it in future async an generate in background thread
  MeshBuffer* generate_buffer(StringHash name, const Parameters& parameters);

  /// Generate and return mesh only
  MeshGeometry* generate_mesh(StringHash name, const Parameters& parameters);

  // TODO: add memory consumption tracking and clearing unused cache

  /// Register mesh generation function
  void add_function(MeshGenerationFunction* function);

  // Helper functions
  /// Lathe
  MeshGeometry* lathe(
    const Polyline2& polyline, int sectors,
    TriangulationType triangulation = ttQUAD,
    float start_angle = 0.0, float end_angle = 0.0 // if start == end - build full circle
  );

private:
  /// All registered functions
  HashMap<StringHash, SharedPtr<MeshGenerationFunction>> m_functions;
  /// Pair of function name + parameters -> function call
  typedef Pair<StringHash, Parameters> FunctionCall;
  /// Cache of generated meshes
  HashMap<FunctionCall, SharedPtr<MeshGeometry>> m_generated_meshes;
  /// Cache of generated mesh buffers
  HashMap<FunctionCall, SharedPtr<MeshBuffer>> m_generated_buffers;
};