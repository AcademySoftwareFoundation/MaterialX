//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/UnitConverter.h>

#include <MaterialXCore/Util.h>

namespace MaterialX
{

namespace {

const string SCALE_ATTRIBUTE = "scale";

} // anonymous namespace

//
// LinearUnitConverter methods
//

LinearUnitConverter::LinearUnitConverter(UnitTypeDefPtr unitTypeDef)
{
    if (!unitTypeDef)
    {
        return;
    }

    // Populate the unit scale and offset maps for each UnitDef.
    unsigned int enumerant = 0;
    for (UnitDefPtr unitdef : unitTypeDef->getUnitDefs())
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

    _unitType = unitTypeDef->getName();
}

LinearUnitConverterPtr LinearUnitConverter::create(UnitTypeDefPtr unitTypeDef)
{
    return std::shared_ptr<LinearUnitConverter>(new LinearUnitConverter(unitTypeDef));
}

float LinearUnitConverter::conversionRatio(const string& inputUnit, const string& outputUnit) const
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

    return fromScale / toScale;
}

float LinearUnitConverter::convert(float input, const string& inputUnit, const string& outputUnit) const
{
    if (inputUnit == outputUnit)
    {
        return input;
    }

    return input * conversionRatio(inputUnit, outputUnit);
}

Vector2 LinearUnitConverter::convert(const Vector2& input, const string& inputUnit, const string& outputUnit) const
{
    if (inputUnit == outputUnit)
    {
        return input;
    }

    return input * conversionRatio(inputUnit, outputUnit);
}

Vector3 LinearUnitConverter::convert(const Vector3& input, const string& inputUnit, const string& outputUnit) const
{
    if (inputUnit == outputUnit)
    {
        return input;
    }

    return input * conversionRatio(inputUnit, outputUnit);
}

Vector4 LinearUnitConverter::convert(const Vector4& input, const string& inputUnit, const string& outputUnit) const
{
    if (inputUnit == outputUnit)
    {
        return input;
    }

    return input * conversionRatio(inputUnit, outputUnit);
}

int LinearUnitConverter::getUnitAsInteger(const string& unitName) const
{
    const auto it = _unitEnumeration.find(unitName);
    if (it != _unitEnumeration.end())
    {
        return it->second;
    }
    return -1;
}

string LinearUnitConverter::getUnitFromInteger(int index) const
{
    auto it = std::find_if(_unitEnumeration.begin(), _unitEnumeration.end(),
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

//
// UnitConverterRegistry methods
//

UnitConverterRegistryPtr UnitConverterRegistry::create()
{
    static UnitConverterRegistryPtr registry(new UnitConverterRegistry());
    return registry;
}

bool UnitConverterRegistry::addUnitConverter(UnitTypeDefPtr def, UnitConverterPtr converter)
{
    if (def && _unitConverters.find(def->getName()) == _unitConverters.end())
    {
        _unitConverters[def->getName()] = converter;
        return true;
    }

    return false;
}

bool UnitConverterRegistry::removeUnitConverter(UnitTypeDefPtr def)
{
    if (def)
    {
        auto it = _unitConverters.find(def->getName());
        if (it != _unitConverters.end())
        {
            _unitConverters.erase(it);
            return true;
        }
    }

    return false;
}

UnitConverterPtr UnitConverterRegistry::getUnitConverter(UnitTypeDefPtr def)
{
    if (def)
    {
        auto it = _unitConverters.find(def->getName());
        if (it != _unitConverters.end())
        {
            return it->second;
        }
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

} // namespace MaterialX
