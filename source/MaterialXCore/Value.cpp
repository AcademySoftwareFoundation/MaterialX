//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Value.h>

#include <MaterialXCore/Util.h>

#include <sstream>

namespace MaterialX
{

Value::CreatorMap Value::_creatorMap;

namespace {

//
// Stream operators
//

template <size_t N> std::istream& operator>>(std::istream& is, VectorN<N>& v)
{
    string str(std::istreambuf_iterator<char>(is), { });
    vector<string> values = splitString(str, ARRAY_VALID_SEPARATORS);
    if (values.size() == N)
    {
        for (size_t i = 0; i < N; i++)
        {
            v[i] = fromValueString<float>(values[i]);
        }
    }
    else
    {
        is.setstate(std::ios::failbit);
    }
    return is;
}

template <size_t N> std::ostream& operator<<(std::ostream& os, const VectorN<N>& v)
{
    for (size_t i = 0; i < N; i++)
    {
        os << v[i];
        if (i + 1 < N)
        {
            os << ARRAY_PREFERRED_SEPARATOR;
        }
    }
    return os;
}

template <class T> std::istream& operator>>(std::istream& is, vector<T>& v)
{
    string str(std::istreambuf_iterator<char>(is), { });
    for (const string& value : splitString(str, ARRAY_VALID_SEPARATORS))
    {
        v.push_back(fromValueString<T>(value));
    }
    return is;
}

template <class T> std::ostream& operator<<(std::ostream& os, const vector<T>& v)
{
    for (size_t i = 0; i < v.size(); i++)
    {
        os << toValueString<T>(v[i]);
        if (i + 1 < v.size())
        {
            os << ARRAY_PREFERRED_SEPARATOR;
        }
    }
    return os;
}

} // anonymous namespace

//
// TypedValue methods
//

template <class T> const string& TypedValue<T>::getTypeString() const
{
    return TYPE;
}

template <class T> string TypedValue<T>::getValueString() const
{
    return toValueString<T>(_data);
}

template <class T> ValuePtr TypedValue<T>::createFromString(const string& value)
{
    return Value::createValue<T>(fromValueString<T>(value));
}

//
// Value methods
//

ValuePtr Value::createValueFromStrings(const string& value, const string& type)
{
    CreatorMap::iterator it = _creatorMap.find(type);
    if (it != _creatorMap.end())
        return it->second(value);

    return TypedValue<string>::createFromString(value);
}

template<class T> bool Value::isA() const
{
    return dynamic_cast<const TypedValue<T>*>(this) != nullptr;    
}

template<class T> T Value::asA() const
{
    const TypedValue<T>* typedVal = dynamic_cast<const TypedValue<T>*>(this);
    if (!typedVal)
    {
        throw Exception("Incorrect type specified for value");
    }
    return typedVal->getData();
}

//
// Global functions
//

template<class T> const string& getTypeString()
{
    return TypedValue<T>::TYPE;
}

template <> string toValueString(const bool& data)
{
    return data ? VALUE_STRING_TRUE : VALUE_STRING_FALSE;
}

template <> string toValueString(const string& data)
{
    return data;
}

template <class T> string toValueString(const T& data)
{
    std::stringstream ss;
    ss << data;
    return ss.str();
}

template <> bool fromValueString(const string& value)
{
    if (value == VALUE_STRING_TRUE)
        return true;
    if (value == VALUE_STRING_FALSE)
        return false;
    throw Exception("Type mismatch in boolean fromValueString: " + value);
}

template <> string fromValueString(const string& value)
{
    return value;
}

template <class T> T fromValueString(const string& value)
{
    std::stringstream ss(value);
    T data;
    if (ss >> data)
        return data;
    throw Exception("Type mismatch in fromValueString: " + value);
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
    ~ValueRegistry() { }
};

//
// Template instantiations
//

#define INSTANTIATE_TYPE(T, name)                       \
template <> const string TypedValue<T>::TYPE = name;    \
template bool Value::isA<T>() const;                    \
template T Value::asA<T>() const;                       \
template const string& getTypeString<T>();              \
ValueRegistry<T> registry##T;

// Base types
INSTANTIATE_TYPE(int, "integer")
INSTANTIATE_TYPE(bool, "boolean")
INSTANTIATE_TYPE(float, "float")
INSTANTIATE_TYPE(Color2, "color2")
INSTANTIATE_TYPE(Color3, "color3")
INSTANTIATE_TYPE(Color4, "color4")
INSTANTIATE_TYPE(Vector2, "vector2")
INSTANTIATE_TYPE(Vector3, "vector3")
INSTANTIATE_TYPE(Vector4, "vector4")
INSTANTIATE_TYPE(Matrix3x3, "matrix33")
INSTANTIATE_TYPE(Matrix4x4, "matrix44")
INSTANTIATE_TYPE(string, "string")

// Array types
INSTANTIATE_TYPE(IntVec, "integerarray")
INSTANTIATE_TYPE(BoolVec, "booleanarray")
INSTANTIATE_TYPE(FloatVec, "floatarray")
INSTANTIATE_TYPE(StringVec, "stringarray")

// Alias types
INSTANTIATE_TYPE(long, "integer")
INSTANTIATE_TYPE(double, "float")

} // namespace MaterialX
