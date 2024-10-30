//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_VALUE_H
#define MATERIALX_VALUE_H

/// @file
/// Generic value classes

#include <MaterialXCore/Exception.h>

#include <MaterialXCore/Types.h>
#include <MaterialXCore/Util.h>

MATERIALX_NAMESPACE_BEGIN

/// A vector of integers.
using IntVec = vector<int>;
/// A vector of booleans.
using BoolVec = vector<bool>;
/// A vector of floats.
using FloatVec = vector<float>;

class Value;
class AggregateValue;

/// A shared pointer to a Value
using ValuePtr = shared_ptr<Value>;
/// A shared pointer to a const Value
using ConstValuePtr = shared_ptr<const Value>;

/// A shared pointer to an Aggregate Value
using AggregateValuePtr = shared_ptr<AggregateValue>;
/// A shared pointer to a const Aggregate Value
using ConstAggregateValuePtr = shared_ptr<const AggregateValue>;

class TypeDef;
using ConstTypeDefPtr = shared_ptr<const TypeDef>;

template <class T> class TypedValue;

/// A generic, discriminated value, whose type may be queried dynamically.
class MX_CORE_API Value
{
  public:
    /// Float formats to use when converting values to strings.
    enum FloatFormat
    {
        FloatFormatDefault = 0,
        FloatFormatFixed = 1,
        FloatFormatScientific = 2
    };

  public:
    Value()
    {
    }
    virtual ~Value() { }

    /// Create a new value from an object of any valid MaterialX type.
    template <class T> static ValuePtr createValue(const T& data)
    {
        return std::make_shared<TypedValue<T>>(data);
    }

    // Create a new value from a C-style string.
    static ValuePtr createValue(const char* data)
    {
        return createValue(data ? string(data) : EMPTY_STRING);
    }

    /// Create a new value instance from value and type strings.
    /// @return A shared pointer to a typed value, or an empty shared pointer
    ///    if the conversion to the given data type cannot be performed.
    static ValuePtr createValueFromStrings(const string& value, const string& type, ConstTypeDefPtr typeDef = nullptr);

    /// Create a deep copy of the value.
    virtual ValuePtr copy() const = 0;

    /// @name Data Accessors
    /// @{

    /// Return true if this value is of the given type.
    template <class T> bool isA() const;

    /// Return our underlying data as an object of the given type.
    /// If the given type doesn't match our own data type, then an
    /// exception is thrown.
    template <class T> const T& asA() const;

    /// Return the type string for this value.
    virtual const string& getTypeString() const = 0;

    /// Return the value string for this value.
    virtual string getValueString() const = 0;

    /// Set float formatting for converting values to strings.
    /// Formats to use are FloatFormatFixed, FloatFormatScientific
    /// or FloatFormatDefault to set default format.
    static void setFloatFormat(FloatFormat format)
    {
        _floatFormat = format;
    }

    /// Set float precision for converting values to strings.
    static void setFloatPrecision(int precision)
    {
        _floatPrecision = precision;
    }

    /// Return the current float format.
    static FloatFormat getFloatFormat()
    {
        return _floatFormat;
    }

    /// Return the current float precision.
    static int getFloatPrecision()
    {
        return _floatPrecision;
    }

  protected:
    template <class T> friend class ValueRegistry;

    using CreatorFunction = ValuePtr (*)(const string&);
    using CreatorMap = std::unordered_map<string, CreatorFunction>;

  private:
    static CreatorMap _creatorMap;
    static FloatFormat _floatFormat;
    static int _floatPrecision;
};

/// The class template for typed subclasses of Value
template <class T> class MX_CORE_API TypedValue : public Value
{
  public:
    TypedValue() :
        _data{}
    {
    }
    explicit TypedValue(const T& value) :
        _data(value)
    {
    }
    virtual ~TypedValue() { }

    /// Create a deep copy of the value.
    ValuePtr copy() const override
    {
        return Value::createValue<T>(_data);
    }

    /// Set stored data object.
    void setData(const T& value)
    {
        _data = value;
    }

    /// Set stored data object.
    void setData(const TypedValue<T>& value)
    {
        _data = value._data;
    }

    /// Return stored data object.
    const T& getData() const
    {
        return _data;
    }

    /// Return type string.
    const string& getTypeString() const override;

    /// Return value string.
    string getValueString() const override;

    //
    // Static helper methods
    //

    /// Create a new value of this type from a value string.
    /// @return A shared pointer to a typed value, or an empty shared pointer
    ///    if the conversion to the given data type cannot be performed.
    static ValuePtr createFromString(const string& value);

  public:
    static const string TYPE;

  private:
    T _data;
};

