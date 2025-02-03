//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_TYPEDESC_H
#define MATERIALX_TYPEDESC_H

/// @file
/// Type descriptor for a MaterialX data type.

#include <MaterialXGenShader/Export.h>
#include <MaterialXCore/Value.h>

#include <string_view>

MATERIALX_NAMESPACE_BEGIN

/// @class TypeDesc
/// A type descriptor for MaterialX data types.
///
/// All types need to have a type descriptor registered in order for shader generators
/// to know about the type. It can be used for type comparisons as well as getting more
/// information about the type. Type descriptors for all standard library data types are
/// registered by default and can be accessed from the Type namespace, e.g. Type::FLOAT.
///
/// To register custom types use the macro TYPEDESC_DEFINE_TYPE to define it in a header
/// and the macro TYPEDESC_REGISTER_TYPE to register it in the type registry. Registration
/// must be done in order to access the type's name later using getName() and to find the
/// type by name using TypeDesc::get().
///
/// The class is a POD type of 64-bits and can efficiently be stored and passed by value.
/// Type compare operations and hash operations are done using a precomputed hash value.
///
class MX_GENSHADER_API TypeDesc
{
  public:
    enum BaseType
    {
        BASETYPE_NONE,
        BASETYPE_BOOLEAN,
        BASETYPE_INTEGER,
        BASETYPE_FLOAT,
        BASETYPE_STRING,
        BASETYPE_STRUCT,
        BASETYPE_LAST
    };

    enum Semantic
    {
        SEMANTIC_NONE,
        SEMANTIC_COLOR,
        SEMANTIC_VECTOR,
        SEMANTIC_MATRIX,
        SEMANTIC_FILENAME,
        SEMANTIC_CLOSURE,
        SEMANTIC_SHADER,
        SEMANTIC_MATERIAL,
        SEMANTIC_ENUM,
        SEMANTIC_LAST
    };

    /// Empty constructor.
    constexpr TypeDesc() noexcept :
        _id(0),
        _basetype(BASETYPE_NONE),
        _semantic(SEMANTIC_NONE),
        _size(0),
        _structIndex(0)
    {
    }

    /// Constructor.
    constexpr TypeDesc(std::string_view name, uint8_t basetype, uint8_t semantic = SEMANTIC_NONE, uint16_t size = 1, uint16_t structIndex = 0) noexcept :
        _id(constexpr_hash(name)), // Note: We only store the hash to keep the class size minimal.
        _basetype(basetype),
        _semantic(semantic),
        _size(size),
        _structIndex(structIndex)
    {
    }

    /// Return the unique id assigned to this type.
    /// The id is a hash of the given type name.
    uint32_t typeId() const { return _id; }

    /// Return the name of the type.
    const string& getName() const;

    /// Return the basetype for the type.
    unsigned char getBaseType() const { return _basetype; }

    /// Return the semantic for the type.
    unsigned char getSemantic() const { return _semantic; }

    /// Return the number of elements the type is composed of.
    /// Will return 1 for scalar types and a size greater than 1 for aggregate type.
    /// For array types 0 is returned since the number of elements is undefined
    /// until an array is instantiated.
    size_t getSize() const { return _size; }

    /// Return true if the type is a scalar type.
    bool isScalar() const { return _size == 1; }

    /// Return true if the type is an aggregate type.
    bool isAggregate() const { return _size > 1; }

    /// Return true if the type is an array type.
    bool isArray() const { return _size == 0; }

    /// Return true if the type is an aggregate of 2 floats.
    bool isFloat2() const { return _size == 2 && (_semantic == SEMANTIC_COLOR || _semantic == SEMANTIC_VECTOR); }

    /// Return true if the type is an aggregate of 3 floats.
    bool isFloat3() const { return _size == 3 && (_semantic == SEMANTIC_COLOR || _semantic == SEMANTIC_VECTOR); }

    /// Return true if the type is an aggregate of 4 floats.
    bool isFloat4() const { return _size == 4 && (_semantic == SEMANTIC_COLOR || _semantic == SEMANTIC_VECTOR); }

    /// Return true if the type represents a closure.
    bool isClosure() const { return (_semantic == SEMANTIC_CLOSURE || _semantic == SEMANTIC_SHADER || _semantic == SEMANTIC_MATERIAL); }

