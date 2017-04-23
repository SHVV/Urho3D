// SHW Spacecraft editor
//
// Test mesh generation function.

#include "TestGenerator.h"

// Editor Includes
#include "../Core/MeshGenerator.h"
#include "../Core/Polyline2.h"

/// Default constructor
TestGenerator::TestGenerator()
: MeshGenerationFunction(s_name())
{
  // Set parameters
  // TODO better interface
  Parameters default_parameters;
  auto& parameters_vector = default_parameters.access_parameters_vector();
  parameters_vector.Push(Variant(5));

  set_default_parameters(default_parameters);
}

/// Destructor
TestGenerator::~TestGenerator()
{
}

String TestGenerator::s_name()
{
  return "Test";
}

/// Generate mesh geometry, basing on parameters
MeshGeometry* TestGenerator::generate(const Parameters& parameters)
{
  Polyline2 profile;
  PolylineSegment seg;
  seg.m_lateral_material_id = 0;
  seg.m_longitudal_material_id = 0;
  seg.m_node_material_id = 0;
  seg.m_plate_material_id = 0;
  seg.m_node_radius = 0.3;
  seg.m_smooth_vertex = true;
  seg.m_smooth_segment = true;

  seg.m_pos = Vector2(-10, 0);
  profile.segments().Push(seg);
  seg.m_pos = Vector2(-5, 10);
  profile.segments().Push(seg);
  seg.m_pos = Vector2(5, 10);
  profile.segments().Push(seg);
  seg.m_pos = Vector2(10, 0);
  profile.segments().Push(seg);


  return generator()->lathe(
    profile, 
    parameters.parameters_vector()[0].GetUInt(), // TODO: better interface
    ttTRIANGLE
  );
}