/// A subclass for aggregate values with multiple members
class MX_CORE_API AggregateValue : public Value
{
  public:
    AggregateValue(const string& typeName) :
        _typeName(typeName)
    {
    }
    virtual ~AggregateValue() { }

    /// Create a deep copy of the value.
    ValuePtr copy() const override
    {
        auto result = createAggregateValue(_typeName);
        for (const auto& val : _data)
        {
            result->appendValue(val->copy());
        }
        return result;
    }

    /// Append a member value to the aggregate.
    void appendValue(ConstValuePtr valuePtr)
    {
        _data.emplace_back(valuePtr);
    }

    const vector<ConstValuePtr>& getMembers() const
    {
        return _data;
    }

    /// Query an indexed member value from the aggregate.
    ConstValuePtr getMemberValue(size_t index) const
    {
        return _data[index];
    }

    /// Return type string.
    const string& getTypeString() const override { return _typeName; }

    /// Return value string.
    string getValueString() const override;

    //
    // Static helper methods
    //

    /// Create a new value from an object of any valid MaterialX type.
    static AggregateValuePtr createAggregateValue(const string& typeName)
    {
        return std::make_shared<AggregateValue>(typeName);
    }

    static AggregateValuePtr createAggregateValueFromString(const string& value, const string& type, ConstTypeDefPtr typeDefPtr);

  private:
    const string _typeName;

    vector<ConstValuePtr> _data;
};

/// @class ScopedFloatFormatting
/// An RAII class for controlling the float formatting of values.
class MX_CORE_API ScopedFloatFormatting
{
  public:
    explicit ScopedFloatFormatting(Value::FloatFormat format, int precision = -1);
    ~ScopedFloatFormatting();

  private:
    Value::FloatFormat _format;
    int _precision;
};

/// Return the type string associated with the given data type.
template <class T> MX_CORE_API const string& getTypeString();

/// Convert the given data value to a value string.
template <class T> MX_CORE_API string toValueString(const T& data);

/// Convert the given value string to a data value of the given type.
/// @throws ExceptionTypeError if the conversion cannot be performed.
template <class T> MX_CORE_API T fromValueString(const string& value);

/// Tokenize the string representation of a struct value i.e, "{1;2;3}" into a
/// vector of substrings.
/// Note: "{1;2;{3;4;5}}" will be split in to ["1", "2", "{3;4;5}"]
MX_CORE_API StringVec parseStructValueString(const string& value);

/// Forward declaration of specific template instantiations.
/// Base types
MX_CORE_EXTERN_TEMPLATE(TypedValue<int>);
MX_CORE_EXTERN_TEMPLATE(TypedValue<bool>);
MX_CORE_EXTERN_TEMPLATE(TypedValue<float>);
MX_CORE_EXTERN_TEMPLATE(TypedValue<Color3>);
MX_CORE_EXTERN_TEMPLATE(TypedValue<Color4>);
MX_CORE_EXTERN_TEMPLATE(TypedValue<Vector2>);
MX_CORE_EXTERN_TEMPLATE(TypedValue<Vector3>);
MX_CORE_EXTERN_TEMPLATE(TypedValue<Vector4>);
MX_CORE_EXTERN_TEMPLATE(TypedValue<Matrix33>);
MX_CORE_EXTERN_TEMPLATE(TypedValue<Matrix44>);
MX_CORE_EXTERN_TEMPLATE(TypedValue<string>);

/// Array types
MX_CORE_EXTERN_TEMPLATE(TypedValue<IntVec>);
MX_CORE_EXTERN_TEMPLATE(TypedValue<BoolVec>);
MX_CORE_EXTERN_TEMPLATE(TypedValue<FloatVec>);
MX_CORE_EXTERN_TEMPLATE(TypedValue<StringVec>);

/// Alias types
MX_CORE_EXTERN_TEMPLATE(TypedValue<long>);
MX_CORE_EXTERN_TEMPLATE(TypedValue<double>);

MATERIALX_NAMESPACE_END

#endif
