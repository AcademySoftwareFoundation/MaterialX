#include "../helpers.h"
#include <MaterialXCore/Geom.h>

#include <emscripten.h>
#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

#define BIND_GEOMINFO_FUNC_INSTANCE(NAME, T) \
.function("_setGeomPropValue" #NAME, &mx::GeomInfo::setGeomPropValue<T>)

extern "C"
{
    EMSCRIPTEN_BINDINGS(geom)
    {
        ems::class_<mx::GeomElement, ems::base<mx::Element>>("GeomElement")
            .smart_ptr<std::shared_ptr<mx::GeomElement>>("GeomElement")
            .smart_ptr<std::shared_ptr<const mx::GeomElement>>("GeomElement")
            .function("setGeom", &mx::GeomElement::setGeom)
            .function("hasGeom", &mx::GeomElement::hasGeom)
            .function("getGeom", &mx::GeomElement::getGeom)
            .function("setCollectionString", &mx::GeomElement::setCollectionString)
            .function("hasCollectionString", &mx::GeomElement::hasCollectionString)
            .function("getCollectionString", &mx::GeomElement::getCollectionString)
            .function("setCollection", &mx::GeomElement::setCollection)
            .function("getCollection", &mx::GeomElement::getCollection);

        ems::class_<mx::GeomInfo, ems::base<mx::GeomElement>>("GeomInfo")
            .smart_ptr_constructor("GeomInfo", &std::make_shared<mx::GeomInfo, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::GeomInfo>>("GeomInfo")
            .function("addGeomProp", &mx::GeomInfo::addGeomProp)
            .function("getGeomProp", &mx::GeomInfo::getGeomProp)
            .function("getGeomProps", &mx::GeomInfo::getGeomProps)
            .function("removeGeomProp", &mx::GeomInfo::removeGeomProp)
            .function("addToken", &mx::GeomInfo::addToken)
            .function("getToken", &mx::GeomInfo::getToken)
            .function("getTokens", &mx::GeomInfo::getTokens)
            .function("removeToken", &mx::GeomInfo::removeToken)
            .function("setTokenValue", &mx::GeomInfo::setTokenValue)
            BIND_GEOMINFO_FUNC_INSTANCE(integer, int)
            BIND_GEOMINFO_FUNC_INSTANCE(boolean, bool)
            BIND_GEOMINFO_FUNC_INSTANCE(float, float)
            BIND_GEOMINFO_FUNC_INSTANCE(color3, mx::Color3)
            BIND_GEOMINFO_FUNC_INSTANCE(color4, mx::Color4)
            BIND_GEOMINFO_FUNC_INSTANCE(vector2, mx::Vector2)
            BIND_GEOMINFO_FUNC_INSTANCE(vector3, mx::Vector3)
            BIND_GEOMINFO_FUNC_INSTANCE(vector4, mx::Vector4)
            BIND_GEOMINFO_FUNC_INSTANCE(matrix33, mx::Matrix33)
            BIND_GEOMINFO_FUNC_INSTANCE(matrix44, mx::Matrix44)
            BIND_GEOMINFO_FUNC_INSTANCE(string, std::string)
            BIND_GEOMINFO_FUNC_INSTANCE(integerarray, mx::IntVec)
            BIND_GEOMINFO_FUNC_INSTANCE(booleanarray, mx::BoolVec)
            BIND_GEOMINFO_FUNC_INSTANCE(floatarray, mx::FloatVec)
            BIND_GEOMINFO_FUNC_INSTANCE(stringarray, mx::StringVec)
            .class_property("CATEGORY", &mx::GeomInfo::CATEGORY);

        ems::class_<mx::GeomProp, ems::base<mx::ValueElement>>("GeomProp")
            .smart_ptr_constructor("GeomProp", &std::make_shared<mx::GeomProp, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::GeomProp>>("GeomProp")
            .class_property("CATEGORY", &mx::GeomProp::CATEGORY);

        ems::class_<mx::GeomPropDef, ems::base<mx::Element>>("GeomPropDef")
            .smart_ptr_constructor("GeomPropDef", &std::make_shared<mx::GeomPropDef, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::GeomPropDef>>("GeomPropDef")
            .function("setGeomProp", &mx::GeomPropDef::setGeomProp)
            .function("hasGeomProp", &mx::GeomPropDef::hasGeomProp)
            .function("getGeomProp", &mx::GeomPropDef::getGeomProp)
            .function("setSpace", &mx::GeomPropDef::setSpace)
            .function("hasSpace", &mx::GeomPropDef::hasSpace)
            .function("getSpace", &mx::GeomPropDef::getSpace)
            .function("setIndex", &mx::GeomPropDef::setIndex)
            .function("hasIndex", &mx::GeomPropDef::hasIndex)
            .function("getIndex", &mx::GeomPropDef::getIndex)
            .function("setGeomProp", &mx::GeomPropDef::setGeomProp)
            .function("hasGeomProp", &mx::GeomPropDef::hasGeomProp)
            .function("getGeomProp", &mx::GeomPropDef::getGeomProp)
            .class_property("CATEGORY", &mx::GeomPropDef::CATEGORY);

        ems::class_<mx::Collection, ems::base<mx::Element>>("Collection")
            .smart_ptr_constructor("Collection", &std::make_shared<mx::Collection, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::Collection>>("Collection")
            .function("setIncludeGeom", &mx::Collection::setIncludeGeom)
            .function("hasIncludeGeom", &mx::Collection::hasIncludeGeom)
            .function("getIncludeGeom", &mx::Collection::getIncludeGeom)
            .function("setExcludeGeom", &mx::Collection::setExcludeGeom)
            .function("hasExcludeGeom", &mx::Collection::hasExcludeGeom)
            .function("getExcludeGeom", &mx::Collection::getExcludeGeom)
            .function("setIncludeCollectionString", &mx::Collection::setIncludeCollectionString)
            .function("hasIncludeCollectionString", &mx::Collection::hasIncludeCollectionString)
            .function("getIncludeCollectionString", &mx::Collection::getIncludeCollectionString)
            .function("setIncludeCollection", &mx::Collection::setIncludeCollection)
            .function("setIncludeCollections", &mx::Collection::setIncludeCollections)
            .function("getIncludeCollections", &mx::Collection::getIncludeCollections)
            .function("hasIncludeCycle", &mx::Collection::hasIncludeCycle)
            .function("matchesGeomString", &mx::Collection::matchesGeomString)
            .class_property("CATEGORY", &mx::Collection::CATEGORY);

        ems::function("geomStringsMatch", &mx::geomStringsMatch);
        
        ems::function("UNIVERSAL_GEOM_NAME", ems::optional_override([]() {
            return mx::UNIVERSAL_GEOM_NAME;
        }));
        
    }
}