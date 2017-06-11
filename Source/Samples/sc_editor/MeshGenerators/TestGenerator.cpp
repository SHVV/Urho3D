// SHW Spacecraft editor
//
// Test mesh generation function.

#include "TestGenerator.h"

// Editor Includes
#include "../Core/MeshGenerator.h"
#include "../Core/Polyline2.h"

/// Generator name
String TestGenerator::s_name = "Test";

/// Tank radius parameter ID
ParameterID TestGenerator::s_radius = 0;
/// Tank length parameter ID
ParameterID TestGenerator::s_length = 1;
/// Tank curvature parameter ID
ParameterID TestGenerator::s_head = 2;
/// Tank fillet parameter ID
ParameterID TestGenerator::s_conical = 3;
/// Tank segments parameter ID
ParameterID TestGenerator::s_segments = 4;

/// Default constructor
TestGenerator::TestGenerator()
  : MeshGenerationFunction(s_name)
{
  // Set parameters
  add_parameter(s_radius, 10.0, pfLINEAR_VALUE | pfINTERACTIVE, "Radius", "Main tank's radius in meters", 0.1, 1000.0);
  add_parameter(s_length, 20.0, pfLINEAR_VALUE | pfINTERACTIVE, "Length", "Tank's linear segment length in meters", 0.1, 1000.0);
  add_parameter(s_head, 0.2, pfINTERACTIVE, "Head", "Relative tank's head height", 0.05, 1.0);
  //add_parameter(s_fillet, 0.0, pfINTERACTIVE, "Fillet", "Relative fillet value", 0.0, 1.0);
  add_parameter(s_segments, 24, pfNONE, "Segments", "Number of circular segments", 4, 64);
}

/// Destructor
TestGenerator::~TestGenerator()
{
}

/// Generate mesh geometry, basing on parameters
MeshGeometry* TestGenerator::generate(const Parameters& parameters)
{
  // Parameters
  float radius = parameters[s_radius].GetFloat();
  float length = parameters[s_length].GetFloat();
  float head_height = parameters[s_head].GetFloat();
  int segments = parameters[s_segments].GetUInt();
  // TODO: add rest parameters

  float x1 = radius * head_height + length / 2;
  //float y2 = radius / sqrt(2.0);
  float y2 = radius * 0.1;
  //float x2 = y2 + length / 2;
  int inner_segments = ceil(segments * length / (radius * M_PI * 2));
  float segment_length = inner_segments ? (length / inner_segments) : 0;
  
  Polyline2 profile;
  PolylineSegment seg;
  seg.m_lateral_material_id = -1;
  seg.m_longitudal_material_id = -1;
  seg.m_node_material_id = -1;
  seg.m_plate_material_id = 0;
  seg.m_node_radius = 0.2 * radius / segments;
  seg.m_smooth_vertex = false;
  seg.m_smooth_segment = false;

  seg.m_pos = Vector2(-x1, 0);
  profile.segments().Push(seg);
  seg.m_pos = Vector2(-x1, y2);
  profile.segments().Push(seg);
  seg.m_pos = Vector2(-length / 2, radius);
  profile.segments().Push(seg);

  for (int i = 0; i < inner_segments; ++i) {
    seg.m_pos = Vector2(-length / 2 + (i + 1) * segment_length, radius);
    profile.segments().Push(seg);
  }

  seg.m_pos = Vector2(x1, y2);
  profile.segments().Push(seg);
  seg.m_pos = Vector2(x1, 0);
  profile.segments().Push(seg);

  /*seg.m_pos = Vector2(-10, 0);
  profile.segments().Push(seg);
  seg.m_pos = Vector2(-5, 10);
  profile.segments().Push(seg);
  seg.m_pos = Vector2(5, 10);
  profile.segments().Push(seg);
  seg.m_pos = Vector2(10, 0);
  profile.segments().Push(seg);*/

  return generator()->lathe(
    profile, 
    parameters[s_segments].GetUInt(),
    ttTRIANGLE
  );
}
