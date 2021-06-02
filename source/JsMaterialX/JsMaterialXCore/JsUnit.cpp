//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include "../VectorHelper.h"
#include "../Helpers.h"

#include <MaterialXCore/Unit.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

EMSCRIPTEN_BINDINGS(unit)
{

    ems::class_<mx::UnitConverter>("UnitConverter")
        .smart_ptr<std::shared_ptr<mx::UnitConverter>>("UnitConverter")
        .smart_ptr<std::shared_ptr<const mx::UnitConverter>>("UnitConverter")
        .function("convertFloat", ems::select_overload<float(float, const std::string&, const std::string&) const>(&mx::UnitConverter::convert))
        .function("getUnitAsInteger", &mx::UnitConverter::getUnitAsInteger)
        .function("getUnitFromInteger", &mx::UnitConverter::getUnitFromInteger)
        .function("convertVector2", ems::select_overload<mx::Vector2(const mx::Vector2&, const std::string&, const std::string&) const>(&mx::UnitConverter::convert))
        .function("convertVector3", ems::select_overload<mx::Vector3(const mx::Vector3&, const std::string&, const std::string&) const>(&mx::UnitConverter::convert))
        .function("convertVector4", ems::select_overload<mx::Vector4(const mx::Vector4&, const std::string&, const std::string&) const>(&mx::UnitConverter::convert))
        .function("write", &mx::UnitConverter::write, ems::pure_virtual());

    ems::class_<mx::LinearUnitConverter, ems::base<mx::UnitConverter>>("LinearUnitConverter")
        .smart_ptr<std::shared_ptr<mx::LinearUnitConverter>>("LinearUnitConverter")
        .smart_ptr<std::shared_ptr<const mx::LinearUnitConverter>>("LinearUnitConverter")
        .class_function("create", &mx::LinearUnitConverter::create)
        .function("getUnitType", &mx::LinearUnitConverter::getUnitType)
        .function("write", &mx::LinearUnitConverter::write)
        .function("getUnitScale", &mx::LinearUnitConverter::getUnitScale)
        .function("conversionRatio", &mx::LinearUnitConverter::conversionRatio)
        .function("convertFloat", ems::select_overload<float(float, const std::string&, const std::string&) const>(&mx::LinearUnitConverter::convert))
        .function("convertVector2", ems::select_overload<mx::Vector2(const mx::Vector2&, const std::string&, const std::string&) const>(&mx::LinearUnitConverter::convert))
        .function("convertVector3", ems::select_overload<mx::Vector3(const mx::Vector3&, const std::string&, const std::string&) const>(&mx::LinearUnitConverter::convert))
        .function("convertVector4", ems::select_overload<mx::Vector4(const mx::Vector4&, const std::string&, const std::string&) const>(&mx::LinearUnitConverter::convert))
        .function("getUnitAsInteger", &mx::LinearUnitConverter::getUnitAsInteger)
        .function("getUnitFromInteger", &mx::LinearUnitConverter::getUnitFromInteger);

    ems::class_<mx::UnitConverterRegistry>("UnitConverterRegistry")
        .smart_ptr<std::shared_ptr<mx::UnitConverterRegistry>>("UnitConverterRegistry")
        .smart_ptr<std::shared_ptr<const mx::UnitConverterRegistry>>("UnitConverterRegistry")
        .class_function("create", &mx::UnitConverterRegistry::create)
        .function("addUnitConverter", &mx::UnitConverterRegistry::addUnitConverter)
        .function("removeUnitConverter", &mx::UnitConverterRegistry::removeUnitConverter)
        .function("getUnitConverter", &mx::UnitConverterRegistry::getUnitConverter)
        .function("clearUnitConverters", &mx::UnitConverterRegistry::clearUnitConverters)
        .function("getUnitAsInteger", &mx::UnitConverterRegistry::getUnitAsInteger)
        .function("write", &mx::UnitConverterRegistry::write);
}
