// SHW Spacecraft editor
//
// Class for storing parameters for functions.

// Editor includes
#include "Parameters.h"

/// null variant
Variant Parameters::s_null;

/// Default constructor.
Parameters::Parameters()
: m_hash_valid(false)
{
}

/// Construct from another parameters.
Parameters::Parameters(const Parameters& map)
{
  *this = map;
}

/// Assign parameters.
Parameters& Parameters::operator=(const Parameters& rhs)
{
  m_parameters_vector = rhs.m_parameters_vector;
  m_hash = rhs.m_hash;
  m_hash_valid = rhs.m_hash_valid;

  return *this;
}

/// Destructor
Parameters::~Parameters()
{
}

/// Test for equality with another parameters.
bool Parameters::operator==(const Parameters& rhs) const
{
  return hash() == rhs.hash();
}

/// Test for inequality with another parameters.
bool Parameters::operator!=(const Parameters& rhs) const
{
  return hash() != rhs.hash();
}

/// Test if less than another parameters.
bool Parameters::operator <(const Parameters& rhs) const
{
  return hash() < rhs.hash();
}

/// Test if greater than another parameters.
bool Parameters::operator >(const Parameters& rhs) const
{
  return hash() > rhs.hash();
}

/// Parameter for writing access
Variant& Parameters::operator [](const ParameterID& id)
{
  m_hash_valid = false;
  if (id >= m_parameters_vector.Size()) {
    m_parameters_vector.Resize(id + 1);
  }
  return m_parameters_vector[id];
}

/// Read parameter. Returns null variant, if not found
const Variant& Parameters::operator [](const ParameterID& id) const
{
  if (id < m_parameters_vector.Size()) {
    return m_parameters_vector[id];
  } else {
    return s_null;
  }
}

/// Get parameters map for reading only
const VariantVector& Parameters::parameters_vector() const
{
  return m_parameters_vector;
}

/// Get access to parameters map for writing
VariantVector& Parameters::access_parameters_vector()
{
  m_hash_valid = false;
  return m_parameters_vector;
}

/// Get hash of variant map
StringHash Parameters::hash() const
{
  if (!m_hash_valid) {
    m_hash = calculate_hash();
    m_hash_valid = true;
  }
  return m_hash;
}

/// HashMap support
unsigned int Parameters::ToHash() const
{
  return hash();
}

/// Calculates hash of parameters map
StringHash Parameters::calculate_hash() const
{
  String all_parameters;
  for (auto i = m_parameters_vector.Begin(); i != m_parameters_vector.End(); ++i) {
    all_parameters += i->ToString() + "|";
    // TODO: add variant vector and map support
  }
  return StringHash(all_parameters);
}
