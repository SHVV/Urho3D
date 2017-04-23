// SHW Spacecraft editor
//
// Class for storing parameters for functions.

#pragma once

// Editor includes

// Urho3D includes
#include <Urho3D/Core/Variant.h>

using namespace Urho3D;

class Parameters
{
public:
  /// Default constructor.
  Parameters();

  /// Construct from another parameters.
  Parameters(const Parameters& map);

  /// Assign parameters.
  Parameters& operator=(const Parameters& rhs);

  /// Destructor
  virtual ~Parameters();

  /// Test for equality with another parameters.
  bool operator==(const Parameters& rhs) const;

  /// Test for inequality with another parameters.
  bool operator !=(const Parameters& rhs) const;

  /// Test if less than another parameters.
  bool operator <(const Parameters& rhs) const;

  /// Test if greater than another parameters.
  bool operator >(const Parameters& rhs) const;

  /// Get parameters map for reading only
  const VariantVector& parameters_vector() const;

  /// Get access to parameters map for writing
  VariantVector& access_parameters_vector();

  /// Get hash of variant map
  StringHash hash() const;

  /// HashMap support
  unsigned int ToHash() const;

protected:
  /// Calculates hash of parameters map
  StringHash calculate_hash() const;

private:
  /// Parameters vector (vector of variants)
  VariantVector m_parameters_vector;
  /// Hash of parameters for faster equality test
  mutable StringHash m_hash;
  /// Flag, that hash is valid
  mutable bool m_hash_valid;
};
