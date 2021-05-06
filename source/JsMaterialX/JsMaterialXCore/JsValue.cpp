#include "../helpers.h"
#include <MaterialXCore/Value.h>

#include <emscripten.h>
#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

#define BIND_TYPE_INSTANCE(NAME, T)                                           \
    ems::class_<mx::TypedValue<T>, ems::base<mx::Value>>("TypedValue_" #NAME) \
        .smart_ptr<std::shared_ptr<mx::TypedValue<T>>>("TypedValue<T>")       \
        .function("getData", &mx::TypedValue<T>::getData)                     \
        .function("getValueString", &mx::TypedValue<T>::getValueString)       \
        .function("getTypeString", &mx::TypedValue<T>::getTypeString)         \
        .class_function("createValue", &mx::Value::createValue<T>);

extern "C"
{
    EMSCRIPTEN_BINDINGS(value)
    {
        ems::class_<mx::Value>("Value")
            .smart_ptr<std::shared_ptr<mx::Value>>("Value")
            .smart_ptr<std::shared_ptr<const mx::Value>>("Value")
            .function("getValueString", &mx::Value::getValueString)
            .function("getTypeString", &mx::Value::getTypeString)
            .class_function("createValueFromStrings", &mx::Value::createValueFromStrings);

        BIND_TYPE_INSTANCE(integer, int)
        BIND_TYPE_INSTANCE(boolean, bool)
        BIND_TYPE_INSTANCE(float, float)
        BIND_TYPE_INSTANCE(color3, mx::Color3)
        BIND_TYPE_INSTANCE(color4, mx::Color4)
        BIND_TYPE_INSTANCE(vector2, mx::Vector2)
        BIND_TYPE_INSTANCE(vector3, mx::Vector3)
        BIND_TYPE_INSTANCE(vector4, mx::Vector4)
        BIND_TYPE_INSTANCE(matrix33, mx::Matrix33)
        BIND_TYPE_INSTANCE(matrix44, mx::Matrix44)
        BIND_TYPE_INSTANCE(string, std::string)
        BIND_TYPE_INSTANCE(integerarray, mx::IntVec)
        BIND_TYPE_INSTANCE(booleanarray, mx::BoolVec)
        BIND_TYPE_INSTANCE(floatarray, mx::FloatVec)
        BIND_TYPE_INSTANCE(stringarray, mx::StringVec)
    }
}