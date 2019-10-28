//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Util.h>
#include <MaterialXCore/Value.h>
#include <MaterialXGenShader/UnitConverter.h>

namespace MaterialX
{
DefaultUnitConverter::DefaultUnitConverter(UnitTypeDefPtr unitTypeDef) :
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

    // In case the default unit was not specified in the UnitDef, explicitly
    // add this to be able to accept conversion with the default as the input
    // or output unit.
    _defaultUnit = unitTypeDef->getDefault();
    auto it = _unitScale.find(_defaultUnit);
    if (it == _unitScale.end())
    {
        _unitScale[_defaultUnit] = 1.0f;
        _unitEnumeration[_defaultUnit] = enumerant++;
    }

    _unitType = unitTypeDef->getName();
}

DefaultUnitConverterPtr DefaultUnitConverter::create(UnitTypeDefPtr unitTypeDef)
{
    std::shared_ptr<DefaultUnitConverter> converter(new DefaultUnitConverter(unitTypeDef));
    return converter;
}

float DefaultUnitConverter::conversionRatio(const string& inputUnit, const string& outputUnit) const
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

float DefaultUnitConverter::convert(float input, const string& inputUnit, const string& outputUnit) const
{
    if (inputUnit == outputUnit)
    {
        return input;
    }

    return (input * conversionRatio(inputUnit, outputUnit));
}

Vector2 DefaultUnitConverter::convert(Vector2 input, const string& inputUnit, const string& outputUnit) const
{
    if (inputUnit == outputUnit)
    {
        return input;
    }

    return (input * conversionRatio(inputUnit, outputUnit));
}

Vector3 DefaultUnitConverter::convert(Vector3 input, const string& inputUnit, const string& outputUnit) const
{
    if (inputUnit == outputUnit)
    {
        return input;
    }

    return (input * conversionRatio(inputUnit, outputUnit));
}

Vector4 DefaultUnitConverter::convert(Vector4 input, const string& inputUnit, const string& outputUnit) const
{
    if (inputUnit == outputUnit)
    {
        return input;
    }

    return (input * conversionRatio(inputUnit, outputUnit));
}

int DefaultUnitConverter::getUnitAsInteger(const string& unitName) const
{
    const auto it = _unitEnumeration.find(unitName);
    if (it != _unitEnumeration.end())
    {
        return it->second;
    }
    return -1;
}

string DefaultUnitConverter::getUnitFromInteger(int index) const
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