    /// Return true if the type represents a struct.
    bool isStruct() const { return _basetype == BASETYPE_STRUCT; }

    /// Return the index for the struct member information in StructTypeDesc, the result is invalid if `isStruct()` returns false.
    uint16_t getStructIndex() const { return _structIndex; }

    /// Equality operator
    bool operator==(TypeDesc rhs) const
    {
        return _id == rhs._id;
    }

    /// Inequality operator
    bool operator!=(TypeDesc rhs) const
    {
        return _id != rhs._id;
    }

    /// Less-than operator
    bool operator<(TypeDesc rhs) const
    {
        return _id < rhs._id;
    }

    /// Hash operator
    struct Hasher
    {
        size_t operator()(const TypeDesc& t) const
        {
            return t._id;
        }
    };

    /// Return a type description by name.
    /// If no type is found Type::NONE is returned.
    static TypeDesc get(const string& name);

    /// Remove a type description by name, if it exists.
    static void remove(const string& name);

    static const string NONE_TYPE_NAME;

    /// Create a Value from a string for a given typeDesc
    ValuePtr createValueFromStrings(const string& value) const;

  private:
    /// Simple constexpr hash function, good enough for the small set of short strings that
    /// are used for our data type names.
    constexpr uint32_t constexpr_hash(std::string_view str, uint32_t n = 0, uint32_t h = 2166136261)
    {
        return n == uint32_t(str.size()) ? h : constexpr_hash(str, n + 1, (h * 16777619) ^ (str[n]));
    }

    uint32_t _id;
    uint8_t _basetype;
    uint8_t _semantic;
    uint16_t _size;
    uint16_t _structIndex;
};

/// @class TypeDescRegistry
/// Helper class for type registration.
class MX_GENSHADER_API TypeDescRegistry
{
  public:
    TypeDescRegistry(TypeDesc type, const string& name);
};

/// Macro to define global type descriptions for commonly used types.
#define TYPEDESC_DEFINE_TYPE(T, name, basetype, semantic, size) \
    static constexpr TypeDesc T(name, basetype, semantic, size);

/// Macro to register a previously defined type in the type registry.
/// Registration must be done in order for the type to be searchable by name.
#define TYPEDESC_REGISTER_TYPE(T, name) \
    TypeDescRegistry register_##T(T, name);

