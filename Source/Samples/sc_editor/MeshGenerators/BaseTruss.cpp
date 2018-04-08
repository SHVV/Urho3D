// SHW Spacecraft editor
//
// Test mesh generation function.

#include "BaseTruss.h"

// Editor Includes
#include "../Core/MeshGenerator.h"
#include "../Core/Polyline2.h"

/// Generator name
String BaseTruss::s_name = "BaseTruss";

/// Cell size parameter ID
ParameterID BaseTruss::s_cell_size = 0;

/// Length in cell parameter ID
ParameterID BaseTruss::s_cells = 1;

/// Truss segments parameter ID
ParameterID BaseTruss::s_segments = 2;

/// Default constructor
BaseTruss::BaseTruss()
  : MeshGenerationFunction(s_name)
{
  // Set parameters
  add_parameter(s_cell_size, 5.0, pfLINEAR_VALUE | pfINTERACTIVE | pfCELL_SIZE, "Cell size", "Truss cell size in meters", 0.1, 1000.0);
  add_parameter(s_cells, 1, pfLINEAR_VALUE | pfINTERACTIVE | pfCELLS, "Cells", "Truss length in cells count", 1, 1000.0);
  add_parameter(s_segments, 6, pfINTERACTIVE, "Sides", "Number of truss sides", 3, 12);
}

/// Destructor
BaseTruss::~BaseTruss()
{
}

/// Generate mesh geometry, basing on parameters
MeshGeometry* BaseTruss::generate(const Parameters& parameters)
{
  // Parameters
  float cell_size = parameters[s_cell_size].GetFloat();
  float cells = parameters[s_cells].GetFloat();
  int segments = parameters[s_segments].GetUInt();
  // TODO: add rest parameters

  float radius = cell_size / sin(M_PI / segments) / 2;
  
  Polyline2 profile;
  PolylineSegment seg;

  seg.m_lateral_material_id = 0;
  seg.m_longitudal_material_id = 0;
  seg.m_node_material_id = 0;
  seg.m_plate_material_id = -1;

  seg.m_node_radius = cell_size / 25;
  seg.m_smooth_vertex = false;
  seg.m_smooth_segment = false;

  seg.m_pos = Vector2(-cell_size * cells * 0.5, 0);
  profile.segments().Push(seg);

  for (int i = 0; i < cells + 1; ++i) {
    seg.m_pos = Vector2((-cells * 0.5 + i) * cell_size, radius);
    profile.segments().Push(seg);
  }

  seg.m_pos = Vector2(cell_size * cells * 0.5, 0);
  profile.segments().Push(seg);

  MeshGeometry* geometry = generator()->lathe(
    profile, 
    segments,
    //ttTRIANGLE
    ttDIAMOND,
    M_PI / segments,
    M_PI / segments
  );
  float scale = Min(cell_size * cells, radius * 2) * 1.3;
  for (int i = 0; i < geometry->vertices().Size(); ++i) {
    geometry->set_scale(i, scale);
    geometry->set_vertex_flags(i, mgfATTACHABLE | mgfVISIBLE);
  }
  for (int i = 0; i < geometry->edges().Size(); ++i) {
    if (geometry->edges()[i].secondary) {
      geometry->set_edge_flags(i, mgfVISIBLE);
    } else {
      geometry->set_edge_flags(i, mgfATTACHABLE | mgfVISIBLE);
    }
  }
  for (int i = 0; i < geometry->polygons().Size(); ++i) {
    geometry->set_polygon_flags(i, mgfATTACHABLE);
  }
  return geometry;
}
