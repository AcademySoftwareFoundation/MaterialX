//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PrvTypeDef.h>

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
void marshalValue(const RtValue&, string&)
{
    // TODO: Fix this for gcc/clang
#ifdef _WIN32
    static_assert(false, "marshalValue must be specialized for all types");
#endif
}
template <> void marshalValue<bool>(const RtValue& src, string& dest)
{
    std::stringstream ss;
    ss << src.asBool();
    dest = ss.str();
}
template <> void marshalValue<float>(const RtValue& src, string& dest)
{
    std::stringstream ss;
    ss << src.asFloat();
    dest = ss.str();
}
template <> void marshalValue<int>(const RtValue& src, string& dest)
{
    std::stringstream ss;
    ss << src.asFloat();
    dest = ss.str();
}
template<class T>
void marshalVector(const T& src, string& dest)
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
void marshalMatrix(const T& src, string& dest)
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
template <> void marshalValue<Color2>(const RtValue& src, string& dest)
{
    marshalVector(src.asColor2(), dest);
}
template <> void marshalValue<Color3>(const RtValue& src, string& dest)
{
    marshalVector(src.asColor3(), dest);
}
template <> void marshalValue<Color4>(const RtValue& src, string& dest)
{
    marshalVector(src.asColor4(), dest);
}
template <> void marshalValue<Vector2>(const RtValue& src, string& dest)
{
    marshalVector(src.asVector2(), dest);
}
template <> void marshalValue<Vector3>(const RtValue& src, string& dest)
{
    marshalVector(src.asVector3(), dest);
}
template <> void marshalValue<Vector4>(const RtValue& src, string& dest)
{
    marshalVector(src.asVector4(), dest);
}
template <> void marshalValue<Matrix33>(const RtValue& src, string& dest)
{
    marshalMatrix(src.asMatrix33(), dest);
}
template <> void marshalValue<Matrix44>(const RtValue& src, string& dest)
{
    marshalMatrix(src.asMatrix44(), dest);
}
template <> void marshalValue<string>(const RtValue& src, string& dest)
{
    dest = src.asString();
}
template <> void marshalValue<RtToken>(const RtValue& src, string& dest)
{
    dest = src.asToken().str();
}
void marshalNoneValue(const RtValue&, string& dest)
{
    dest = "";
}

template<class T>
void unmarshalValue(const string& str, RtValue& dest)
{
    T value;
    std::stringstream ss(str);
    if (!(ss >> value))
    {
        throw ExceptionRuntimeError("Failed parsing value from string: " + str);
    }
    dest = RtValue(value);
}
void unmarshalFloat(const string& str, float& value)
{
    std::stringstream ss(str);
    if (!(ss >> value))
    {
        throw ExceptionRuntimeError("Failed parsing value from string: " + str);
    }
}
template<class T>
void unmarshalVector(const string& str, T& dest)
{
    StringVec tokens = splitString(str, ARRAY_VALID_SEPARATORS);
    if (tokens.size() != T::numElements())
    {
        throw ExceptionRuntimeError("Failed parsing value from string: " + str);
    }
    for (size_t i = 0; i < T::numElements(); ++i)
    {
        unmarshalFloat(tokens[i], dest[i]);
    }
}
template<class T>
void unmarshalMatrix(const string& str, T& dest)
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
            unmarshalFloat(tokens[i * T::numRows() + j], dest[i][j]);
        }
    }
}
template<> void unmarshalValue<bool>(const string& str, RtValue& dest)
{
    if (str == VALUE_STRING_TRUE)
        dest.asBool() = true;
    else if (str == VALUE_STRING_FALSE)
        dest.asBool() = false;
    else
        throw ExceptionRuntimeError("Failed setting value from string: " + str);
}
template<> void unmarshalValue<Color2>(const string& str, RtValue& dest)
{
    unmarshalVector(str, dest.asColor2());
}
template<> void unmarshalValue<Color3>(const string& str, RtValue& dest)
{
    unmarshalVector(str, dest.asColor3());
}
template<> void unmarshalValue<Color4>(const string& str, RtValue& dest)
{
    unmarshalVector(str, dest.asColor4());
}
template<> void unmarshalValue<Vector2>(const string& str, RtValue& dest)
{
    unmarshalVector(str, dest.asVector2());
}
template<> void unmarshalValue<Vector3>(const string& str, RtValue& dest)
{
    unmarshalVector(str, dest.asVector3());
}
template<> void unmarshalValue<Vector4>(const string& str, RtValue& dest)
{
    unmarshalVector(str, dest.asVector4());
}
template<> void unmarshalValue<Matrix33>(const string& str, RtValue& dest)
{
    unmarshalMatrix(str, dest.asMatrix33());
}
template<> void unmarshalValue<Matrix44>(const string& str, RtValue& dest)
{
    unmarshalMatrix(str, dest.asMatrix44());
}
template<> void unmarshalValue<string>(const string& str, RtValue& dest)
{
    dest.asString() = str;
}
template<> void unmarshalValue<RtToken>(const string& str, RtValue& dest)
{
    dest.asToken() = str;
}
void unmarshalNoneValue(const string&, RtValue& dest)
{
    dest = RtValue(0);
}

}

