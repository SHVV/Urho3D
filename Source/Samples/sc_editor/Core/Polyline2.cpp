// SHW Spacecraft editor
//
// Class for representing 2 dimensional polyline

#include "Polyline2.h"

/// Default constructor.
PolylineSegment::PolylineSegment()
: m_node_material_id(-1),
  m_lateral_material_id(-1),
  m_longitudal_material_id(-1),
  m_node_radius(0.0),
  m_plate_material_id(0),
  m_smooth_vertex(true)
{
}

/// Default constructor.
Polyline2::Polyline2()
: RefCounted()
{
}

/// Destructor
Polyline2::~Polyline2()
{
}

/// Vector of segments
Vector<PolylineSegment>& Polyline2::segments()
{
  return m_segments;
}

const Vector<PolylineSegment>& Polyline2::segments() const
{
  return m_segments;
}
