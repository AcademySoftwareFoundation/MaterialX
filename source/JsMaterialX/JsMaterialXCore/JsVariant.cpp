#include "../helpers.h"
#include <MaterialXCore/Variant.h>

#include <emscripten.h>
#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

extern "C"
{
    EMSCRIPTEN_BINDINGS(variant)
    {
        ems::class_<mx::Variant, ems::base<mx::InterfaceElement>>("Variant")
            .smart_ptr_constructor("Variant", &std::make_shared<mx::Variant, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::Variant>>("Variant")
            .class_property("CATEGORY", &mx::Variant::CATEGORY);

        ems::class_<mx::VariantSet, ems::base<mx::Element>>("VariantSet")
            .smart_ptr_constructor("VariantSet", &std::make_shared<mx::VariantSet, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::VariantSet>>("VariantSet")
            .function("addVariant", &mx::VariantSet::addVariant)
            .function("getVariant", &mx::VariantSet::getVariant)
            .function("getVariants", &mx::VariantSet::getVariants)
            .function("removeVariant", &mx::VariantSet::removeVariant)
            .class_property("CATEGORY", &mx::VariantSet::CATEGORY);

        ems::class_<mx::VariantAssign, ems::base<mx::Element>>("VariantAssign")
            .smart_ptr_constructor("VariantAssign", &std::make_shared<mx::VariantAssign, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::VariantAssign>>("VariantAssign")
            .function("setVariantSetString", &mx::VariantAssign::setVariantSetString)
            .function("hasVariantSetString", &mx::VariantAssign::hasVariantSetString)
            .function("getVariantSetString", &mx::VariantAssign::getVariantSetString)
            .function("setVariantString", &mx::VariantAssign::setVariantString)
            .function("hasVariantString", &mx::VariantAssign::hasVariantString)
            .function("getVariantString", &mx::VariantAssign::getVariantString)
            .class_property("CATEGORY", &mx::VariantAssign::CATEGORY);
    }
}