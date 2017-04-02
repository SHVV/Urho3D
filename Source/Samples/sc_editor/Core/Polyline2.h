// SHW Spacecraft editor
//
// Class for representing 2 dimensional polyline

#pragma once

#include "Urho3D/Container/RefCounted.h"
#include "Urho3D/Container/Vector.h"
#include "Urho3D/Math/Vector2.h"
using namespace Urho3D;

//  TODO: may be it is better to split vertex and segments
struct PolylineSegment
{
  PolylineSegment();

  Vector2 m_pos;
  float m_node_radius;
  int m_node_material_id;
  int m_lateral_material_id;
  int m_longitudal_material_id;
  int m_plate_material_id;
  bool m_smooth_vertex;
  bool m_smooth_segment;
};

class Polyline2 : public RefCounted
{
public:
  /// Default constructor.
  Polyline2();

  /// Destructor
  virtual ~Polyline2();

  /// Vector of segments
  Vector<PolylineSegment>& segments();
  const Vector<PolylineSegment>& segments() const;

private:
  Vector<PolylineSegment> m_segments;
};