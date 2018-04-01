// SHW Spacecraft editor
//
// Wing radiator generation function.

#include "WingRadiator.h"

// Editor Includes
#include "../Core/MeshGenerator.h"
#include "../Core/Polyline2.h"

/// Generator name
String WingRadiator::s_name = "WingRadiator";

/// Cell size parameter ID
ParameterID WingRadiator::s_width = 0;

/// Length in cell parameter ID
ParameterID WingRadiator::s_length = 1;

/// Truss segments parameter ID
ParameterID WingRadiator::s_taper = 2;

/// Default constructor
WingRadiator::WingRadiator()
  : MeshGenerationFunction(s_name)
{
  // Set parameters
  add_parameter(s_width, 5.0, pfLINEAR_VALUE | pfINTERACTIVE, "Width", "Radiator's width in meters", 0.1, 1000.0);
  add_parameter(s_length, 20.0, pfLINEAR_VALUE | pfINTERACTIVE, "Length", "Radiator's length in meters", 0.1, 1000.0);
  add_parameter(s_taper, 0.3, pfINTERACTIVE, "Taper", "Relative radiator's taper", 0.1, 1.0);
}

/// Destructor
WingRadiator::~WingRadiator()
{
}

/// Generate mesh geometry, basing on parameters
MeshGeometry* WingRadiator::generate(const Parameters& parameters)
{
  // Parameters
  float width = parameters[s_width].GetFloat();
  float length = parameters[s_length].GetFloat();
  float taper = parameters[s_taper].GetFloat();
  // TODO: add rest parameters

  float radius = width / 20;
  
  Polyline2 profile;
  PolylineSegment seg;

  seg.m_lateral_material_id = -1;
  seg.m_longitudal_material_id = -1;
  seg.m_node_material_id = -1;
  seg.m_plate_material_id = 0;

  seg.m_smooth_vertex = false;
  seg.m_smooth_segment = false;

  seg.m_pos = Vector2(0, radius);
  profile.segments().Push(seg);
  seg.m_pos = Vector2(length - radius, radius);
  profile.segments().Push(seg);
  seg.m_pos = Vector2(length, 0);
  profile.segments().Push(seg);

  MeshGeometry* geometry = generator()->lathe(
    profile, 
    8,
    //ttTRIANGLE
    ttDIAMOND
  );
  float thickness = radius / 5;
  float tapered = width * taper;

  int v1_1 = geometry->add(Vector3(0, thickness, radius), Vector3::UP);
  int v1_2 = geometry->add(Vector3(width, thickness, radius), Vector3::UP);
  int v1_3 = geometry->add(Vector3(tapered, thickness, length - radius), Vector3::UP);
  int v1_4 = geometry->add(Vector3(0, thickness, length - radius), Vector3::UP);

  int v2_1 = geometry->add(Vector3(0, -thickness, radius), Vector3::UP);
  int v2_2 = geometry->add(Vector3(width, -thickness, radius), Vector3::UP);
  int v2_3 = geometry->add(Vector3(tapered, -thickness, length - radius), Vector3::UP);
  int v2_4 = geometry->add(Vector3(0, -thickness, length - radius), Vector3::UP);

  int f1 = geometry->add(v2_1, v2_2, v2_3, v2_4);
  int f2 = geometry->add(v1_4, v1_3, v1_2, v1_1);

  float scale = Min(width * 2, length) * 1.3;
  for (int i = 0; i < geometry->vertices().Size(); ++i) {
    geometry->set_scale(i, scale);
  }

  return geometry;
}
