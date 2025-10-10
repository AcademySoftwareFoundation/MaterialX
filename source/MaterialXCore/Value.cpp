//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Document.h>
#include <MaterialXCore/Value.h>

#include <iomanip>
#include <sstream>
#include <type_traits>

MATERIALX_NAMESPACE_BEGIN

Value::CreatorMap Value::_creatorMap;

namespace
{

thread_local Value::FloatFormat _floatFormat = Value::FloatFormatDefault;
thread_local int _floatPrecision = 6;

template <class T> inline constexpr bool is_std_vector_v = false;
template <class T> inline constexpr bool is_std_vector_v<vector<T>> = true;

} // anonymous namespace

//
// Global functions
//

template <class T> const string& getTypeString()
{
    return TypedValue<T>::TYPE;
}

template <class T> string toValueString(const T& data)
{
    string str;
    
    if constexpr(std::is_same_v<T, string>)
    {
        str = data;
    }
    else if constexpr(std::is_same_v<T, bool>)
    {
        str = data ? VALUE_STRING_TRUE : VALUE_STRING_FALSE;
    }
    else if constexpr(std::is_base_of_v<VectorBase, T>)
    {
        for (size_t i = 0; i < data.numElements(); i++)
        {
            str += toValueString(data[i]);
            if (i + 1 < data.numElements())
            {
                str += ARRAY_PREFERRED_SEPARATOR;
            }
        }
    }
    else if constexpr(std::is_base_of_v<MatrixBase, T>)
    {
        for (size_t i = 0; i < data.numRows(); i++)
        {
            for (size_t j = 0; j < data.numColumns(); j++)
            {
                str += toValueString(data[i][j]);
                if (i + 1 < data.numRows() ||
                    j + 1 < data.numColumns())
                {
                    str += ARRAY_PREFERRED_SEPARATOR;
                }
            }
        }
    }
    else if constexpr(is_std_vector_v<T>)
    {
        for (size_t i = 0; i < data.size(); i++)
        {
            str += toValueString<typename T::value_type>(data[i]);
            if (i + 1 < data.size())
            {
                str += ARRAY_PREFERRED_SEPARATOR;
            }
        }
    }
    else
    {
        std::stringstream ss;
        ss.imbue(std::locale::classic());

        // Set float format and precision for the stream
        const Value::FloatFormat fmt = Value::getFloatFormat();
        ss.setf(std::ios_base::fmtflags(
                (fmt == Value::FloatFormatFixed ? std::ios_base::fixed :
                (fmt == Value::FloatFormatScientific ? std::ios_base::scientific : 0))),
            std::ios_base::floatfield);
        ss.precision(Value::getFloatPrecision());

        ss << data;
        str = ss.str();
    }
    
    return str;
}

template <class T> T fromValueString(const string& value)
{
    T data;
    
    if constexpr(std::is_same_v<T, string>)
    {
        data = value;
    }
    else if constexpr(std::is_same_v<T, bool>)
    {
        if (value == VALUE_STRING_TRUE)
            data = true;
        else if (value == VALUE_STRING_FALSE)
            data = false;
        else
            throw ExceptionTypeError("Type mismatch in boolean fromValueString: " + value);
    }
    else if constexpr(std::is_base_of_v<VectorBase, T>)
    {
        StringVec tokens = splitString(value, ARRAY_VALID_SEPARATORS);
        if (tokens.size() != data.numElements())
        {
            throw ExceptionTypeError("Type mismatch in vector fromValueString: " + value);
        }
        for (size_t i = 0; i < data.numElements(); i++)
        {
            data[i] = fromValueString<float>(tokens[i]);
        }
    }
    else if constexpr(std::is_base_of_v<MatrixBase, T>)
    {
        StringVec tokens = splitString(value, ARRAY_VALID_SEPARATORS);
        if (tokens.size() != data.numRows() * data.numColumns())
        {
            throw ExceptionTypeError("Type mismatch in matrix fromValueString: " + value);
        }
        for (size_t i = 0; i < data.numRows(); i++)
        {
            for (size_t j = 0; j < data.numColumns(); j++)
            {
                data[i][j] = fromValueString<float>(tokens[i * data.numRows() + j]);
            }
        }
    }
    else if constexpr(is_std_vector_v<T>)
    {
        // This code path parses an array of arbitrary substrings, so we split the string
        // in a fashion that preserves substrings with internal spaces.
        const string COMMA_SEPARATOR = ",";
        for (const string& token : splitString(value, COMMA_SEPARATOR))
        {
            typename T::value_type val = fromValueString<typename T::value_type>(trimSpaces(token));
            data.push_back(val);
        }
    }
    else
    {
        std::stringstream ss(value);
        ss.imbue(std::locale::classic());
        if (!(ss >> data))
        {
            throw ExceptionTypeError("Type mismatch in generic fromValueString: " + value);
        }
    }
    
    return data;
}

