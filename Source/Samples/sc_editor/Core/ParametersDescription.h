// SHW Spacecraft editor
//
// Class for storing description of parameters for functions.

#pragma once

// Editor includes

// Urho3D includes
#include <Urho3D/Core/Variant.h>

using namespace Urho3D;

typedef unsigned int ParameterID;

enum ParameterFlag {
  pfNONE = 0,
  pfINVALID = 1,        // Invalid non-initialized parameter
  pfTEMPORARY = 2,      // Temporary parameter, that should not be serialized
  pfNOT_EDITABLE = 4,   // Parameter, that cannot be editied manually
  pfINTERACTIVE = 8,    // Parameter, that can be entered interactively during creation process
  pfLINEAR_VALUE = 16,  // Linear value in "m"
  pfANGULAR_VALUE = 32, // Angular value in degrees
  pfCELL_SIZE = 64,     // Parameter determine cell size
  pfCELLS = 128         // Parameter is a number of cells
};

struct ParameterDescription
{
  int m_flags;
  String m_name;
  String m_tooltip;
  Variant m_min;
  Variant m_max;
  // TODO: other settings like increment step, units, names for enumerations, and so on...

  bool valid() {
    return !(m_flags & pfINVALID);
  }
};

class ParametersDescription
{
public:
  /// Default constructor.
  ParametersDescription();

  /// Deleted copy construcot.
  ParametersDescription(const ParametersDescription& map) = delete;

  /// Deleted assignment operator.
  ParametersDescription& operator=(const ParametersDescription& rhs) = delete;

  /// Destructor
  virtual ~ParametersDescription();

  /// Parameter description for writing access
  ParameterDescription& operator [](const ParameterID& id);

  /// Read parameter description. Returns invalid, if not found
  const ParameterDescription& operator [](const ParameterID& id) const;

  /// Parameter IDs list
  Vector<ParameterID>& parameter_ids();

  /// Parameter IDs list
  const Vector<ParameterID>& parameter_ids() const;
protected:
private:
  /// ParametersDescription vector
  Vector<ParameterDescription> m_descriptions_vector;

  /// Parameter IDs list
  Vector<ParameterID> m_parameter_ids;

  /// invalid description for "not found"
  static ParameterDescription s_null;
};
