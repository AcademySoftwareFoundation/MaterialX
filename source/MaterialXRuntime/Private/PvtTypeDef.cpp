//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtTypeDef.h>

#include <MaterialXRuntime/RtTypeDef.h>

#include <MaterialXCore/Util.h>

#include <sstream>

namespace MaterialX
{

namespace
{

template<class T>
RtValue createValue(RtObject&)
{
    return RtValue((T)0);
}
template<> RtValue createValue<Matrix33>(RtObject& owner)
{
    return RtValue(Matrix33::IDENTITY, owner);
}
template<> RtValue createValue<Matrix44>(RtObject& owner)
{
    return RtValue(Matrix44::IDENTITY, owner);
}
template<> RtValue createValue<string>(RtObject& owner)
{
    return RtValue(string(""), owner);
}
template<> RtValue createValue<RtToken>(RtObject&)
{
    return RtValue(EMPTY_TOKEN);
}
RtValue createNoneValue(RtObject&)
{
    return RtValue(0);
}

template<class T>
void copyValue(const RtValue& src, RtValue& dest)
{
    dest = src;
}
template <> void copyValue<Matrix33>(const RtValue& src, RtValue& dest)
{
    dest.asMatrix33() = src.asMatrix33();
}
template <> void copyValue<Matrix44>(const RtValue& src, RtValue& dest)
{
    dest.asMatrix44() = src.asMatrix44();
}
template <> void copyValue<string>(const RtValue& src, RtValue& dest)
{
    dest.asString() = src.asString();
}
void copyNoneValue(const RtValue&, RtValue&)
{
}

template<class T>
void toStringValue(const RtValue&, string&)
{
    // TODO: Fix this for gcc/clang
#ifdef _WIN32
    static_assert(false, "toStringValue must be specialized for all types");
#endif
}
template <> void toStringValue<bool>(const RtValue& src, string& dest)
{
    std::stringstream ss;
    ss << src.asBool();
    dest = ss.str();
}
template <> void toStringValue<float>(const RtValue& src, string& dest)
{
    std::stringstream ss;
    ss << src.asFloat();
    dest = ss.str();
}
template <> void toStringValue<int>(const RtValue& src, string& dest)
{
    std::stringstream ss;
    ss << src.asFloat();
    dest = ss.str();
}
template<class T>
void toStringVector(const T& src, string& dest)
{
    std::stringstream ss;
    for (size_t i = 0; i < T::numElements(); ++i)
    {
        if (i)
        {
            ss << ", ";
        }
        ss << src[i];
    }
    dest = ss.str();
}
template<class T>
void toStringMatrix(const T& src, string& dest)
{
    std::stringstream ss;
    for (size_t i = 0; i < T::numColumns(); i++)
    {
        for (size_t j = 0; j < T::numRows(); j++)
        {
            if (i || j)
            {
                ss << ", ";
            }
            ss << src[i][j];
        }
    }
    dest = ss.str();
}
template <> void toStringValue<Color2>(const RtValue& src, string& dest)
{
    toStringVector(src.asColor2(), dest);
}
template <> void toStringValue<Color3>(const RtValue& src, string& dest)
{
    toStringVector(src.asColor3(), dest);
}
template <> void toStringValue<Color4>(const RtValue& src, string& dest)
{
    toStringVector(src.asColor4(), dest);
}
template <> void toStringValue<Vector2>(const RtValue& src, string& dest)
{
    toStringVector(src.asVector2(), dest);
}
template <> void toStringValue<Vector3>(const RtValue& src, string& dest)
{
    toStringVector(src.asVector3(), dest);
}
template <> void toStringValue<Vector4>(const RtValue& src, string& dest)
{
    toStringVector(src.asVector4(), dest);
}
template <> void toStringValue<Matrix33>(const RtValue& src, string& dest)
{
    toStringMatrix(src.asMatrix33(), dest);
}
template <> void toStringValue<Matrix44>(const RtValue& src, string& dest)
{
    toStringMatrix(src.asMatrix44(), dest);
}
template <> void toStringValue<string>(const RtValue& src, string& dest)
{
    dest = src.asString();
}
template <> void toStringValue<RtToken>(const RtValue& src, string& dest)
{
    dest = src.asToken().str();
}
void toStringNoneValue(const RtValue&, string& dest)
{
    dest = "";
}

template<class T>
void fromStringValue(const string& str, RtValue& dest)
{
    T value;
    std::stringstream ss(str);
    if (!(ss >> value))
    {
        throw ExceptionRuntimeError("Failed parsing value from string: " + str);
    }
    dest = RtValue(value);
}
void fromStringFloat(const string& str, float& value)
{
    std::stringstream ss(str);
    if (!(ss >> value))
    {
        throw ExceptionRuntimeError("Failed parsing value from string: " + str);
    }
}
template<class T>
void fromStringVector(const string& str, T& dest)
{
    StringVec tokens = splitString(str, ARRAY_VALID_SEPARATORS);
    if (tokens.size() != T::numElements())
    {
        throw ExceptionRuntimeError("Failed parsing value from string: " + str);
    }
    for (size_t i = 0; i < T::numElements(); ++i)
    {
        fromStringFloat(tokens[i], dest[i]);
    }
}
template<class T>
void fromStringMatrix(const string& str, T& dest)
{
    StringVec tokens = splitString(str, ARRAY_VALID_SEPARATORS);
    if (tokens.size() != T::numRows() * T::numColumns())
    {
        throw ExceptionRuntimeError("Failed parsing value from string: " + str);
    }
    for (size_t i = 0; i < T::numColumns(); ++i)
    {
        for (size_t j = 0; j < T::numRows(); ++j)
        {
            fromStringFloat(tokens[i * T::numRows() + j], dest[i][j]);
        }
    }
}
template<> void fromStringValue<bool>(const string& str, RtValue& dest)
{
    if (str == VALUE_STRING_TRUE)
        dest.asBool() = true;
    else if (str == VALUE_STRING_FALSE)
        dest.asBool() = false;
    else
        throw ExceptionRuntimeError("Failed setting value from string: " + str);
}
template<> void fromStringValue<Color2>(const string& str, RtValue& dest)
{
    fromStringVector(str, dest.asColor2());
}
template<> void fromStringValue<Color3>(const string& str, RtValue& dest)
{
    fromStringVector(str, dest.asColor3());
}
template<> void fromStringValue<Color4>(const string& str, RtValue& dest)
{
    fromStringVector(str, dest.asColor4());
}
template<> void fromStringValue<Vector2>(const string& str, RtValue& dest)
{
    fromStringVector(str, dest.asVector2());
}
template<> void fromStringValue<Vector3>(const string& str, RtValue& dest)
{
    fromStringVector(str, dest.asVector3());
}
template<> void fromStringValue<Vector4>(const string& str, RtValue& dest)
{
    fromStringVector(str, dest.asVector4());
}
template<> void fromStringValue<Matrix33>(const string& str, RtValue& dest)
{
    fromStringMatrix(str, dest.asMatrix33());
}
template<> void fromStringValue<Matrix44>(const string& str, RtValue& dest)
{
    fromStringMatrix(str, dest.asMatrix44());
}
template<> void fromStringValue<string>(const string& str, RtValue& dest)
{
    dest.asString() = str;
}
template<> void fromStringValue<RtToken>(const string& str, RtValue& dest)
{
    dest.asToken() = str;
}
void fromStringNoneValue(const string&, RtValue& dest)
{
    dest = RtValue(0);
}

}

PvtTypeDef::PvtTypeDef(const RtToken& name, const RtToken& basetype, const RtValueFuncs& funcs, 
                       const RtToken& semantic, size_t size) :
    _name(name),
    _basetype(basetype),
    _funcs(funcs),
    _semantic(semantic),
    _size(size)
{
    if (size > 1)
    {
        // An aggregate type so reserve space for the components.
        _components.resize(size);
    }

    // TODO: Handle other types in connections
    _connectionTypes.insert(name);
}

PvtTypeDefRegistry::PvtTypeDefRegistry()
{
    // Register all default types.

    RtValueFuncs boolFuncs = { createValue<bool>, copyValue<bool>, toStringValue<bool>, fromStringValue<bool>  };
    newType("boolean", RtTypeDef::BASETYPE_BOOLEAN, boolFuncs);

    RtValueFuncs intFuncs = { createValue<int>, copyValue<int>, toStringValue<int>, fromStringValue<int> };
    newType("integer", RtTypeDef::BASETYPE_INTEGER, intFuncs);

    RtValueFuncs floatFuncs = { createValue<float>, copyValue<float>,  toStringValue<float>, fromStringValue<float> };
    newType("float", RtTypeDef::BASETYPE_FLOAT, floatFuncs);

    RtValueFuncs color2Funcs = { createValue<Color2>, copyValue<Color2>, toStringValue<Color2> , fromStringValue<Color2> };
    RtTypeDef* color2 = newType("color2", RtTypeDef::BASETYPE_FLOAT, color2Funcs, RtTypeDef::SEMANTIC_NONE, 2);
    color2->setComponent(0, "r", RtTypeDef::BASETYPE_FLOAT);
    color2->setComponent(1, "a", RtTypeDef::BASETYPE_FLOAT);

    RtValueFuncs color3Funcs = { createValue<Color3>, copyValue<Color3>, toStringValue<Color3> , fromStringValue<Color3> };
    RtTypeDef* color3 = newType("color3", RtTypeDef::BASETYPE_FLOAT, color3Funcs, RtTypeDef::SEMANTIC_COLOR, 3);
    color3->setComponent(0, "r", RtTypeDef::BASETYPE_FLOAT);
    color3->setComponent(1, "g", RtTypeDef::BASETYPE_FLOAT);
    color3->setComponent(2, "b", RtTypeDef::BASETYPE_FLOAT);

    RtValueFuncs color4Funcs = { createValue<Color4>, copyValue<Color4>, toStringValue<Color4> , fromStringValue<Color4> };
    RtTypeDef* color4 = newType("color4", RtTypeDef::BASETYPE_FLOAT, color4Funcs, RtTypeDef::SEMANTIC_COLOR, 4);
    color4->setComponent(0, "r", RtTypeDef::BASETYPE_FLOAT);
    color4->setComponent(1, "g", RtTypeDef::BASETYPE_FLOAT);
    color4->setComponent(2, "b", RtTypeDef::BASETYPE_FLOAT);
    color4->setComponent(3, "a", RtTypeDef::BASETYPE_FLOAT);

    RtValueFuncs vector2Funcs = { createValue<Vector2>, copyValue<Vector2>, toStringValue<Vector2> , fromStringValue<Vector2> };
    RtTypeDef* vector2 = newType("vector2", RtTypeDef::BASETYPE_FLOAT, vector2Funcs, RtTypeDef::SEMANTIC_VECTOR, 2);
    vector2->setComponent(0, "x", RtTypeDef::BASETYPE_FLOAT);
    vector2->setComponent(1, "y", RtTypeDef::BASETYPE_FLOAT);

    RtValueFuncs vector3Funcs = { createValue<Vector3>, copyValue<Vector3>, toStringValue<Vector3> , fromStringValue<Vector3> };
    RtTypeDef* vector3 = newType("vector3", RtTypeDef::BASETYPE_FLOAT, vector3Funcs, RtTypeDef::SEMANTIC_VECTOR, 3);
    vector3->setComponent(0, "x", RtTypeDef::BASETYPE_FLOAT);
    vector3->setComponent(1, "y", RtTypeDef::BASETYPE_FLOAT);
    vector3->setComponent(2, "z", RtTypeDef::BASETYPE_FLOAT);

    RtValueFuncs vector4Funcs = { createValue<Vector4>, copyValue<Vector4>, toStringValue<Vector4> , fromStringValue<Vector4> };
    RtTypeDef* vector4 = newType("vector4", RtTypeDef::BASETYPE_FLOAT, vector4Funcs, RtTypeDef::SEMANTIC_VECTOR, 4);
    vector4->setComponent(0, "x", RtTypeDef::BASETYPE_FLOAT);
    vector4->setComponent(1, "y", RtTypeDef::BASETYPE_FLOAT);
    vector4->setComponent(2, "z", RtTypeDef::BASETYPE_FLOAT);
    vector4->setComponent(3, "w", RtTypeDef::BASETYPE_FLOAT);

    RtValueFuncs matrix33Funcs = { createValue<Matrix33>, copyValue<Matrix33>, toStringValue<Matrix33> , fromStringValue<Matrix33> };
    newType("matrix33", RtTypeDef::BASETYPE_FLOAT, matrix33Funcs, RtTypeDef::SEMANTIC_MATRIX, 9);

    RtValueFuncs matrix44Funcs = { createValue<Matrix44>, copyValue<Matrix44>, toStringValue<Matrix44> , fromStringValue<Matrix44> };
    newType("matrix44", RtTypeDef::BASETYPE_FLOAT, matrix44Funcs, RtTypeDef::SEMANTIC_MATRIX, 16);

    RtValueFuncs stringFuncs = { createValue<string>, copyValue<string>, toStringValue<string> , fromStringValue<string> };
    newType("string", RtTypeDef::BASETYPE_STRING, stringFuncs);
    newType("filename", RtTypeDef::BASETYPE_STRING, stringFuncs, RtTypeDef::SEMANTIC_FILENAME);

    RtValueFuncs tokenFuncs = { createValue<RtToken>, copyValue<RtToken>, toStringValue<RtToken> , fromStringValue<RtToken> };
    newType("token", RtTypeDef::BASETYPE_STRING, tokenFuncs);

    newType("integerarray", RtTypeDef::BASETYPE_INTEGER, intFuncs, RtTypeDef::SEMANTIC_NONE, 0);
    newType("floatarray", RtTypeDef::BASETYPE_FLOAT, floatFuncs, RtTypeDef::SEMANTIC_NONE, 0);
    newType("color2array", RtTypeDef::BASETYPE_FLOAT, color2Funcs, RtTypeDef::SEMANTIC_NONE, 0);
    newType("color3array", RtTypeDef::BASETYPE_FLOAT, color3Funcs, RtTypeDef::SEMANTIC_COLOR, 0);
    newType("color4array", RtTypeDef::BASETYPE_FLOAT, color4Funcs, RtTypeDef::SEMANTIC_COLOR, 0);
    newType("vector2array", RtTypeDef::BASETYPE_FLOAT, vector2Funcs, RtTypeDef::SEMANTIC_VECTOR, 0);
    newType("vector3array", RtTypeDef::BASETYPE_FLOAT, vector3Funcs, RtTypeDef::SEMANTIC_VECTOR, 0);
    newType("vector4array", RtTypeDef::BASETYPE_FLOAT, vector4Funcs, RtTypeDef::SEMANTIC_VECTOR, 0);
    newType("stringarray", RtTypeDef::BASETYPE_STRING, stringFuncs, RtTypeDef::SEMANTIC_NONE, 0);

    RtValueFuncs noneFuncs = { createNoneValue, copyNoneValue, toStringNoneValue, fromStringNoneValue };
    newType("BSDF", RtTypeDef::BASETYPE_NONE, noneFuncs, RtTypeDef::SEMANTIC_CLOSURE);
    newType("EDF", RtTypeDef::BASETYPE_NONE, noneFuncs, RtTypeDef::SEMANTIC_CLOSURE);
    newType("VDF", RtTypeDef::BASETYPE_NONE, noneFuncs, RtTypeDef::SEMANTIC_CLOSURE);
    newType("surfaceshader", RtTypeDef::BASETYPE_NONE, noneFuncs, RtTypeDef::SEMANTIC_SHADER);
    newType("volumeshader", RtTypeDef::BASETYPE_NONE, noneFuncs, RtTypeDef::SEMANTIC_SHADER);
    newType("displacementshader", RtTypeDef::BASETYPE_NONE, noneFuncs, RtTypeDef::SEMANTIC_SHADER);
    newType("lightshader", RtTypeDef::BASETYPE_NONE, noneFuncs, RtTypeDef::SEMANTIC_SHADER);

    newType("none", RtTypeDef::BASETYPE_NONE, noneFuncs);
    newType("auto", RtTypeDef::BASETYPE_NONE, noneFuncs);
}

RtTypeDef* PvtTypeDefRegistry::newType(const RtToken& name, const RtToken& basetype, const RtValueFuncs& funcs,
                                       const RtToken& sematic, size_t size)
{
    _types.push_back(std::unique_ptr<RtTypeDef>(new RtTypeDef(name, basetype, funcs, sematic, size)));

    RtTypeDef* ptr = _types.back().get();
    _typesByName[name] = ptr;

    return ptr;
}

}
