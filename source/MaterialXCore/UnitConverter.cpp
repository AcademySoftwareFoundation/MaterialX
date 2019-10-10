//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <algorithm>
#include <MaterialXCore/Util.h>
#include <MaterialXCore/Value.h>
#include <MaterialXCore/UnitConverter.h>

namespace MaterialX
{
const string DistanceUnitConverter::DISTANCE_UNIT = "distance";

DistanceUnitConverter::DistanceUnitConverter(UnitTypeDefPtr unitTypeDef) :
    UnitConverter()
{
    static const string SCALE_ATTRIBUTE = "scale";
    unsigned int enumerant = 0;

    // Populate the unit scale and offset maps for each UnitDef. 
    vector<UnitDefPtr> unitDefs = unitTypeDef->getUnitDefs();
    for (UnitDefPtr unitdef : unitDefs)
    {
        for (UnitPtr unit : unitdef->getUnits())
        {
            const string& name = unit->getName();
            if (!name.empty())
            {
                const string& scaleString = unit->getAttribute(SCALE_ATTRIBUTE);
                if (!scaleString.empty())
                {
                    ValuePtr scaleValue = Value::createValueFromStrings(scaleString, getTypeString<float>());
                    _unitScale[name] = scaleValue->asA<float>();
                }
                else
                {
                    _unitScale[name] = 1.0f;
                }
                _unitEnumeration[name] = enumerant++;
            }
        }
    }

    // In case the default unit was not specified in the UnitDef explicit
    // add this to be able to accept converstion with the default 
    // as the input or output unit
    _defaultUnit = unitTypeDef->getDefault();
    auto it = _unitScale.find(_defaultUnit);
    if (it == _unitScale.end())
    {
        _unitScale[_defaultUnit] = 1.0f;
        _unitEnumeration[_defaultUnit] = enumerant++;
    }
}

DistanceUnitConverterPtr DistanceUnitConverter::create(UnitTypeDefPtr unitTypeDef)
{
    std::shared_ptr<DistanceUnitConverter> converter(new DistanceUnitConverter(unitTypeDef));
    return converter;
}

float DistanceUnitConverter::conversionRatio(const string& inputUnit, const string& outputUnit) const
{
    auto it = _unitScale.find(inputUnit);
    if (it == _unitScale.end())
    {
        throw ExceptionTypeError("Unrecognized source unit: " + inputUnit);
    }
    float fromScale = it->second;

    it = _unitScale.find(outputUnit);
    if (it == _unitScale.end())
    {
        throw ExceptionTypeError("Unrecognized destination unit: " + outputUnit);
    }
    float toScale = it->second;

    return (fromScale / toScale);

}

float DistanceUnitConverter::convert(float input, const string& inputUnit, const string& outputUnit) const
{
    if (inputUnit == outputUnit)
    {
        return input;
    }

    return (input * conversionRatio(inputUnit, outputUnit));
}

Vector2 DistanceUnitConverter::convert(Vector2 input, const string& inputUnit, const string& outputUnit) const
{
    if (inputUnit == outputUnit)
    {
        return input;
    }

    return (input * conversionRatio(inputUnit, outputUnit));
}

Vector3 DistanceUnitConverter::convert(Vector3 input, const string& inputUnit, const string& outputUnit) const
{
    if (inputUnit == outputUnit)
    {
        return input;
    }

    return (input * conversionRatio(inputUnit, outputUnit));
}

Vector4 DistanceUnitConverter::convert(Vector4 input, const string& inputUnit, const string& outputUnit) const
{
    if (inputUnit == outputUnit)
    {
        return input;
    }

    return (input * conversionRatio(inputUnit, outputUnit));
}

int DistanceUnitConverter::getUnitAsInteger(const string& unitName) const
{
    const auto it = _unitEnumeration.find(unitName);
    if (it != _unitEnumeration.end())
    {
        return it->second;
    }
    return -1;
}

string DistanceUnitConverter::getUnitFromInteger(int index) const
{
    auto it = std::find_if(
                _unitEnumeration.begin(), _unitEnumeration.end(),
                [&index](const std::pair<string, int> &e)->bool
                {
                    return (e.second == index);
                });

    if (it != _unitEnumeration.end())
    {
        return it->first;
    }
    return EMPTY_STRING;
}

UnitConverterRegistryPtr UnitConverterRegistry::create()
{
    static UnitConverterRegistryPtr registry(new UnitConverterRegistry());
    return registry;
}

bool UnitConverterRegistry::addUnitConverter(UnitTypeDefPtr def, UnitConverterPtr converter)
{
    const string& name = def->getName();
    if (_unitConverters.find(name) != _unitConverters.end())
    {
        return false;
    }
    _unitConverters[name] = converter;
    return true;
}

bool UnitConverterRegistry::removeUnitConverter(UnitTypeDefPtr def)
{
    const string& name = def->getName();
    auto it = _unitConverters.find(name);
    if (it == _unitConverters.end())
    {
        return false;
    }

    _unitConverters.erase(it);
    return true;
}

UnitConverterPtr UnitConverterRegistry::getUnitConverter(UnitTypeDefPtr def)
{
    const string& name = def->getName();
    auto it = _unitConverters.find(name);
    if (it != _unitConverters.end())
    {
        return it->second;
    }
    return nullptr;
}


void UnitConverterRegistry::clearUnitConverters()
{
    _unitConverters.clear();
}

int UnitConverterRegistry::getUnitAsInteger(const string& unitName) const
{
    for (auto it : _unitConverters)
    {
        int value = it.second->getUnitAsInteger(unitName);
        if (value >= 0)
            return value;
    }
    return -1;
}


}
