// SHW Spacecraft editor
//
// Mathematics and geometry utilities

#include "MathUtils.h"

namespace MathUtils {

/// Clip world bounding box to Frustum
BoundingBox clip(const BoundingBox& bb, const Frustum& fr)
{
  // Create array with diagonal lines of BB
  Line3 lines[] = {
    { { bb.min_.x_, bb.min_.y_, bb.min_.z_ }, { bb.max_.x_, bb.max_.y_, bb.max_.z_ } },
    { { bb.min_.x_, bb.min_.y_, bb.max_.z_ }, { bb.max_.x_, bb.max_.y_, bb.min_.z_ } },
    { { bb.min_.x_, bb.max_.y_, bb.min_.z_ }, { bb.max_.x_, bb.min_.y_, bb.max_.z_ } },
    { { bb.max_.x_, bb.min_.y_, bb.min_.z_ }, { bb.min_.x_, bb.max_.y_, bb.max_.z_ } }
  };

  // Clip them by frustum
  for (Line3& line : lines) {
    line = clip(line, fr);
  }

  // Collect points from trimmed lines
  BoundingBox res;
  for (Line3& line : lines) {
    if (line.p1.x_ != M_INFINITY) {
      res.Merge(line.p1);
      res.Merge(line.p2);
    }
  }
  return res;
}

/// Clip line by Frustum
Line3 clip(const Line3& line, const Frustum& fr)
{
  Line3 res = line;
  for (unsigned i = 0; i < NUM_FRUSTUM_PLANES; ++i) {
    res = clip(res, fr.planes_[i]);
  }
  return res;
}

/// Clip line by plane
Line3 clip(const Line3& line, const Plane& plane)
{
  if (line.p1.x_ == M_INFINITY) {
    return line;
  }

  float d1 = plane.Distance(line.p1);
  float d2 = plane.Distance(line.p2);

  if (d1 > 0) {
    if (d2 > 0) {
      // No clip
      return line;
    }
  } else if (d2 <=0) {
    // Completely out of plane
    return {
      { M_INFINITY, M_INFINITY, M_INFINITY },
      { M_INFINITY, M_INFINITY, M_INFINITY }
    };
  }

  // Calculate clip point
  float abs_d1 = abs(d1);
  float abs_d2 = abs(d2);
  float full_d = abs_d1 + abs_d2;
  if (full_d < M_EPSILON) {
    return{
      { M_INFINITY, M_INFINITY, M_INFINITY },
      { M_INFINITY, M_INFINITY, M_INFINITY }
    };
  }

  Vector3 clip_p = (line.p1 * abs_d2 + line.p2 * abs_d1) * (1 / full_d);
  if (d1 < 0) {
    return{ clip_p, line.p2 };
  } else {
    return{ line.p1, clip_p };
  }
}

}
