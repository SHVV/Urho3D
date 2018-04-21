// SHW Spacecraft editor
//
// Beam to surface mount adapter.

#include "BeamAdapter.h"

// Editor Includes
#include "../Core/MeshGenerator.h"
#include "../Core/Polyline2.h"

#include <Urho3D/Math/Quaternion.h>

/// Generator name
String BeamAdapter::s_name = "Beam adapter";

/// Beam radius parameter ID
ParameterID BeamAdapter::s_beam_radius = 0;
/// Plate size parameter ID
ParameterID BeamAdapter::s_plate_size = 1;
/// Beam to mount node shift parameter ID
ParameterID BeamAdapter::s_shift = 2;

/// Default constructor
BeamAdapter::BeamAdapter()
  : MeshGenerationFunction(s_name)
{
  // Set parameters
  add_parameter(s_beam_radius, 0.1, pfLINEAR_VALUE, "Beam radius", "Beam's radius in meters", 0.0, 1000.0);
  add_parameter(s_plate_size, 1.0, pfLINEAR_VALUE, "Plate size", "Mount plate size in meters", 0.0, 1000.0);
  add_parameter(s_shift, 0.2, pfLINEAR_VALUE, "Mount shift", "Shift between beam and mount node", 0.0, 1000.0);
}

/// Destructor
BeamAdapter::~BeamAdapter()
{
}

/// Generate mesh geometry, basing on parameters
MeshGeometry* BeamAdapter::generate(const Parameters& parameters)
{
  // Parameters
  float beam_radius = parameters[s_beam_radius].GetFloat();
  float plate_size = parameters[s_plate_size].GetFloat();
  float shift = parameters[s_shift].GetFloat();

  // TODO: add rest parameters

  float x = plate_size;
  float y = beam_radius * 1.2;
  float fillet = beam_radius * 0.1;
  
  Polyline2 profile;
  PolylineSegment seg;

  seg.m_lateral_material_id = -1;
  seg.m_longitudal_material_id = -1;
  seg.m_node_material_id = -1;

  seg.m_plate_material_id = 0;
  seg.m_smooth_vertex = false;
  seg.m_smooth_segment = false;

  MeshGeometry* geometry = nullptr;
  // First - beam support
  if (beam_radius > 0) {
    seg.m_pos = Vector2(-x, 0);
    profile.segments().Push(seg);
    seg.m_pos = Vector2(-x, y - fillet);
    profile.segments().Push(seg);
    seg.m_pos = Vector2(-x + fillet, y);
    profile.segments().Push(seg);
    seg.m_pos = Vector2(x - fillet, y);
    profile.segments().Push(seg);
    seg.m_pos = Vector2(x, y - fillet);
    profile.segments().Push(seg);
    seg.m_pos = Vector2(x, 0);
    profile.segments().Push(seg);

    geometry = generator()->lathe(profile, 8, ttDIAMOND);
    Quaternion rotation(90, Vector3::UP);
    Matrix3x4 transform(Vector3(0, 0, -shift), rotation, 1);
    geometry->transform(transform);
  }

  // Second - plate
  float start_y = plate_size - shift * 0.75;
  if (start_y < 0) start_y = 0;

  float start_x = -shift * 0.25;
  float end_x = -shift * 0.10;
  float end_y = plate_size - shift * 0.10;

  profile.segments().Clear();
  seg.m_pos = Vector2(-shift, 0);
  profile.segments().Push(seg);
  if (start_y > 0) {
    seg.m_pos = Vector2(-shift, start_y);
    profile.segments().Push(seg);
  }
  seg.m_pos = Vector2(start_x, plate_size);
  profile.segments().Push(seg);
  seg.m_pos = Vector2(end_x, plate_size);
  profile.segments().Push(seg);
  seg.m_pos = Vector2(0, end_y);
  profile.segments().Push(seg);
  seg.m_pos = Vector2(0, 0);
  profile.segments().Push(seg);
  if (geometry) {
    generator()->lathe(geometry, profile, 8, ttDIAMOND);
  } else {
    geometry = generator()->lathe(profile, 8, ttDIAMOND);
  }

  float scale = Max(plate_size, beam_radius) * 2;
  for (int i = 0; i < geometry->vertices().Size(); ++i) {
    geometry->set_scale(i, scale);
  }
  return geometry;
}
