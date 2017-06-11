// SHW Spacecraft editor
//
// Class for storing description of parameters for functions.

// Editor includes
#include "ParametersDescription.h"

/// null variant
ParameterDescription ParametersDescription::s_null;

/// Default constructor.
ParametersDescription::ParametersDescription()
{
}

/// Destructor
ParametersDescription::~ParametersDescription()
{
}

/// Parameter for writing access
ParameterDescription& ParametersDescription::operator [](const ParameterID& id)
{
  if (id >= m_descriptions_vector.Size()) {
    m_descriptions_vector.Resize(id + 1);
  }
  return m_descriptions_vector[id];
}

/// Read parameter. Returns null variant, if not found
const ParameterDescription& ParametersDescription::operator [](const ParameterID& id) const
{
  if (id < m_descriptions_vector.Size()) {
    return m_descriptions_vector[id];
  } else {
    return s_null;
  }
}

/// Parameter IDs list
Vector<ParameterID>& ParametersDescription::parameter_ids()
{
  return m_parameter_ids;
}

/// Parameter IDs list
const Vector<ParameterID>& ParametersDescription::parameter_ids() const
{
  return m_parameter_ids;
}
