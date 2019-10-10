//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_UNITCONVERTER_H_
#define MATERIALX_UNITCONVERTER_H_

/// @file UnitConverter.h
/// Unit converter classes.

#include <MaterialXCore/Library.h>

#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Types.h>

namespace MaterialX
{

class UnitConverter;

/// A shared pointer to an UnitConverter
using UnitConverterPtr = shared_ptr<UnitConverter>;
/// A shared pointer to a const UnitConverter
using ConstUnitConverterPtr = shared_ptr<const UnitConverter>;

/// @class UnitConverter
/// An unit conversion utility class.
///
/// This class can perform a linear conversion for a given UnitTypeDef.
/// The conversion of a value to the default unit is defined as multipling 
/// by a scale value and adding an offset value. 
/// Reversing these operations performs a conversion from the default unit.
///
class UnitConverter
{
  public:
    UnitConverter() {};
    virtual ~UnitConverter() { }

    /// Convert a given value in a given unit to a desired unit
    /// @param input Input value to convert
    /// @param inputUnit Unit of input value
    /// @param outputUnit Unit for output value
    virtual float convert(float input, const string& inputUnit, const string& outputUnit) const = 0;

    /// Given a unit name return a value that it can map to as an integer
    /// Returns -1 value if not found
    virtual int getUnitAsInteger(const string&) const { return -1; }

    /// Given an integer index return the unit name in the map used by the converter
    /// Returns Empty string if not found
    virtual string getUnitFromInteger(int) const { return EMPTY_STRING; }

    /// Convert a given value in a given unit to a desired unit
    /// @param input Input value to convert
    /// @param inputUnit Unit of input value
    /// @param outputUnit Unit for output value
    virtual Vector2 convert(Vector2 input, const string& inputUnit, const string& outputUnit) const = 0;

    /// Convert a given value in a given unit to a desired unit
    /// @param input Input value to convert
    /// @param inputUnit Unit of input value
    /// @param outputUnit Unit for output value
    virtual Vector3 convert(Vector3 input, const string& inputUnit, const string& outputUnit) const = 0;

    /// Convert a given value in a given unit to a desired unit
    /// @param input Input value to convert
    /// @param inputUnit Unit of input value
    /// @param outputUnit Unit for output value
    virtual Vector4 convert(Vector4 input, const string& inputUnit, const string& outputUnit) const = 0;

};

class DistanceUnitConverter;

/// A shared pointer to an DistanceUnitConverter
using DistanceUnitConverterPtr = shared_ptr<DistanceUnitConverter>;
/// A shared pointer to a const DistanceUnitConverter
using ConstDistanceUnitConverterPtr = shared_ptr<const DistanceUnitConverter>;

/// @class LDistanceUnitConverter
/// An unit conversion utility for handling length.
///
class DistanceUnitConverter : public UnitConverter
{
  public:
    virtual ~DistanceUnitConverter() { }

    /// Creator 
    static DistanceUnitConverterPtr create(UnitTypeDefPtr UnitDef);

    /// Return the name of the default unit for "distance"
    const string& getDefaultUnit() const
    {
        return _defaultUnit;
    }

    /// @name Conversion
    /// @{

    /// Return the mappings from unit names to the scale value
    /// defined by a linear converter. 
    const std::unordered_map<string, float>& getUnitScale() const
    {
        return _unitScale;
    }

    /// Ratio between the given unit to a desired unit
    /// @param inputUnit Unit of input value
    /// @param outputUnit Unit for output value
    float conversionRatio(const string& inputUnit, const string& outputUnit) const;

    /// Convert a given value in a given unit to a desired unit
    /// @param input Input value to convert
    /// @param inputUnit Unit of input value
    /// @param outputUnit Unit for output value
    float convert(float input, const string& inputUnit, const string& outputUnit) const override;

    /// Convert a given value in a given unit to a desired unit
    /// @param input Input value to convert
    /// @param inputUnit Unit of input value
    /// @param outputUnit Unit for output value
    Vector2 convert(Vector2 input, const string& inputUnit, const string& outputUnit) const override;

    /// Convert a given value in a given unit to a desired unit
    /// @param input Input value to convert
    /// @param inputUnit Unit of input value
    /// @param outputUnit Unit for output value
    Vector3 convert(Vector3 input, const string& inputUnit, const string& outputUnit) const override;

    /// Convert a given value in a given unit to a desired unit
    /// @param input Input value to convert
    /// @param inputUnit Unit of input value
    /// @param outputUnit Unit for output value
    Vector4 convert(Vector4 input, const string& inputUnit, const string& outputUnit) const override;

    /// @}
    /// @name Shader Mapping
    /// @{

    /// Given a unit name return a value that it can map to as an integer.
    /// Returns -1 value if not found
    int getUnitAsInteger(const string& unitName) const override;

    /// Given an integer index return the unit name in the map used by the converter.
    /// Returns Empty string if not found
    virtual string getUnitFromInteger(int index) const override;

    /// @}

    /// Length unit type name
    static const string DISTANCE_UNIT;

  private:
    DistanceUnitConverter(UnitTypeDefPtr UnitDef);

    std::unordered_map<string, float> _unitScale;
    std::unordered_map<string, int> _unitEnumeration;
    string _defaultUnit;
};


class UnitConverterRegistry;

/// A shared pointer to an UnitConverterRegistry
using UnitConverterRegistryPtr = shared_ptr<UnitConverterRegistry>;
/// A shared pointer to a const UnitConverterRegistry
using ConstUnitConverterRegistryPtr = shared_ptr<const UnitConverterRegistry>;

/// @class UnitConverterRegistry
/// A registry of unit converters.
///
class UnitConverterRegistry
{
  public:
    virtual ~UnitConverterRegistry() { }

    /// Creator 
    static UnitConverterRegistryPtr create();

    /// Add a unit converter for a given UnitDef.
    /// Returns false if a converter has already been registered for the given UnitDef 
    bool addUnitConverter(UnitTypeDefPtr def, UnitConverterPtr converter);

    /// Remove a unit converter for a given UnitDef.
    /// Returns false if a converter does not exist for the given UnitDef 
    bool removeUnitConverter(UnitTypeDefPtr def);

    /// Get a unit converter for a given UnitDef
    /// Returns any empty pointer if a converter does not exist for the given UnitDef 
    UnitConverterPtr getUnitConverter(UnitTypeDefPtr def);

    /// Clear all unit converters from the registry.
    void clearUnitConverters();

    /// Given a unit name return a value that it can map to as an integer
    /// Returns -1 value if not found
    int getUnitAsInteger(const string& unitName) const;

  private:
    UnitConverterRegistry() { }

    UnitConverterRegistry(const UnitConverterRegistry&) = delete;

    UnitConverterRegistry& operator=(const UnitConverterRegistry&) = delete;

    std::unordered_map<string, UnitConverterPtr> _unitConverters;
};

}  // namespace MaterialX

#endif  // MATERIALX_UNITCONVERTER_H_