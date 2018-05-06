// SHW Spacecraft editor
//
// Basic procedural unit. Contains link to function for generating guts.

#pragma once

#include "../Model/UnitModel.h"

using namespace Urho3D;

class ProceduralUnit : public UnitModel {
  // Enable type information.
  URHO3D_OBJECT(ProceduralUnit, UnitModel);
public:

  /// Construct.
  ProceduralUnit(Context* context);
  
  /// Destructor
  ~ProceduralUnit();

  /// Register object attributes.
  static void RegisterObject(Context* context);

  /// Get all parameters description of the component
  virtual const ParametersDescription& parameters_description() const override;

  /// Set function name
  void set_function_name(StringHash name);
  /// Set function name
  StringHash function_name();

  /// Handle attribute write access.
  //virtual void OnSetAttribute(const AttributeInfo& attr, const Variant& src);
  /// Apply attribute changes that can not be applied immediately. Called after scene load or a network update.
  //virtual void ApplyAttributes() override;
  // TODO: add other component methods, that we need to override

  // TODO: ....

protected:
  // New interface functions
  /// Called on setting parameters
  //virtual void apply_parameters(int index = -1);
  /// Create or update all necessary components - override for derived classes
  virtual void update_guts_int() override;

private:
  StringHash m_function;
};
