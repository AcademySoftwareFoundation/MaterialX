//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTTYPEDEF_H
#define MATERIALX_RTTYPEDEF_H

/// @file
/// Type identifiers and type descriptors for runtime data types.

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtToken.h>
#include <MaterialXRuntime/RtValue.h>

namespace MaterialX
{

/// Class holding identifier tokens for the built in data types.
class RtType
{
public:
    static const RtToken BOOLEAN;
    static const RtToken INTEGER;
    static const RtToken FLOAT;
    static const RtToken VECTOR2;
    static const RtToken VECTOR3;
    static const RtToken VECTOR4;
    static const RtToken COLOR3;
    static const RtToken COLOR4;
    static const RtToken MATRIX33;
    static const RtToken MATRIX44;
    static const RtToken TOKEN;
    static const RtToken STRING;
    static const RtToken FILENAME;
    static const RtToken INTEGERARRAY;
    static const RtToken FLOATARRAY;
    static const RtToken COLOR3ARRAY;
    static const RtToken COLOR4ARRAY;
    static const RtToken VECTOR2ARRAY;
    static const RtToken VECTOR3ARRAY;
    static const RtToken VECTOR4ARRAY;
    static const RtToken STRINGARRAY;
    static const RtToken BSDF;
    static const RtToken EDF;
    static const RtToken VDF;
    static const RtToken SURFACESHADER;
    static const RtToken VOLUMESHADER;
    static const RtToken DISPLACEMENTSHADER;
    static const RtToken LIGHTSHADER;
    static const RtToken MATERIAL;
    static const RtToken AUTO;
};

/// Function type for creating a value of a specific data type.
using RtValueCreateFunc = std::function<RtValue(RtPrim & owner)>;

/// Function type for copying a value of a specific data type.
using RtValueCopyFunc = std::function<void(const RtValue & src, RtValue & dest)>;

/// Function type for comparing two values for equality.
using RtValueCompareFunc = std::function<bool(const RtValue & a, const RtValue & b)>;

/// Function type for converting a value of a specific data type.
using RtValueToStringFunc = std::function<void(const RtValue & src, string & dest)>;

/// Function type for converting a value of a specific data type.
using RtValueFromStringFunc = std::function<void(const string & src, RtValue & dest)>;

/// @struct RtValueFuncs
/// Struct holding functions for creation and conversion
/// of values of a specific data type.
struct RtValueFuncs
{
    RtValueCreateFunc create;
    RtValueCopyFunc copy;
    RtValueCompareFunc compare;
    RtValueToStringFunc toString;
    RtValueFromStringFunc fromString;
};

/// @class RtTypeDef
/// A type definition for MaterialX data types.
/// All types need to have a type definition registered in order for the runtime
/// to know about the type. A unique type definition pointer can be used for type
/// comparisons as well as getting more information about the type.
/// All standard library data types are registered by default. Custom types can be
/// registered using RtTypeDef::registerType().
class RtTypeDef
{
public:
    /// Identifiers for base types.
    static const RtToken BASETYPE_NONE;
    static const RtToken BASETYPE_BOOLEAN;
    static const RtToken BASETYPE_INTEGER;
    static const RtToken BASETYPE_FLOAT;
    static const RtToken BASETYPE_STRING;
    static const RtToken BASETYPE_STRUCT;

    /// Identifiers for type semantics.
    static const RtToken SEMANTIC_NONE;
    static const RtToken SEMANTIC_COLOR;
    static const RtToken SEMANTIC_VECTOR;
    static const RtToken SEMANTIC_MATRIX;
    static const RtToken SEMANTIC_FILENAME;
    static const RtToken SEMANTIC_CLOSURE;
    static const RtToken SEMANTIC_SHADER;
    static const RtToken SEMANTIC_MATERIAL;

public:
    /// Constructor.
    RtTypeDef(const RtToken& name, const RtToken& basetype, const RtValueFuncs& funcs, const RtToken& semantic, size_t size);

    /// Destructor.
    ~RtTypeDef();

    /// Return the name of the type.
    const RtToken& getName() const;

    /// Return the basetype for the type.
    const RtToken& getBaseType() const;

    /// Return the semantic for the type.
    const RtToken& getSemantic() const;

    /// Return the number of components the type is composed of.
    /// Will return 1 for scalar types and a size greater than 1 for aggregate type.
    /// For array types 0 is returned since the number of elements is undefined
    /// until an array is instantiated.
    size_t getSize() const;

    /// Return true if the type is a scalar type.
    bool isScalar() const
    {
        return getSize() == 1;
    }

    /// Return true if the type is an aggregate type.
    bool isAggregate() const
    {
        return getSize() > 1;
    }

    /// Return true if the type is an array type.
    bool isArray() const
    {
        return getSize() == 0;
    }

    /// Set named and basetype for a component of an aggregate type.
    void setComponent(size_t index, const RtToken& name, const RtToken& basetype);

    /// Returns the index for the given component name.
    /// Will return INVALID_INDEX on failure to find a matching component.
    size_t getComponentIndex(const RtToken& name) const;

    /// Return the name of the aggregate component for the given index.
    /// Will throw exception if the index is larger than the type size.
    const RtToken& getComponentName(size_t index) const;

    /// Return the basetype of the aggregate component for the given index.
    /// Will throw exception if the index is larger than the type size.
    const RtToken& getComponentBaseType(size_t index) const;

    /// Return a set of all types that this type can be connected to.
    /// The type itself is also included in this set.
    const RtTokenSet& getValidConnectionTypes() const;

    /// Create a new value of this type.
    /// If the type is a large value the given prim will take
    /// ownership of allocated data.
    RtValue createValue(RtPrim& owner) const;

    /// Copy data from one value to another.
    void copyValue(const RtValue& src, RtValue& dest) const;

    /// Test if two values are equal.
    bool compareValue(const RtValue& a, const RtValue& b) const;

    /// Convert an RtValue of this type into a string representation.
    void toStringValue(const RtValue& src, string& dest) const;

    /// Convert a string representation into an RtValue of this type.
    /// Destination RtValue must been initialized for the given type.
    void fromStringValue(const string& src, RtValue& dest) const;

    /// Register a type descriptor for a MaterialX data type.
    /// Throws an exception if a type with the same name is already registered.
    static RtTypeDef* registerType(const RtToken& name, const RtToken& basetype, const RtValueFuncs& funcs,
                                   const RtToken& semantic = SEMANTIC_NONE, size_t size = 1);

    /// Return the number of registered types.
    static size_t numTypes();

    /// Get the typedef for the i:th registered data type.
    /// Returns nullptr if no such type is registered.
    static const RtTypeDef* getType(size_t index);

    /// Get the typedef for the given type name.
    /// Returns nullptr if no such type is registered.
    static const RtTypeDef* findType(const RtToken& name);

private:
    void* _ptr;
};

} // namespace MaterialX

#endif
