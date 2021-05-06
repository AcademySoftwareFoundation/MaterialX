//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTVALUE_H
#define MATERIALX_RTVALUE_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtType.h>

#include <MaterialXCore/Types.h>

namespace MaterialX
{

/// @class RtValue
/// Generic value class for storing values of all the data types
/// supported by the API. Values that fit into 16 bytes of data
/// are stored directly. Values larger than 16 bytes are stored
/// as pointers and lifetime managed by the prim they belong to.
class RtValue
{
public:
    /// Default constructor
    RtValue() : _data{0,0} {}

    /// Explicit value constructor
    explicit RtValue(bool v) { asBool() = v; }
    explicit RtValue(int v) { asInt() = v; }
    explicit RtValue(float v) { asFloat() = v; }
    explicit RtValue(const Color3& v) { asColor3() = v; }
    explicit RtValue(const Color4& v) { asColor4() = v; }
    explicit RtValue(const Vector2& v) { asVector2() = v; }
    explicit RtValue(const Vector3& v) { asVector3() = v; }
    explicit RtValue(const Vector4& v) { asVector4() = v; }
    explicit RtValue(const RtIdentifier& v) { asIdentifier() = v; }

    /// Explicit value constructor for large values.
    /// Allocated data is managed by the given prim.
    explicit RtValue(const Matrix33& v, RtPrim& prim);
    explicit RtValue(const Matrix44& v, RtPrim& prim);
    explicit RtValue(const string& v, RtPrim& prim);

    /// Return bool value.
    const bool& asBool() const
    {
        return *_reinterpret_cast<const bool*>(&_data);
    }
    /// Return reference to bool value.
    bool& asBool()
    {
        return *_reinterpret_cast<bool*>(&_data);
    }

    /// Return int value.
    int asInt() const
    {
        return *_reinterpret_cast<const int*>(&_data);
    }
    /// Return reference to int value.
    int& asInt()
    {
        return *_reinterpret_cast<int*>(&_data);
    }

    /// Return float value.
    float asFloat() const
    {
        return *_reinterpret_cast<const float*>(&_data);
    }
    /// Return reference to float value.
    float& asFloat()
    {
        return *_reinterpret_cast<float*>(&_data);
    }
  
    /// Return Color3 value.
    const Color3& asColor3() const
    {
        return *_reinterpret_cast<const Color3*>(&_data);
    }
    /// Return reference to Color3 value.
    Color3& asColor3()
    {
        return *_reinterpret_cast<Color3*>(&_data);
    }

    /// Return Color4 value.
    const Color4& asColor4() const
    {
        return *_reinterpret_cast<const Color4*>(&_data);
    }
    /// Return reference to Color4 value.
    Color4& asColor4()
    {
        return *_reinterpret_cast<Color4*>(&_data);
    }

    /// Return Vector2 value.
    const Vector2& asVector2() const
    {
        return *_reinterpret_cast<const Vector2*>(&_data);
    }
    /// Return reference to Vector2 value.
    Vector2& asVector2()
    {
        return *_reinterpret_cast<Vector2*>(&_data);
    }

    /// Return Vector3 value.
    const Vector3& asVector3() const
    {
        return *_reinterpret_cast<const Vector3*>(&_data);
    }
    /// Return reference to Vector3 value.
    Vector3& asVector3()
    {
        return *_reinterpret_cast<Vector3*>(&_data);
    }

    /// Return Vector4 value.
    const Vector4& asVector4() const
    {
        return *_reinterpret_cast<const Vector4*>(&_data);
    }
    /// Return reference to Vector4 value.
    Vector4& asVector4()
    {
        return *_reinterpret_cast<Vector4*>(&_data);
    }

    /// Return identifier value.
    const RtIdentifier& asIdentifier() const
    {
        return *_reinterpret_cast<const RtIdentifier*>(&_data);
    }
    /// Return reference to identifier value.
    RtIdentifier& asIdentifier()
    {
        return *_reinterpret_cast<RtIdentifier*>(&_data);
    }