StringVec parseStructValueString(const string& value)
{
    static const char SEPARATOR = ';';
    static const char OPEN_BRACE = '{';
    static const char CLOSE_BRACE = '}';

    if (value.empty())
        return StringVec();

    // Validate the string is correctly formatted - must be at least 2 characters long and start and end with braces
    if (value.size() < 2 || (value[0] != OPEN_BRACE || value[value.size()-1] != CLOSE_BRACE))
    {
        return StringVec();
    }

    StringVec split;

    // Strip off the surrounding braces
    string substring = value.substr(1, value.size()-2);

    // Sequentially examine each character to parse the list initializer.
    string part = "";
    int braceDepth = 0;
    for (const char c : substring)
    {
        if (c == OPEN_BRACE)
        {
            // We've already trimmed the starting brace, so any additional braces indicate members that are themselves list initializers.
            // We will just return this as a string of the list initializer.
            braceDepth += 1;
        }
        if (braceDepth > 0 && c == CLOSE_BRACE)
        {
            braceDepth -= 1;
        }

        if (braceDepth == 0 && c == SEPARATOR)
        {
            // When we hit a separator we store the currently accumulated part, and clear to start collecting the next.
            split.emplace_back(part);
            part.clear();
        }
        else
        {
            part += c;
        }
    }

    if (!part.empty())
        split.emplace_back(part);

    return split;
}

//
// TypedValue methods
//

template <class T> ValuePtr TypedValue<T>::createFromString(const string& value)
{
    try
    {
        return Value::createValue<T>(fromValueString<T>(value));
    }
    catch (ExceptionTypeError&)
    {
    }
    return ValuePtr();
}

//
// Value methods
//

void Value::setFloatFormat(FloatFormat format)
{
    _floatFormat = format;
}

void Value::setFloatPrecision(int precision)
{
    _floatPrecision = precision;
}

Value::FloatFormat Value::getFloatFormat()
{
    return _floatFormat;
}

int Value::getFloatPrecision()
{
    return _floatPrecision;
}

ValuePtr Value::createValueFromStrings(const string& value, const string& type, ConstTypeDefPtr typeDef)
{
    CreatorMap::iterator it = _creatorMap.find(type);
    if (it != _creatorMap.end())
        return it->second(value);

    if (typeDef && !typeDef->getMembers().empty())
    {
        // If we're given a TypeDef pointer that has child members, then we can create a new AggregateValue.
        return AggregateValue::createAggregateValueFromString(value, type, typeDef);
    }

    return TypedValue<string>::createFromString(value);
}

template <class T> bool Value::isA() const
{
    return dynamic_cast<const TypedValue<T>*>(this) != nullptr;
}

template <class T> const T& Value::asA() const
{
    const TypedValue<T>* typedVal = dynamic_cast<const TypedValue<T>*>(this);
    if (!typedVal)
    {
        throw ExceptionTypeError("Incorrect type specified for value");
    }
    return typedVal->getData();
}

template <>
MX_CORE_API bool Value::isA<AggregateValue>() const
{
    return dynamic_cast<const AggregateValue*>(this) != nullptr;
}

template <>
MX_CORE_API const AggregateValue& Value::asA<AggregateValue>() const
{
    const AggregateValue* typedVal = dynamic_cast<const AggregateValue*>(this);
    if (!typedVal)
    {
        throw ExceptionTypeError("Incorrect type specified for value");
    }
    return *typedVal;
}

string AggregateValue::getValueString() const
{
    if (_data.empty())
        return EMPTY_STRING;

    std::string result = "{";
    std::string separator = "";
    for (const auto& val : _data)
    {
        result += separator + val->getValueString();
        separator = ";";
    }
    result += "}";

    return result;
}