PrvTypeDef::PrvTypeDef(const RtToken& name, const RtToken& basetype, const RtValueFuncs& funcs, const RtToken& semantic,
    size_t size, const ChannelMap& channelMap) :
    _name(name),
    _basetype(basetype),
    _funcs(funcs),
    _semantic(semantic),
    _size(size),
    _channelMap(channelMap)
{
    // TODO: Handle other types in connections
    _connectionTypes.insert(name);
}

PrvTypeDefRegistry::PrvTypeDefRegistry()
{
    // Register all default types.

    RtValueFuncs boolFuncs = { createValue<bool>, copyValue<bool>, marshalValue<bool>, unmarshalValue<bool>  };
    newType("boolean", RtTypeDef::BASETYPE_BOOLEAN, boolFuncs);

    RtValueFuncs intFuncs = { createValue<int>, copyValue<int>, marshalValue<int>, unmarshalValue<int> };
    newType("integer", RtTypeDef::BASETYPE_INTEGER, intFuncs);

    RtValueFuncs floatFuncs = { createValue<float>, copyValue<float>,  marshalValue<float>, unmarshalValue<float> };
    newType("float", RtTypeDef::BASETYPE_FLOAT, floatFuncs);

    RtValueFuncs color2Funcs = { createValue<Color2>, copyValue<Color2>, marshalValue<Color2> , unmarshalValue<Color2> };
    newType("color2", RtTypeDef::BASETYPE_FLOAT, color2Funcs, RtTypeDef::SEMANTIC_NONE, 2, { {'r', 0}, {'a', 1} });

    RtValueFuncs color3Funcs = { createValue<Color3>, copyValue<Color3>, marshalValue<Color3> , unmarshalValue<Color3> };
    newType("color3", RtTypeDef::BASETYPE_FLOAT, color3Funcs, RtTypeDef::SEMANTIC_COLOR, 3, { {'r', 0}, {'g', 1}, {'b', 2} });

    RtValueFuncs color4Funcs = { createValue<Color4>, copyValue<Color4>, marshalValue<Color4> , unmarshalValue<Color4> };
    newType("color4", RtTypeDef::BASETYPE_FLOAT, color4Funcs, RtTypeDef::SEMANTIC_COLOR, 4, { {'r', 0}, {'g', 1}, {'b', 2}, {'a', 3} });

    RtValueFuncs vector2Funcs = { createValue<Vector2>, copyValue<Vector2>, marshalValue<Vector2> , unmarshalValue<Vector2> };
    newType("vector2", RtTypeDef::BASETYPE_FLOAT, vector2Funcs, RtTypeDef::SEMANTIC_VECTOR, 2, { {'x', 0}, {'y', 1} });

    RtValueFuncs vector3Funcs = { createValue<Vector3>, copyValue<Vector3>, marshalValue<Vector3> , unmarshalValue<Vector3> };
    newType("vector3", RtTypeDef::BASETYPE_FLOAT, vector3Funcs, RtTypeDef::SEMANTIC_VECTOR, 3, { {'x', 0}, {'y', 1}, {'z', 2} });

    RtValueFuncs vector4Funcs = { createValue<Vector4>, copyValue<Vector4>, marshalValue<Vector4> , unmarshalValue<Vector4> };
    newType("vector4", RtTypeDef::BASETYPE_FLOAT, vector4Funcs, RtTypeDef::SEMANTIC_VECTOR, 4, { {'x', 0}, {'y', 1}, {'z', 2}, {'w', 3} });

    RtValueFuncs matrix33Funcs = { createValue<Matrix33>, copyValue<Matrix33>, marshalValue<Matrix33> , unmarshalValue<Matrix33> };
    newType("matrix33", RtTypeDef::BASETYPE_FLOAT, matrix33Funcs, RtTypeDef::SEMANTIC_MATRIX, 9);

    RtValueFuncs matrix44Funcs = { createValue<Matrix44>, copyValue<Matrix44>, marshalValue<Matrix44> , unmarshalValue<Matrix44> };
    newType("matrix44", RtTypeDef::BASETYPE_FLOAT, matrix44Funcs, RtTypeDef::SEMANTIC_MATRIX, 16);

    RtValueFuncs stringFuncs = { createValue<string>, copyValue<string>, marshalValue<string> , unmarshalValue<string> };
    newType("string", RtTypeDef::BASETYPE_STRING, stringFuncs);
    newType("filename", RtTypeDef::BASETYPE_STRING, stringFuncs, RtTypeDef::SEMANTIC_FILENAME);

    RtValueFuncs tokenFuncs = { createValue<RtToken>, copyValue<RtToken>, marshalValue<RtToken> , unmarshalValue<RtToken> };
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

    RtValueFuncs noneFuncs = { createNoneValue, copyNoneValue, marshalNoneValue, unmarshalNoneValue };
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

RtTypeDef* PrvTypeDefRegistry::newType(const RtToken& name, const RtToken& basetype, const RtValueFuncs& funcs,
    const RtToken& sematic, size_t size, const PrvTypeDef::ChannelMap& channelMapping)
{
    _types.push_back(std::unique_ptr<RtTypeDef>(new RtTypeDef(name, basetype, funcs, sematic, size)));

    RtTypeDef* ptr = _types.back().get();
    _typesByName[name] = ptr;

    for (auto it : channelMapping)
    {
        ptr->setChannelIndex(it.first, it.second);
    }

    return ptr;
}

}