    /// Return Matrix33 value.
    const Matrix33& asMatrix33() const
    {
        return **_reinterpret_cast<const Matrix33* const*>(&_data);
    }
    /// Return reference to Matrix33 value.
    Matrix33& asMatrix33()
    {
        return **_reinterpret_cast<Matrix33**>(&_data);
    }

    /// Return Matrix44 value.
    const Matrix44& asMatrix44() const
    {
        return **_reinterpret_cast<Matrix44* const*>(&_data);
    }
    /// Return reference to Matrix44 value.
    Matrix44& asMatrix44()
    {
        return **_reinterpret_cast<Matrix44**>(&_data);
    }

    /// Return string value.
    const string& asString() const
    {
        return **_reinterpret_cast<string* const*>(&_data);
    }
    /// Return reference to string value.
    string& asString()
    {
        return **_reinterpret_cast<string**>(&_data);
    }

    /// Clear the value with zeores.
    void clear()
    {
        _data[0] = _data[1] = 0;
    }

    /// Equality operator.
    /// NOTE: This operator will only compare the raw data stored.
    /// For a semantic compare which considers the actual type being stored and
    /// handles comparison for large types stored as pointers you should instead
    /// use the method RtValue::compare().
    bool operator==(const RtValue& other) const
    {
        return _data[0] == other._data[0] && _data[1] == other._data[1];
    }

    /// Create a new value of given type.
    /// If the type is a large value the given prim will take
    /// ownership of allocated data.
    static RtValue createNew(const RtIdentifier& type, RtPrim owner);

    /// Clone a value of given type.
    /// If the type is a large value the given prim will take
    /// ownership of allocated data.
    static RtValue clone(const RtIdentifier& type, const RtValue& value, RtPrim owner);

    /// Copy a value from one instance to another.
    /// Both RtValue instances must be initialized for the given type.
    static void copy(const RtIdentifier& type, const RtValue& src, RtValue& dest);

    /// Test if two values are equal.
    /// Both RtValue instances must be initialized for the given type.
    static bool compare(const RtIdentifier& type, const RtValue& a, const RtValue& b);

    /// Convert an RtValue of given type into a string representation.
    static void toString(const RtIdentifier& type, const RtValue& src, string& dest);

    /// Convert a value from a string representation into an RtValue of the given type.
    /// Destination RtValue must been initialized for the given type.
    static void fromString(const RtIdentifier& type, const string& src, RtValue& dest);

private:
    // 16 bytes of data storage to hold the main data types,
    // up to four component vector/color. Larger data types
    // needs to be allocated through the RtValueStore class.
    // Storage is aligned to 64-bit to hold pointers for
    // heap allocated data types as well as other pointers.
    uint64_t _data[2];
};

class RtTypedValue
{
public:
    RtTypedValue() :
        _type(EMPTY_IDENTIFIER)
    {}

    RtTypedValue(const RtIdentifier& t, const RtValue& v) :
        _type(t),
        _value(v)
    {}

    const RtIdentifier& getType() const
    {
        return _type;
    }

    const RtValue& getValue() const
    {
        return _value;
    }

    RtValue& getValue()
    {
        return _value;
    }

    void setValue(const RtValue& v)
    {
        _value = v;
    }

    /// Return a string representation for the value of this attribute.
    string getValueString() const
    {
        string dest;
        RtValue::toString(_type, _value, dest);
        return dest;
    }

    /// Set attribute value from a string representation.
    void setValueString(const string& v)
    {
        RtValue::fromString(_type, v, _value);
    }

    /// Return bool value.
    const bool& asBool() const
    {
#ifndef NDEBUG
        if (_type != RtType::BOOLEAN)
        {
            throw ExceptionRuntimeTypeError("Referencing a value as '" + RtType::BOOLEAN.str() + "' when the value is in fact a '" + _type.str() + "'");
        }
#endif
        return _value .asBool();
    }
    /// Return reference to bool value.
    bool& asBool()
    {
#ifndef NDEBUG
        if (_type != RtType::BOOLEAN)
        {
            throw ExceptionRuntimeTypeError("Referencing a value as '" + RtType::BOOLEAN.str() + "' when the value is in fact a '" + _type.str() + "'");
        }
#endif
        return _value.asBool();
    }

