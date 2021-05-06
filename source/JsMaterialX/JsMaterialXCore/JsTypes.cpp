#include "../helpers.h"
#include <MaterialXCore/Types.h>
#include <MaterialXCore/Value.h>

#include <emscripten.h>
#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

#define BIND_VECTOR_SUBCLASS(V)                                                                          \
    .function("equals", ems::optional_override([](V &self, const V &rhs) { return self == rhs; }))            \
        .function("not_equals", ems::optional_override([](V &self, const V &rhs) { return self != rhs; }))    \
        .function("add", ems::optional_override([](V &self, const V &rhs) { return self + rhs; }))            \
        .function("sub", ems::optional_override([](V &self, const V &rhs) { return self - rhs; }))            \
        .function("multiply", ems::optional_override([](V &self, const V &rhs) { return self * rhs; }))       \
        .function("divide", ems::optional_override([](V &self, const V &rhs) { return self / rhs; }))         \
        .function("getMagnitude", ems::optional_override([](V &self) { return self.V::getMagnitude(); }))     \
        .function("getNormalized", ems::optional_override([](V &self) { return self.V::getNormalized(); }))   \
        .function("dot", ems::optional_override([](V &self, const V &rhs) { return self.V::dot(rhs); }))      \
        .function("getItem", ems::optional_override([](V &self, size_t i) { return self[i]; }))               \
        .function("setItem", ems::optional_override([](V &self, size_t i, float f) { self[i] = f; }))         \
        .function("toString", ems::optional_override([](const V &self) { return toValueString(self); }))      \
        .function("copy", ems::optional_override([](const V &self) { return V(self); }))                      \
        .function("numElements", ems::optional_override([](const V &self) { return self.V::numElements(); })) \
        .function("length", ems::optional_override([](const V &self) { return self.V::numElements(); }))

#define BIND_MATRIX_SUBCLASS(M)                                                                                                                         \
    .function("equals", ems::optional_override([](M &self, const M &rhs) { return self == rhs; }))                                                           \
        .function("not_equals", ems::optional_override([](M &self, const M &rhs) { return self != rhs; }))                                                   \
        .function("add", ems::optional_override([](M &self, const M &rhs) { return self + rhs; }))                                                           \
        .function("sub", ems::optional_override([](M &self, const M &rhs) { return self - rhs; }))                                                           \
        .function("multiply", ems::optional_override([](M &self, const M &rhs) { return self * rhs; }))                                                      \
        .function("divide", ems::optional_override([](M &self, const M &rhs) { return self / rhs; }))                                                        \
        .function("getItem", ems::optional_override([](M &self, size_t row, size_t col) { return self[row][col]; }))                                         \
        .function("setItem", ems::optional_override([](M &self, size_t row, size_t col, float f) { self[row][col] = f; }))                                   \
        .function("toString", ems::optional_override([](const M &self) { return toValueString(self); }))                                                     \
        .function("copy", ems::optional_override([](const M &self) { return M(self); }))                                                                     \
        .function("isEquivalent", ems::optional_override([](const M &self, const M &rhs, float tolerance) { return self.M::isEquivalent(rhs, tolerance); })) \
        .function("getTranspose", ems::optional_override([](const M &self) { return self.M::getTranspose(); }))                                              \
        .function("getDeterminant", ems::optional_override([](const M &self) { return self.M::getDeterminant(); }))                                          \
        .function("getAdjugate", ems::optional_override([](const M &self) { return self.M::getAdjugate(); }))                                                \
        .function("getInverse", ems::optional_override([](const M &self) { return self.M::getInverse(); }))                                                  \
        .function("numRows", ems::optional_override([](const M &self) { return self.M::numRows(); }))                                                        \
        .function("numColumns", ems::optional_override([](const M &self) { return self.M::numColumns(); }))                                                  \
        .function("length", ems::optional_override([](const M &self) { return self.M::numRows(); }))

