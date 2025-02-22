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
#include <MaterialXCore/Document.h>

#include <string_view>

MATERIALX_NAMESPACE_BEGIN

class TypeDesc;
class StructMemberDesc;
using TypeDescVec = vector<TypeDesc>;
using TypeDescMap = std::unordered_map<string, TypeDesc>;
using StructMemberDescVec = vector<StructMemberDesc>;
using StructMemberDescVecPtr = shared_ptr<const StructMemberDescVec>;

/// @class TypeDesc
/// A type descriptor for MaterialX data types.
///
/// All types need to have a type descriptor registered in order for shader generators
/// to know about the type. It can be used for type comparisons as well as getting more
/// information about the type. Type descriptors for all standard library data types are
/// defined by default and can be accessed from the Type namespace, e.g. Type::FLOAT.
/// Custom struct types defined through typedef elements in a data library are loaded in 
/// and registered by calling the ShaderGenerator::registerTypeDefs method. The TypeSystem
/// class, see below, is used to manage all type descriptions. It can be used to query the 
/// registered types.
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

    /// Data block holding large data needed by the type description.
    class DataBlock
    {
      public:
        DataBlock(const string& name, const StructMemberDescVecPtr members = nullptr) noexcept : _name(name), _members(members) {}

        const string& getName() const { return _name; }
        StructMemberDescVecPtr getStructMembers() const { return _members; }

      private:
        const string _name;
        const StructMemberDescVecPtr _members;
    };

    using DataBlockPtr = std::shared_ptr<DataBlock>;

    /// Empty constructor.
    constexpr TypeDesc() noexcept :
        _id(0),
        _basetype(BASETYPE_NONE),
        _semantic(SEMANTIC_NONE),
        _size(0),
        _data(nullptr)
    {
    }

    /// Constructor.
    constexpr TypeDesc(std::string_view name, uint8_t basetype, uint8_t semantic, uint16_t size, const DataBlock* data) noexcept :
        _id(constexpr_hash(name)), // Note: We only store the hash to keep the class size minimal.
        _basetype(basetype),
        _semantic(semantic),
        _size(size),
        _data(data)
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

    /// Return a pointer to the struct member description.
    /// Will return nullptr if this is not a struct type.
    StructMemberDescVecPtr getStructMembers() const;

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
    const DataBlock* _data;
};

/// @class StructMemberDesc
/// Type descriptor for member of a struct type.
class StructMemberDesc
{
public:
    StructMemberDesc(TypeDesc type, const string& name, const string& defaultValueStr) :
        _type(type),
        _name(name),
        _defaultValueStr(defaultValueStr)
    {
    }

    TypeDesc getType() const { return _type; }
    const string& getName() const { return _name; }
    const string& getDefaultValueStr() const { return _defaultValueStr; }

private:
    const TypeDesc _type;
    const string _name;
    const string _defaultValueStr;
};

using TypeSystemPtr = shared_ptr<class TypeSystem>;

/// @class TypeSystem
/// Class handling registration, storage and query of type descriptions.
class MX_GENSHADER_API TypeSystem
{
public:
    /// Create a new type system.
    static TypeSystemPtr create();

    /// Register an existing type decription.
    void registerType(TypeDesc type);

    /// Create and register a new type description.
    void registerType(const string& name, uint8_t basetype, uint8_t semantic, uint16_t size, StructMemberDescVecPtr members = nullptr);

    /// Return a type description by name.
    /// If no type is found Type::NONE is returned.
    TypeDesc getType(const string& name) const;

    /// Return all registered type descriptions.
    const TypeDescVec& getTypes() const
    {
        return _types;
    }

protected:
    // Protected constructor
    TypeSystem();

private:
    TypeDescVec _types;
    TypeDescMap _typesByName;
    vector<TypeDesc::DataBlockPtr> _dataBlocks;
};

/// Macro to define global type descriptions for commonly used types.
#define TYPEDESC_DEFINE_TYPE(T, name, basetype, semantic, size) \
       inline const TypeDesc::DataBlock* T##_data() { static const TypeDesc::DataBlock _data(name); return &_data; } \
       static const TypeDesc T(name, basetype, semantic, size, T##_data());

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

MATERIALX_NAMESPACE_END

#endif