bool AggregateValue::isEqual(ConstValuePtr other) const
{
    if (!other->isA<AggregateValue>())
    {
        return false;
    }

    const AggregateValue& otherAggregate = other->asA<AggregateValue>();

    size_t dataSize = _data.size();
    size_t otherDataSize = otherAggregate._data.size();

    if (dataSize != otherDataSize)
    {
        return false;
    }

    for (size_t i = 0; i < dataSize; i++)
    {
        if (!_data[i]->isEqual(otherAggregate._data[i]))
        {
            return false;
        }
    }

    return true;
}

AggregateValuePtr AggregateValue::createAggregateValueFromString(const string& value, const string& type, ConstTypeDefPtr typeDef)
{
    StringVec subValues = parseStructValueString(value);

    AggregateValuePtr result = AggregateValue::createAggregateValue(type);
    const auto& members = typeDef->getMembers();

    if (subValues.size() != members.size())
    {
        std::stringstream ss;
        ss << "Wrong number of initializers - expect " << members.size();
        throw Exception(ss.str());
    }

    auto doc = typeDef->getDocument();
    for (size_t i = 0; i < members.size(); ++i)
    {
        const auto& member = members[i];

        // This will return nullptr if the type is not a listed typedef.
        ConstTypeDefPtr subTypeDef = doc->getTypeDef(members[i]->getType());

        // Calling Value::createValueFromStrings() here allows support for recursively nested structs.
        result->appendValue(Value::createValueFromStrings(subValues[i], member->getType(), subTypeDef));
    }

    return result;
}

ScopedFloatFormatting::ScopedFloatFormatting(Value::FloatFormat format, int precision) :
    _format(Value::getFloatFormat()),
    _precision(Value::getFloatPrecision())
{
    Value::setFloatFormat(format);
    if (precision >= 0)
    {
        Value::setFloatPrecision(precision);
    }
}

ScopedFloatFormatting::~ScopedFloatFormatting()
{
    Value::setFloatFormat(_format);
    Value::setFloatPrecision(_precision);
}

//
// Value registry class
//

template <class T> class ValueRegistry
{
  public:
    ValueRegistry()
    {
        if (!Value::_creatorMap.count(TypedValue<T>::TYPE))
        {
            Value::_creatorMap[TypedValue<T>::TYPE] = TypedValue<T>::createFromString;
        }
    }
    ~ValueRegistry() = default;
};

//
// Template instantiations
//

#define INSTANTIATE_TYPE(T, name)                                                                            \
    template <> MX_CORE_API const string TypedValue<T>::TYPE = name;                                         \
    template <> MX_CORE_API const string& TypedValue<T>::getTypeString() const { return TYPE; }              \
    template <> MX_CORE_API string TypedValue<T>::getValueString() const { return toValueString<T>(_data); } \
    template MX_CORE_API bool Value::isA<T>() const;                                                         \
    template MX_CORE_API const T& Value::asA<T>() const;                                                     \
    template MX_CORE_API const string& getTypeString<T>();                                                   \
    template MX_CORE_API string toValueString(const T& data);                                                \
    template MX_CORE_API T fromValueString(const string& value);                                             \
    ValueRegistry<T> registry##T;

// Base types
INSTANTIATE_TYPE(int, "integer")
INSTANTIATE_TYPE(bool, "boolean")
INSTANTIATE_TYPE(float, "float")
INSTANTIATE_TYPE(Color3, "color3")
INSTANTIATE_TYPE(Color4, "color4")
INSTANTIATE_TYPE(Vector2, "vector2")
INSTANTIATE_TYPE(Vector3, "vector3")
INSTANTIATE_TYPE(Vector4, "vector4")
INSTANTIATE_TYPE(Matrix33, "matrix33")
INSTANTIATE_TYPE(Matrix44, "matrix44")
INSTANTIATE_TYPE(string, "string")

// Array types
INSTANTIATE_TYPE(IntVec, "integerarray")
INSTANTIATE_TYPE(BoolVec, "booleanarray")
INSTANTIATE_TYPE(FloatVec, "floatarray")
INSTANTIATE_TYPE(StringVec, "stringarray")

// Alias types
INSTANTIATE_TYPE(long, "integer")
INSTANTIATE_TYPE(double, "float")

MATERIALX_NAMESPACE_END
