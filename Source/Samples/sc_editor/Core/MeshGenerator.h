// SHW Spacecraft editor
//
// Class for generating meshes.

#pragma once

// Editor includes
#include "MeshGeometry.h"
#include "Polyline2.h"

// Urho3D includes
#include <Urho3D/Core/Object.h>

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

  // Helper functions
  /// Lathe
  MeshGeometry* lathe(
    const Polyline2& polyline, int sectors,
    TriangulationType triangulation = ttQUAD,
    float start_angle = 0.0, float end_angle = 0.0 // if start == end - build full circle
  );

  // TODO: registering functions and cache
};