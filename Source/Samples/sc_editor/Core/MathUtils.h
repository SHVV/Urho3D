// SHW Spacecraft editor
//
// Mathematics and geometry utilities

#include "Urho3D\Math\Frustum.h"
#include "Urho3D\Math\BoundingBox.h"

using namespace Urho3D;

namespace MathUtils {
  /// Helper line structure
  struct Line3 {
    Vector3 p1;
    Vector3 p2;
  };

  /// Clip world bounding box to Frustum
  BoundingBox clip(const BoundingBox& bb, const Frustum& fr);

  /// Clip line by Frustum
  Line3 clip(const Line3& line, const Frustum& fr);

  /// Clip line by plane
  Line3 clip(const Line3& line, const Plane& plane);

  /// Calculate ortoronal vector
  Vector3 ortogonal(const Vector3& original, const Vector3& reference = Vector3::FORWARD);
};
