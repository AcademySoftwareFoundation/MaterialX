#include "../helpers.h"
#include <MaterialXCore/Property.h>

#include <emscripten.h>
#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

#define BIND_PROPERTYSET_TYPE_INSTANCE(NAME, T) \
.function("_setPropertyValue" #NAME, &mx::PropertySet::setPropertyValue<T>)

extern "C"
{
    EMSCRIPTEN_BINDINGS(property)
    {

        ems::class_<mx::Property, ems::base<mx::ValueElement>>("Property")
            .smart_ptr_constructor("Property", &std::make_shared<mx::Property, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::Property>>("Property")
            .class_property("CATEGORY", &mx::Property::CATEGORY);

        ems::class_<mx::PropertyAssign, ems::base<mx::ValueElement>>("PropertyAssign")
            .smart_ptr_constructor("PropertyAssign", &std::make_shared<mx::PropertyAssign, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::PropertyAssign>>("PropertyAssign")
            .function("setProperty", &mx::PropertyAssign::setProperty)
            .function("hasProperty", &mx::PropertyAssign::hasProperty)
            .function("getProperty", &mx::PropertyAssign::getProperty)
            .function("setGeom", &mx::PropertyAssign::setGeom)
            .function("hasGeom", &mx::PropertyAssign::hasGeom)
            .function("getGeom", &mx::PropertyAssign::getGeom)
            .function("setCollectionString", &mx::PropertyAssign::setCollectionString)
            .function("hasCollectionString", &mx::PropertyAssign::hasCollectionString)
            .function("getCollectionString", &mx::PropertyAssign::getCollectionString)
            .function("setCollection", &mx::PropertyAssign::setCollection)
            .function("getCollection", &mx::PropertyAssign::getCollection)
            .class_property("CATEGORY", &mx::PropertyAssign::CATEGORY);

        ems::class_<mx::PropertySet, ems::base<mx::Element>>("PropertySet")
            .smart_ptr_constructor("PropertySet", &std::make_shared<mx::PropertySet, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::PropertySet>>("PropertySet")
            .function("addProperty", &mx::PropertySet::addProperty)
            .function("getProperties", &mx::PropertySet::getProperties)
            .function("removeProperty", &mx::PropertySet::removeProperty)
            .function("_getPropertyValue", &mx::PropertySet::getPropertyValue)
            BIND_PROPERTYSET_TYPE_INSTANCE(integer, int)
            BIND_PROPERTYSET_TYPE_INSTANCE(boolean, bool)
            BIND_PROPERTYSET_TYPE_INSTANCE(float, float)
            BIND_PROPERTYSET_TYPE_INSTANCE(color3, mx::Color3)
            BIND_PROPERTYSET_TYPE_INSTANCE(color4, mx::Color4)
            BIND_PROPERTYSET_TYPE_INSTANCE(vector2, mx::Vector2)
            BIND_PROPERTYSET_TYPE_INSTANCE(vector3, mx::Vector3)
            BIND_PROPERTYSET_TYPE_INSTANCE(vector4, mx::Vector4)
            BIND_PROPERTYSET_TYPE_INSTANCE(matrix33, mx::Matrix33)
            BIND_PROPERTYSET_TYPE_INSTANCE(matrix44, mx::Matrix44)
            BIND_PROPERTYSET_TYPE_INSTANCE(string, std::string)
            BIND_PROPERTYSET_TYPE_INSTANCE(integerarray, mx::IntVec)
            BIND_PROPERTYSET_TYPE_INSTANCE(booleanarray, mx::BoolVec)
            BIND_PROPERTYSET_TYPE_INSTANCE(floatarray, mx::FloatVec)
            BIND_PROPERTYSET_TYPE_INSTANCE(stringarray, mx::StringVec)
            .class_property("CATEGORY", &mx::Property::CATEGORY);

        ems::class_<mx::PropertySetAssign, ems::base<mx::GeomElement>>("PropertySetAssign")
            .smart_ptr_constructor("PropertySetAssign", &std::make_shared<mx::PropertySetAssign, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::PropertySetAssign>>("PropertySetAssign")
            .function("setPropertySetString", &mx::PropertySetAssign::setPropertySetString)
            .function("hasPropertySetString", &mx::PropertySetAssign::hasPropertySetString)
            .function("getPropertySetString", &mx::PropertySetAssign::getPropertySetString)
            .function("setPropertySet", &mx::PropertySetAssign::setPropertySet)
            .function("getPropertySet", &mx::PropertySetAssign::getPropertySet)
            .class_property("CATEGORY", &mx::PropertySetAssign::CATEGORY);
    }
}