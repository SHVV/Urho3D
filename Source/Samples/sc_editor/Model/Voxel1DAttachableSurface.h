// SHW Spacecraft editor
//
// 1 dimension cell based attachable surface (base trusses).
// Used to preserve attachment coordinates during topology changes.

#pragma once

#include "BaseAttachableSurface.h"

using namespace Urho3D;


class Voxel1DAttachableSurface : public BaseAttachableSurface {
  // Enable type information.
  URHO3D_OBJECT(Voxel1DAttachableSurface, BaseAttachableSurface);
public:

  /// Construct.
  Voxel1DAttachableSurface(Context* context);

  /// Destructor
  virtual ~Voxel1DAttachableSurface() override = default;

  /// Register object attributes.
  static void RegisterObject(Context* context);

  /// Initialize parameters
  void initialize(float cell_size, float shift);

protected:
  /// Convert local position to topology independent one
  virtual Vector3 local_to_topology(
    const Vector3& position,
    const Vector3& normal
  ) override;

  /// Convert position from topology independent to local
  virtual Vector3 topology_to_local(
    const Vector3& norm_position,
    const Vector3& normal
  ) override;

private:
  /// Voxel cell size
  float m_cell_size;
  /// Shift of 0 voxel position
  float m_shift;
};
