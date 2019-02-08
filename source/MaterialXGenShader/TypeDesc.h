#ifndef MATERIALX_TYPEDESC_H
#define MATERIALX_TYPEDESC_H

#include <MaterialXCore/Library.h>

namespace MaterialX
{

using ChannelMap = std::unordered_map<char, int>;

/// @class TypeDesc
/// A type descriptor for MaterialX data types.
/// All types need to have a type descriptor registered in order for shader generators
/// to know about the type. A unique type descriptor pointer is the identifier used for
/// types, and can be used for type comparisons as well as getting more information
/// about the type. All standard library data types are registered by default and their
/// type descriptors can be accessed from the Type namespace, e.g. MaterialX::Type::FLOAT.
/// If custom types are used they must be registered by calling TypeDesc::registerType().
/// Descriptors for registered types can be retreived using TypeDesc::get(), see below.
class TypeDesc
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
        SEMANTIC_LAST
    };

    /// Register a type descriptor for a MaterialX data type.
    /// Throws an exception if a type with the same name is already registered.
    static const TypeDesc* registerType(const string& name, unsigned char basetype,
        unsigned char semantic = SEMANTIC_NONE, int size = 1, bool editable = true,
        const ChannelMap& channelMapping = ChannelMap());

    /// Get a type descriptor for given name.
    /// Throws an exception if no type with that name is found.
    static const TypeDesc* get(const string& name);

    /// Return the name of the type.
    const string& getName() const { return _name; }

    /// Return the basetype for the type.
    unsigned char getBaseType() const { return _basetype; }

    /// Returns the channel index for the supplied channel name.
    /// Will return -1 on failure to find a matching index.
    int getChannelIndex(char channel) const;

    /// Return the semantic for the type.
    unsigned char getSemantic() const { return _semantic; }

    /// Return the number of elements the type is composed of.
    /// Will return 1 for scalar types and a size greater than 1 for aggregate type.
    /// For array types 0 is returned since the number of elements is undefined
    /// until an array is instantiated.
    size_t getSize() const { return _size; }

    /// Returns true if the type is editable by users.
    /// Editable types are allowed to be published as shader uniforms
    /// and hence must be presentable in a user interface.
    bool isEditable() const { return _editable; }

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

private:
    TypeDesc(const string& name, unsigned char basetype,
      unsigned char semantic, int size, bool editable,
      const ChannelMap& channelMapping);

    const string _name;
    const unsigned char _basetype;
    const unsigned char _semantic;
    const int _size;
    const bool _editable;
    const ChannelMap _channelMapping;
};

namespace Type
{
    /// Type descriptors for all standard types.
    /// These are always registered by default.
    ///
    /// TODO: Add support for the standard array types.
    ///
    extern const TypeDesc* NONE;
    extern const TypeDesc* BOOLEAN;
    extern const TypeDesc* INTEGER;
    extern const TypeDesc* INTEGERARRAY;
    extern const TypeDesc* FLOAT; 
    extern const TypeDesc* FLOATARRAY;
    extern const TypeDesc* VECTOR2;
    extern const TypeDesc* VECTOR3;
    extern const TypeDesc* VECTOR4;
    extern const TypeDesc* COLOR2;
    extern const TypeDesc* COLOR3;
    extern const TypeDesc* COLOR4;
    extern const TypeDesc* MATRIX33;
    extern const TypeDesc* MATRIX44;
    extern const TypeDesc* STRING;
    extern const TypeDesc* FILENAME;
    extern const TypeDesc* BSDF;
    extern const TypeDesc* EDF;
    extern const TypeDesc* VDF;
    extern const TypeDesc* ROUGHNESSINFO;
    extern const TypeDesc* SURFACESHADER;
    extern const TypeDesc* VOLUMESHADER;
    extern const TypeDesc* DISPLACEMENTSHADER;
    extern const TypeDesc* LIGHTSHADER;
} // namespace Type

} // namespace MaterialX

#endif