namespace Type
{

//
/// Define type descriptors for standard types.
//
TYPEDESC_DEFINE_TYPE(NONE, "none", TypeDesc::BASETYPE_NONE, TypeDesc::SEMANTIC_NONE, 1)
TYPEDESC_DEFINE_TYPE(BOOLEAN, "boolean", TypeDesc::BASETYPE_BOOLEAN, TypeDesc::SEMANTIC_NONE, 1)
TYPEDESC_DEFINE_TYPE(INTEGER, "integer", TypeDesc::BASETYPE_INTEGER, TypeDesc::SEMANTIC_NONE, 1)
TYPEDESC_DEFINE_TYPE(FLOAT, "float", TypeDesc::BASETYPE_FLOAT, TypeDesc::SEMANTIC_NONE, 1)
TYPEDESC_DEFINE_TYPE(INTEGERARRAY, "integerarray", TypeDesc::BASETYPE_INTEGER, TypeDesc::SEMANTIC_NONE, 0)
TYPEDESC_DEFINE_TYPE(FLOATARRAY, "floatarray", TypeDesc::BASETYPE_FLOAT, TypeDesc::SEMANTIC_NONE, 0)
TYPEDESC_DEFINE_TYPE(VECTOR2, "vector2", TypeDesc::BASETYPE_FLOAT, TypeDesc::SEMANTIC_VECTOR, 2)
TYPEDESC_DEFINE_TYPE(VECTOR3, "vector3", TypeDesc::BASETYPE_FLOAT, TypeDesc::SEMANTIC_VECTOR, 3)
TYPEDESC_DEFINE_TYPE(VECTOR4, "vector4", TypeDesc::BASETYPE_FLOAT, TypeDesc::SEMANTIC_VECTOR, 4)
TYPEDESC_DEFINE_TYPE(COLOR3, "color3", TypeDesc::BASETYPE_FLOAT, TypeDesc::SEMANTIC_COLOR, 3)
TYPEDESC_DEFINE_TYPE(COLOR4, "color4", TypeDesc::BASETYPE_FLOAT, TypeDesc::SEMANTIC_COLOR, 4)
TYPEDESC_DEFINE_TYPE(MATRIX33, "matrix33", TypeDesc::BASETYPE_FLOAT, TypeDesc::SEMANTIC_MATRIX, 9)
TYPEDESC_DEFINE_TYPE(MATRIX44, "matrix44", TypeDesc::BASETYPE_FLOAT, TypeDesc::SEMANTIC_MATRIX, 16)
TYPEDESC_DEFINE_TYPE(STRING, "string", TypeDesc::BASETYPE_STRING, TypeDesc::SEMANTIC_NONE, 1)
TYPEDESC_DEFINE_TYPE(FILENAME, "filename", TypeDesc::BASETYPE_STRING, TypeDesc::SEMANTIC_FILENAME, 1)
TYPEDESC_DEFINE_TYPE(BSDF, "BSDF", TypeDesc::BASETYPE_NONE, TypeDesc::SEMANTIC_CLOSURE, 1)
TYPEDESC_DEFINE_TYPE(EDF, "EDF", TypeDesc::BASETYPE_NONE, TypeDesc::SEMANTIC_CLOSURE, 1)
TYPEDESC_DEFINE_TYPE(VDF, "VDF", TypeDesc::BASETYPE_NONE, TypeDesc::SEMANTIC_CLOSURE, 1)
TYPEDESC_DEFINE_TYPE(SURFACESHADER, "surfaceshader", TypeDesc::BASETYPE_NONE, TypeDesc::SEMANTIC_SHADER, 1)
TYPEDESC_DEFINE_TYPE(VOLUMESHADER, "volumeshader", TypeDesc::BASETYPE_NONE, TypeDesc::SEMANTIC_SHADER, 1)
TYPEDESC_DEFINE_TYPE(DISPLACEMENTSHADER, "displacementshader", TypeDesc::BASETYPE_NONE, TypeDesc::SEMANTIC_SHADER, 1)
TYPEDESC_DEFINE_TYPE(LIGHTSHADER, "lightshader", TypeDesc::BASETYPE_NONE, TypeDesc::SEMANTIC_SHADER, 1)
TYPEDESC_DEFINE_TYPE(MATERIAL, "material", TypeDesc::BASETYPE_NONE, TypeDesc::SEMANTIC_MATERIAL, 1)

} // namespace Type


/// @class StructTypeDesc
/// A type descriptor for MaterialX struct types.
///
/// All types need to have a type descriptor registered in order for shader generators
/// to know about the type. If the type represented is of basetype=BASETYPE_STRUCT then
/// the type also needs to have an associated StructTypeDesc that describes the members
/// of the struct.
///
class MX_GENSHADER_API StructTypeDesc
{
  public:
    struct StructMemberTypeDesc
    {
        StructMemberTypeDesc(string name, TypeDesc typeDesc, string defaultValueStr) :
            _name(name), _typeDesc(typeDesc), _defaultValueStr(defaultValueStr)
        {
        }
        string _name;
        TypeDesc _typeDesc;
        string _defaultValueStr;
    };

    /// Empty constructor.
    StructTypeDesc() noexcept{}

    void addMember(const string& name, TypeDesc type, const string& defaultValueStr);
    void setTypeDesc(TypeDesc typedesc) { _typedesc = typedesc; }

    /// Return a type description by index.
    static StructTypeDesc& get(unsigned int index);
    static vector<string> getStructTypeNames();
    static uint16_t emplace_back(StructTypeDesc structTypeDesc);
    static void clear();

    TypeDesc typeDesc() const { return _typedesc; }

    const string& getName() const;

    const vector<StructMemberTypeDesc>& getMembers() const;

  private:
    TypeDesc _typedesc;
    vector<StructMemberTypeDesc> _members;
};

/// @class StructTypeDescRegistry
/// Helper class for struct type registration.
class MX_GENSHADER_API StructTypeDescRegistry
{
  public:
    StructTypeDescRegistry();
};

MATERIALX_NAMESPACE_END

#endif