    /// Return int value.
    int asInt() const
    {
#ifndef NDEBUG
        if (_type != RtType::INTEGER)
        {
            throw ExceptionRuntimeTypeError("Referencing a value as '" + RtType::INTEGER.str() + "' when the value is in fact a '" + _type.str() + "'");
        }
#endif
        return _value.asInt();
    }
    /// Return reference to int value.
    int& asInt()
    {
#ifndef NDEBUG
        if (_type != RtType::INTEGER)
        {
            throw ExceptionRuntimeTypeError("Referencing a value as '" + RtType::INTEGER.str() + "' when the value is in fact a '" + _type.str() + "'");
        }
#endif
        return _value.asInt();
    }

    /// Return float value.
    float asFloat() const
    {
#ifndef NDEBUG
        if (_type != RtType::FLOAT)
        {
            throw ExceptionRuntimeTypeError("Referencing a value as '" + RtType::FLOAT.str() + "' when the value is in fact a '" + _type.str() + "'");
        }
#endif
        return _value.asFloat();
    }
    /// Return reference to float value.
    float& asFloat()
    {
#ifndef NDEBUG
        if (_type != RtType::FLOAT)
        {
            throw ExceptionRuntimeTypeError("Referencing a value as '" + RtType::FLOAT.str() + "' when the value is in fact a '" + _type.str() + "'");
        }
#endif
        return _value.asFloat();
    }

    /// Return Color3 value.
    const Color3& asColor3() const
    {
#ifndef NDEBUG
        if (_type != RtType::COLOR3)
        {
            throw ExceptionRuntimeTypeError("Referencing a value as '" + RtType::COLOR3.str() + "' when the value is in fact a '" + _type.str() + "'");
        }
#endif
        return _value.asColor3();
    }
    /// Return reference to Color3 value.
    Color3& asColor3()
    {
#ifndef NDEBUG
        if (_type != RtType::COLOR3)
        {
            throw ExceptionRuntimeTypeError("Referencing a value as '" + RtType::COLOR3.str() + "' when the value is in fact a '" + _type.str() + "'");
        }
#endif
        return _value.asColor3();
    }

    /// Return Color4 value.
    const Color4& asColor4() const
    {
#ifndef NDEBUG
        if (_type != RtType::COLOR4)
        {
            throw ExceptionRuntimeTypeError("Referencing a value as '" + RtType::COLOR4.str() + "' when the value is in fact a '" + _type.str() + "'");
        }
#endif
        return _value.asColor4();
    }
    /// Return reference to Color4 value.
    Color4& asColor4()
    {
#ifndef NDEBUG
        if (_type != RtType::COLOR4)
        {
            throw ExceptionRuntimeTypeError("Referencing a value as '" + RtType::COLOR4.str() + "' when the value is in fact a '" + _type.str() + "'");
        }
#endif
        return _value.asColor4();
    }

    /// Return Vector2 value.
    const Vector2& asVector2() const
    {
#ifndef NDEBUG
        if (_type != RtType::VECTOR2)
        {
            throw ExceptionRuntimeTypeError("Referencing a value as '" + RtType::VECTOR2.str() + "' when the value is in fact a '" + _type.str() + "'");
        }
#endif
        return _value.asVector2();
    }
    /// Return reference to Vector2 value.
    Vector2& asVector2()
    {
#ifndef NDEBUG
        if (_type != RtType::VECTOR2)
        {
            throw ExceptionRuntimeTypeError("Referencing a value as '" + RtType::VECTOR2.str() + "' when the value is in fact a '" + _type.str() + "'");
        }
#endif
        return _value.asVector2();
    }

    /// Return Vector3 value.
    const Vector3& asVector3() const
    {
#ifndef NDEBUG
        if (_type != RtType::VECTOR3)
        {
            throw ExceptionRuntimeTypeError("Referencing a value as '" + RtType::VECTOR3.str() + "' when the value is in fact a '" + _type.str() + "'");
        }
#endif
        return _value.asVector3();
    }
    /// Return reference to Vector3 value.
    Vector3& asVector3()
    {
#ifndef NDEBUG
        if (_type != RtType::VECTOR3)
        {
            throw ExceptionRuntimeTypeError("Referencing a value as '" + RtType::VECTOR3.str() + "' when the value is in fact a '" + _type.str() + "'");
        }
#endif
        return _value.asVector3();
    }

