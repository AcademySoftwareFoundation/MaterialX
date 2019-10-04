//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

/**
* Copyright 2019 Autodesk, Inc.
* All rights reserved.
*
* This computer source code and related instructions and comments are the unpublished confidential
* and proprietary information of Autodesk, Inc. and are protected under Federal copyright and state
* trade secret law. They may not be disclosed to, copied or used by any third party without the
* prior written consent of Autodesk, Inc.
*
* @file Units.cpp
* @brief Units support
*/

#include <algorithm>
#include <MaterialXCore/Util.h>
#include <MaterialXCore/Value.h>
#include <MaterialXCore/UnitConverter.h>

namespace MaterialX
{
const string LengthUnitConverter::LENGTH_UNIT = "length";

LengthUnitConverter::LengthUnitConverter(UnitTypeDefPtr unitTypeDef) :
    UnitConverter()
{
    static const string SCALE_ATTRIBUTE = "scale";
    unsigned int enumerant = 0;

    // Populate the unit scale and offset maps for each UnitDef. 
    vector<UnitDefPtr> unitDefs = unitTypeDef->getUnitDefs();
    for (UnitDefPtr unitdef : unitDefs)
    {
        const string& name = unitdef->getName();
        if (!name.empty())
        {
            const string& scaleString = unitdef->getAttribute(SCALE_ATTRIBUTE);
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

    // In case the default unit was not specified in the unittypedef explicit
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

LengthUnitConverterPtr LengthUnitConverter::create(UnitTypeDefPtr unitTypeDef)
{
    std::shared_ptr<LengthUnitConverter> converter(new LengthUnitConverter(unitTypeDef));
    return converter;
}

float LengthUnitConverter::conversionRatio(const string& inputUnit, const string& outputUnit) const
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

float LengthUnitConverter::convert(float input, const string& inputUnit, const string& outputUnit) const
{
    if (inputUnit == outputUnit)
    {
        return input;
    }

    return (input * conversionRatio(inputUnit, outputUnit));
}

Vector2 LengthUnitConverter::convert(Vector2 input, const string& inputUnit, const string& outputUnit) const
{
    if (inputUnit == outputUnit)
    {
        return input;
    }

    return (input * conversionRatio(inputUnit, outputUnit));
}

Vector3 LengthUnitConverter::convert(Vector3 input, const string& inputUnit, const string& outputUnit) const
{
    if (inputUnit == outputUnit)
    {
        return input;
    }

    return (input * conversionRatio(inputUnit, outputUnit));
}

Vector4 LengthUnitConverter::convert(Vector4 input, const string& inputUnit, const string& outputUnit) const
{
    if (inputUnit == outputUnit)
    {
        return input;
    }

    return (input * conversionRatio(inputUnit, outputUnit));
}

int LengthUnitConverter::getUnitAsInteger(const string& unitName) const
{
    const auto it = _unitEnumeration.find(unitName);
    if (it != _unitEnumeration.end())
    {
        return it->second;
    }
    return -1;
}

string LengthUnitConverter::getUnitFromInteger(unsigned int index) const
{
    auto it = std::find_if(
                _unitEnumeration.begin(), _unitEnumeration.end(),
                [&index](const std::pair<string, unsigned int> &e)->bool
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