extern "C"
{
    EMSCRIPTEN_BINDINGS(types)
    {
        ems::class_<mx::VectorBase>("VectorBase");
        ems::class_<mx::MatrixBase>("MatrixBase");

        ems::class_<mx::Vector2, ems::base<mx::VectorBase>>("Vector2")
            .constructor<>()
            .constructor<float, float>()
                BIND_VECTOR_SUBCLASS(mx::Vector2)
            .function("cross", &mx::Vector2::cross);

        ems::class_<mx::Vector3, ems::base<mx::VectorBase>>("Vector3")
            .constructor<>()
            .constructor<float, float, float>()
                BIND_VECTOR_SUBCLASS(mx::Vector3)
            .function("cross", &mx::Vector3::cross);

        ems::class_<mx::Vector4, ems::base<mx::VectorBase>>("Vector4")
            .constructor<>()
            .constructor<float, float, float, float>()
                BIND_VECTOR_SUBCLASS(mx::Vector4);

        ems::class_<mx::Color3, ems::base<mx::VectorBase>>("Color3")
            .constructor<>()
            .constructor<float, float, float>()
                BIND_VECTOR_SUBCLASS(mx::Color3);

        ems::class_<mx::Color4, ems::base<mx::VectorBase>>("Color4")
            .constructor<>()
            .constructor<float, float, float, float>()
                BIND_VECTOR_SUBCLASS(mx::Color4);

        ems::class_<mx::Matrix33, ems::base<mx::MatrixBase>>("Matrix33")
            .constructor<>()
            .constructor<float, float, float, float, float, float, float, float, float>()
                BIND_MATRIX_SUBCLASS(mx::Matrix33)
            .function("createScale", ems::optional_override([](const mx::Matrix33 &self, const mx::Vector2 &v) { return self.mx::Matrix33::createScale(v); }))
            .function("createTranslation", ems::optional_override([](const mx::Matrix33 &self, const mx::Vector2 &v) { return self.mx::Matrix33::createTranslation(v); }))
            .function("transformPoint", ems::optional_override([](const mx::Matrix33 &self, const mx::Vector2 &v) { return self.mx::Matrix33::transformPoint(v); }))
            .function("transformVector", ems::optional_override([](const mx::Matrix33 &self, const mx::Vector2 &v) { return self.mx::Matrix33::transformVector(v); }))
            .function("transformNormal", ems::optional_override([](const mx::Matrix33 &self, const mx::Vector3 &v) { return self.mx::Matrix33::transformNormal(v); }))
            .function("createRotation", ems::optional_override([](const mx::Matrix33 &self, float angle) { return self.mx::Matrix33::createRotation(angle); }))
            .class_property("IDENTITY", &mx::Matrix33::IDENTITY);

        ems::class_<mx::Matrix44, ems::base<mx::MatrixBase>>("Matrix44")
            .constructor<>()
            .constructor<float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float>()
                BIND_MATRIX_SUBCLASS(mx::Matrix44)
            .function("createScale", ems::optional_override([](const mx::Matrix44 &self, const mx::Vector3 &v) { return self.mx::Matrix44::createScale(v); }))
            .function("createTranslation", ems::optional_override([](const mx::Matrix44 &self, const mx::Vector3 &v) { return self.mx::Matrix44::createTranslation(v); }))
            .function("transformPoint", ems::optional_override([](const mx::Matrix44 &self, const mx::Vector3 &v) { return self.mx::Matrix44::transformPoint(v); }))
            .function("transformVector", ems::optional_override([](const mx::Matrix44 &self, const mx::Vector3 &v) { return self.mx::Matrix44::transformVector(v); }))
            .function("transformNormal", ems::optional_override([](const mx::Matrix44 &self, const mx::Vector3 &v) { return self.mx::Matrix44::transformNormal(v); }))
            .function("createRotationX", ems::optional_override([](const mx::Matrix44 &self, float angle) { return self.mx::Matrix44::createRotationX(angle); }))
            .function("createRotationY", ems::optional_override([](const mx::Matrix44 &self, float angle) { return self.mx::Matrix44::createRotationY(angle); }))
            .function("createRotationZ", ems::optional_override([](const mx::Matrix44 &self, float angle) { return self.mx::Matrix44::createRotationZ(angle); }))
            .class_property("IDENTITY", &mx::Matrix44::IDENTITY);

        ems::function("DEFAULT_TYPE_STRING", ems::optional_override([]() {
                     return mx::DEFAULT_TYPE_STRING;
                 }));
        ems::function("EMPTY_STRING", ems::optional_override([]() {
                     return mx::EMPTY_STRING;
                 }));
        ems::function("FILENAME_TYPE_STRING", ems::optional_override([]() {
                     return mx::FILENAME_TYPE_STRING;
                 }));
        ems::function("GEOMNAME_TYPE_STRING", ems::optional_override([]() {
                     return mx::GEOMNAME_TYPE_STRING;
                 }));
        ems::function("SURFACE_SHADER_TYPE_STRING", ems::optional_override([]() {
                     return mx::SURFACE_SHADER_TYPE_STRING;
                 }));
        ems::function("DISPLACEMENT_SHADER_TYPE_STRING", ems::optional_override([]() {
                     return mx::DISPLACEMENT_SHADER_TYPE_STRING;
                 }));
        ems::function("VOLUME_SHADER_TYPE_STRING", ems::optional_override([]() {
                     return mx::VOLUME_SHADER_TYPE_STRING;
                 }));
        ems::function("LIGHT_SHADER_TYPE_STRING", ems::optional_override([]() {
                     return mx::LIGHT_SHADER_TYPE_STRING;
                 }));
        ems::function("MULTI_OUTPUT_TYPE_STRING", ems::optional_override([]() {
                     return mx::MULTI_OUTPUT_TYPE_STRING;
                 }));
        ems::function("NONE_TYPE_STRING", ems::optional_override([]() {
                     return mx::NONE_TYPE_STRING;
                 }));
        ems::function("VALUE_STRING_TRUE", ems::optional_override([]() {
                     return mx::VALUE_STRING_TRUE;
                 }));
        ems::function("VALUE_STRING_FALSE", ems::optional_override([]() {
                     return mx::VALUE_STRING_FALSE;
                 }));
        ems::function("NAME_PREFIX_SEPARATOR", ems::optional_override([]() {
                     return mx::NAME_PREFIX_SEPARATOR;
                 }));
        ems::function("NAME_PATH_SEPARATOR", ems::optional_override([]() {
                     return mx::NAME_PATH_SEPARATOR;
                 }));
        ems::function("ARRAY_VALID_SEPARATORS", ems::optional_override([]() {
                     return mx::ARRAY_VALID_SEPARATORS;
                 }));
        ems::function("ARRAY_PREFERRED_SEPARATOR", ems::optional_override([]() {
                     return mx::ARRAY_PREFERRED_SEPARATOR;
                 }));
    }
}