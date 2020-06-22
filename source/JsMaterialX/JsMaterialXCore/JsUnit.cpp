#include "../helpers.h"
#include <MaterialXCore/Unit.h>

#include <emscripten.h>
#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;


extern "C"
{
    EMSCRIPTEN_BINDINGS(unit)
    {

        ems::class_<mx::UnitConverter>("UnitConverter")
            .smart_ptr<std::shared_ptr<mx::UnitConverter>>("UnitConverter")
            .smart_ptr<std::shared_ptr<const mx::UnitConverter>>("UnitConverter")
            .function("convert_float", ems::select_overload<float(float, const std::string&, const std::string&) const>(&mx::UnitConverter::convert))
            .function("convert_vector2", ems::select_overload<mx::Vector2(const mx::Vector2&, const std::string&, const std::string&) const>(&mx::UnitConverter::convert))
            .function("convert_vector3", ems::select_overload<mx::Vector3(const mx::Vector3&, const std::string&, const std::string&) const>(&mx::UnitConverter::convert))
            .function("convert_vector4", ems::select_overload<mx::Vector4(const mx::Vector4&, const std::string&, const std::string&) const>(&mx::UnitConverter::convert))
            .function("getUnitAsInteger", &mx::UnitConverter::getUnitAsInteger)
            .function("getUnitFromInteger", &mx::UnitConverter::getUnitFromInteger);

        ems::class_<mx::LinearUnitConverter, ems::base<mx::UnitConverter>>("LinearUnitConverter")
            .smart_ptr<std::shared_ptr<mx::LinearUnitConverter>>("LinearUnitConverter")
            .smart_ptr<std::shared_ptr<const mx::LinearUnitConverter>>("LinearUnitConverter")
            .function("convert_float", ems::select_overload<float(float, const std::string&, const std::string&) const>(&mx::LinearUnitConverter::convert))
            .function("convert_vector2", ems::select_overload<mx::Vector2(const mx::Vector2&, const std::string&, const std::string&) const>(&mx::LinearUnitConverter::convert))
            .function("convert_vector3", ems::select_overload<mx::Vector3(const mx::Vector3&, const std::string&, const std::string&) const>(&mx::LinearUnitConverter::convert))
            .function("convert_vector4", ems::select_overload<mx::Vector4(const mx::Vector4&, const std::string&, const std::string&) const>(&mx::LinearUnitConverter::convert))
            .function("getUnitAsInteger", &mx::LinearUnitConverter::getUnitAsInteger)
            .function("getUnitFromInteger", &mx::LinearUnitConverter::getUnitFromInteger)
            .function("getUnitScale", &mx::LinearUnitConverter::getUnitScale);
            
        ems::class_<mx::UnitConverterRegistry>("UnitConverterRegistry")
            .smart_ptr<std::shared_ptr<mx::UnitConverterRegistry>>("UnitConverterRegistry")
            .smart_ptr<std::shared_ptr<const mx::UnitConverterRegistry>>("UnitConverterRegistry")
            .function("addUnitConverter", &mx::UnitConverterRegistry::addUnitConverter)
            .function("removeUnitConverter", &mx::UnitConverterRegistry::removeUnitConverter)
            .function("getUnitConverter", &mx::UnitConverterRegistry::getUnitConverter)
            .function("clearUnitConverters", &mx::UnitConverterRegistry::clearUnitConverters)
            .function("create", ems::optional_override([](mx::UnitConverterRegistry &self) {
                return self.mx::UnitConverterRegistry::create();
            }));
    }
}