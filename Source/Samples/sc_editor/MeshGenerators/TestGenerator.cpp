// SHW Spacecraft editor
//
// Test mesh generation function.

#include "TestGenerator.h"

// Editor Includes
#include "../Core/MeshGenerator.h"
#include "../Core/Polyline2.h"
#include "../Model/BaseAttachableSurface.h"
#include "../Model/ProceduralUnit.h"

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
  float sector_length = (radius * M_PI * 2) / segments;
  int inner_segments = ceil(length / sector_length);
  float segment_length = inner_segments ? (length / inner_segments) : 0;
  
  Polyline2 profile;
  PolylineSegment seg;

  seg.m_lateral_material_id = -1;
  seg.m_longitudal_material_id = -1;
  seg.m_node_material_id = -1;

  seg.m_plate_material_id = 0;
  seg.m_node_radius = 0.4 * radius / segments;
  seg.m_smooth_vertex = false;
  seg.m_smooth_segment = false;

  seg.m_pos = Vector2(-x1, 0);
  profile.segments().Push(seg);
  seg.m_pos = Vector2(-x1, y2);
  profile.segments().Push(seg);

  //seg.m_lateral_material_id = 0;
  //seg.m_node_material_id = 0;

  seg.m_pos = Vector2(-length / 2, radius);
  seg.m_node_material_id = 0;
  profile.segments().Push(seg);
  seg.m_node_material_id = -1;

  //seg.m_longitudal_material_id = 0;
  int supports = (inner_segments >= 5) ? 2 :
    ((inner_segments == 3) ? 1 : 0);
  for (int i = 0; i < inner_segments; ++i) {
    seg.m_pos = Vector2(-length / 2 + (i + 1) * segment_length, radius);
    if (i == inner_segments - 1) {
      seg.m_node_material_id = 0;
    }

    if (supports > 0) {
      if (i == 0 || i == inner_segments - 3) {
        seg.m_node_material_id = 0;
        seg.m_lateral_material_id = 0;
      }
      if (i == 1 || i == inner_segments - 2) {
        seg.m_node_material_id = 0;
        seg.m_lateral_material_id = 0;
        seg.m_longitudal_material_id = 0;
        seg.m_plate_material_id = 1;
      }
    }

    profile.segments().Push(seg);
    seg.m_node_material_id = -1;
    seg.m_lateral_material_id = -1;
    seg.m_longitudal_material_id = -1;
    seg.m_plate_material_id = 0;
  }

  seg.m_lateral_material_id = -1;
  seg.m_longitudal_material_id = -1;
  seg.m_node_material_id = -1;

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

  MeshGeometry* geometry = generator()->lathe(
    profile, 
    parameters[s_segments].GetUInt(),
    //ttTRIANGLE
    ttDIAMOND
  );

  int start_vertex = geometry->vertices().Size();
  int start_edge = geometry->edges().Size();
  int start_polygon = geometry->polygons().Size();

  // Set mountable surfaces
  for (int i = 0; i < geometry->vertices().Size(); ++i) {
    if (geometry->vertices()[i].material == 0) {
      geometry->set_vertex_flags(i, mgfATTACHABLE | mgfVISIBLE);
    }
  }
  for (int i = 0; i < geometry->edges().Size(); ++i) {
    if (geometry->edges()[i].material == 0) {
      if (geometry->edges()[i].secondary) {
        geometry->set_edge_material(i, -1);
      } else {
        geometry->set_edge_flags(i, mgfATTACHABLE | mgfVISIBLE);
      }
    }
  }
  for (int i = 0; i < geometry->polygons().Size(); ++i) {
    if (geometry->polygons()[i].material == 1) {
      geometry->set_polygon_flags(i, mgfATTACHABLE | mgfVISIBLE);
    }
  }

  float scale = Min(x1 * 2, radius * 2) * 1.3;
  for (int i = 0; i < geometry->vertices().Size(); ++i) {
    geometry->set_scale(i, scale);
  }
  return geometry;
}

/// Update procedural unit guts
void TestGenerator::update_unit(
  const Parameters& parameters,
  ProceduralUnit* unit
)
{
  // Call default implementation first
  MeshGenerationFunction::update_unit(parameters, unit);
  // Create default attachable surface
  unit->get_component<BaseAttachableSurface>();
}
