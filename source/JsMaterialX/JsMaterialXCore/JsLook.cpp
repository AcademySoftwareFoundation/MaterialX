#include "../helpers.h"
#include <MaterialXCore/Look.h>

#include <emscripten.h>
#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

extern "C"
{
    EMSCRIPTEN_BINDINGS(look)
    {
        ems::class_<mx::Look, ems::base<mx::Element>>("Look")
            .smart_ptr_constructor("Look", &std::make_shared<mx::Look, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::Look>>("Look")
            .function("addMaterialAssign", &mx::Look::addMaterialAssign)
            .function("getMaterialAssign", &mx::Look::getMaterialAssign)
            .function("getMaterialAssigns", &mx::Look::getMaterialAssigns)
            .function("getActiveMaterialAssigns", &mx::Look::getActiveMaterialAssigns)
            .function("removeMaterialAssign", &mx::Look::removeMaterialAssign)

            .function("addPropertyAssign", &mx::Look::addPropertyAssign) 
            .function("getPropertyAssign", &mx::Look::getPropertyAssign)
            .function("getPropertyAssigns", &mx::Look::getPropertyAssigns)
            .function("getActivePropertyAssigns", &mx::Look::getActivePropertyAssigns)
            .function("removePropertyAssign", &mx::Look::removePropertyAssign)
            .function("addPropertySetAssign", &mx::Look::addPropertySetAssign)
            .function("getPropertySetAssign", &mx::Look::getPropertySetAssign)
            .function("getPropertySetAssigns", &mx::Look::getPropertySetAssigns)
            .function("getActivePropertySetAssigns", &mx::Look::getActivePropertySetAssigns)
            .function("removePropertySetAssign", &mx::Look::removePropertySetAssign)
            .function("addVariantAssign", &mx::Look::addVariantAssign)
            .function("getVariantAssign", &mx::Look::getVariantAssign)
            .function("getVariantAssigns", &mx::Look::getVariantAssigns)
            .function("getActiveVariantAssigns", &mx::Look::getActiveVariantAssigns)
            .function("removeVariantAssign", &mx::Look::removeVariantAssign)
            .function("addVisibility", &mx::Look::addVisibility)
            .function("getVisibility", &mx::Look::getVisibility)
            .function("getVisibilities", &mx::Look::getVisibilities)
            .function("getActiveVisibilities", &mx::Look::getActiveVisibilities)
            .function("removeVisibility", &mx::Look::removeVisibility)

            .class_property("CATEGORY", &mx::Look::CATEGORY);

        ems::class_<mx::LookGroup, ems::base<mx::Element>>("LookGroup")
            .smart_ptr_constructor("LookGroup", &std::make_shared<mx::LookGroup, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::LookGroup>>("LookGroup")
            .function("getLooks", &mx::LookGroup::getLooks)
            .function("setLooks", &mx::LookGroup::setLooks)
            .function("getEnabledLooksString", &mx::LookGroup::getEnabledLooksString)
            .function("setEnabledLooks", &mx::LookGroup::setEnabledLooks)
            .class_property("CATEGORY", &mx::LookGroup::CATEGORY)
            .class_property("LOOKS_ATTRIBUTE", &mx::LookGroup::LOOKS_ATTRIBUTE)
            .class_property("ENABLED_ATTRIBUTE", &mx::LookGroup::ENABLED_ATTRIBUTE);

        ems::class_<mx::MaterialAssign, ems::base<mx::GeomElement>>("MaterialAssign")
            .smart_ptr_constructor("MaterialAssign", &std::make_shared<mx::MaterialAssign, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::MaterialAssign>>("MaterialAssign")
            .function("setMaterial", &mx::MaterialAssign::setMaterial)
            .function("hasMaterial", &mx::MaterialAssign::hasMaterial)
            .function("getMaterial", &mx::MaterialAssign::getMaterial)
            .function("setExclusive", &mx::MaterialAssign::setExclusive)
            .function("getExclusive", &mx::MaterialAssign::getExclusive)
            .function("getReferencedMaterial", &mx::MaterialAssign::getReferencedMaterial)
            .class_property("CATEGORY", &mx::MaterialAssign::CATEGORY);

        ems::class_<mx::Visibility, ems::base<mx::GeomElement>>("Visibility")
            .smart_ptr_constructor("Visibility", &std::make_shared<mx::Visibility, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::Visibility>>("Visibility")
            .function("setViewerGeom", &mx::Visibility::setViewerGeom)
            .function("hasViewerGeom", &mx::Visibility::hasViewerGeom)
            .function("getViewerGeom", &mx::Visibility::getViewerGeom)
            .function("setViewerCollection", &mx::Visibility::setViewerCollection)
            .function("hasViewerCollection", &mx::Visibility::hasViewerCollection)
            .function("getViewerCollection", &mx::Visibility::getViewerCollection)
            .function("setVisibilityType", &mx::Visibility::setVisibilityType)
            .function("hasVisibilityType", &mx::Visibility::hasVisibilityType)
            .function("getVisibilityType", &mx::Visibility::getVisibilityType)
            .function("setVisible", &mx::Visibility::setVisible)
            .function("getVisible", &mx::Visibility::getVisible)
            .class_property("CATEGORY", &mx::Visibility::CATEGORY);
    }
}