    /// Return Vector4 value.
    const Vector4& asVector4() const
    {
#ifndef NDEBUG
        if (_type != RtType::VECTOR4)
        {
            throw ExceptionRuntimeTypeError("Referencing a value as '" + RtType::VECTOR4.str() + "' when the value is in fact a '" + _type.str() + "'");
        }
#endif
        return _value.asVector4();
    }
    /// Return reference to Vector4 value.
    Vector4& asVector4()
    {
#ifndef NDEBUG
        if (_type != RtType::VECTOR4)
        {
            throw ExceptionRuntimeTypeError("Referencing a value as '" + RtType::VECTOR4.str() + "' when the value is in fact a '" + _type.str() + "'");
        }
#endif
        return _value.asVector4();
    }

    /// Return identifier value.
    const RtIdentifier& asIdentifier() const
    {
#ifndef NDEBUG
        if (_type != RtType::IDENTIFIER)
        {
            throw ExceptionRuntimeTypeError("Referencing a value as '" + RtType::IDENTIFIER.str() + "' when the value is in fact a '" + _type.str() + "'");
        }
#endif
        return _value.asIdentifier();
    }
    /// Return reference to identifier value.
    RtIdentifier& asIdentifier()
    {
#ifndef NDEBUG
        if (_type != RtType::IDENTIFIER)
        {
            throw ExceptionRuntimeTypeError("Referencing a value as '" + RtType::IDENTIFIER.str() + "' when the value is in fact a '" + _type.str() + "'");
        }
#endif
        return _value.asIdentifier();
    }

    /// Return Matrix33 value.
    const Matrix33& asMatrix33() const
    {
#ifndef NDEBUG
        if (_type != RtType::MATRIX33)
        {
            throw ExceptionRuntimeTypeError("Referencing a value as '" + RtType::MATRIX33.str() + "' when the value is in fact a '" + _type.str() + "'");
        }
#endif
        return _value.asMatrix33();
    }
    /// Return reference to Matrix33 value.
    Matrix33& asMatrix33()
    {
#ifndef NDEBUG
        if (_type != RtType::MATRIX33)
        {
            throw ExceptionRuntimeTypeError("Referencing a value as '" + RtType::MATRIX33.str() + "' when the value is in fact a '" + _type.str() + "'");
        }
#endif
        return _value.asMatrix33();
    }

    /// Return Matrix44 value.
    const Matrix44& asMatrix44() const
    {
#ifndef NDEBUG
        if (_type != RtType::MATRIX44)
        {
            throw ExceptionRuntimeTypeError("Referencing a value as '" + RtType::MATRIX44.str() + "' when the value is in fact a '" + _type.str() + "'");
        }
#endif
        return _value.asMatrix44();
    }
    /// Return reference to Matrix44 value.
    Matrix44& asMatrix44()
    {
#ifndef NDEBUG
        if (_type != RtType::MATRIX44)
        {
            throw ExceptionRuntimeTypeError("Referencing a value as '" + RtType::MATRIX44.str() + "' when the value is in fact a '" + _type.str() + "'");
        }
#endif
        return _value.asMatrix44();
    }

    /// Return string value.
    const string& asString() const
    {
#ifndef NDEBUG
        if (_type != RtType::STRING)
        {
            throw ExceptionRuntimeTypeError("Referencing a value as '" + RtType::STRING.str() + "' when the value is in fact a '" + _type.str() + "'");
        }
#endif
        return _value.asString();
    }
    /// Return reference to string value.
    string& asString()
    {
#ifndef NDEBUG
        if (_type != RtType::STRING)
        {
            throw ExceptionRuntimeTypeError("Referencing a value as '" + RtType::STRING.str() + "' when the value is in fact a '" + _type.str() + "'");
        }
#endif
        return _value.asString();
    }

private:
    RtIdentifier _type;
    RtValue _value;
};

}

#endif
