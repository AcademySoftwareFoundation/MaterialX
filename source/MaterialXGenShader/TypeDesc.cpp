//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/TypeDesc.h>

#include <MaterialXGenShader/ShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{

using TypeDescMap = std::unordered_map<string, TypeDesc>;
using TypeDescNameMap = std::unordered_map<uint32_t, string>;

// Internal storage of registered type descriptors
TypeDescMap& typeMap()
{
    static TypeDescMap map;
    return map;
}

TypeDescNameMap& typeNameMap()
{
    static TypeDescNameMap map;
    return map;
}

using StructTypeDescStorage = vector<StructTypeDesc>;
StructTypeDescStorage& structTypeStorage()
{
    // TODO: Our use of the singleton pattern for TypeDescMap and StructTypeDestStorage
    //       is not thread-safe, and we should consider replacing this with thread-local
    //       data in the GenContext object.
    static StructTypeDescStorage storage;
    return storage;
}

} // anonymous namespace

const string TypeDesc::NONE_TYPE_NAME = "none";

const string& TypeDesc::getName() const
{
    TypeDescNameMap& typenames = typeNameMap();
    auto it = typenames.find(_id);
    return it != typenames.end() ? it->second : NONE_TYPE_NAME;
}

TypeDesc TypeDesc::get(const string& name)
{
    TypeDescMap& types = typeMap();
    auto it = types.find(name);
    return it != types.end() ? it->second : Type::NONE;
}

void TypeDesc::remove(const string& name)
{
    TypeDescNameMap& typenames = typeNameMap();

    TypeDescMap& types = typeMap();

    auto it = types.find(name);
    if (it == types.end())
        return;

    typenames.erase(it->second.typeId());
    types.erase(it);
}

ValuePtr TypeDesc::createValueFromStrings(const string& value) const
{
    ValuePtr newValue = Value::createValueFromStrings(value, getName());
    if (!isStruct())
        return newValue;

    // Value::createValueFromStrings() can only create a valid Value for a struct if it is passed
    // the optional TypeDef argument, otherwise it just returns a "string" typed Value.
    // So if this is a struct type we need to create a new AggregateValue.

    StringVec subValues = parseStructValueString(value);

    AggregateValuePtr  result = AggregateValue::createAggregateValue(getName());
    auto structTypeDesc = StructTypeDesc::get(getStructIndex());
    const auto& members = structTypeDesc.getMembers();

    if (subValues.size() != members.size())
    {
        std::stringstream ss;
        ss << "Wrong number of initializers - expect " << members.size();
        throw ExceptionShaderGenError(ss.str());
    }

    for (size_t i = 0; i < members.size(); ++i)
    {
        result->appendValue( members[i]._typeDesc.createValueFromStrings(subValues[i]));
    }

    return result;
}

TypeDescRegistry::TypeDescRegistry(TypeDesc type, const string& name)
{
    TypeDescMap& types = typeMap();
    TypeDescNameMap& typenames = typeNameMap();
    types[name] = type;
    typenames[type.typeId()] = name;
}

namespace Type
{

///
/// Register type descriptors for standard types.
///
TYPEDESC_REGISTER_TYPE(NONE, "none")
TYPEDESC_REGISTER_TYPE(BOOLEAN, "boolean")
TYPEDESC_REGISTER_TYPE(INTEGER, "integer")
TYPEDESC_REGISTER_TYPE(INTEGERARRAY, "integerarray")
TYPEDESC_REGISTER_TYPE(FLOAT, "float")
TYPEDESC_REGISTER_TYPE(FLOATARRAY, "floatarray")
TYPEDESC_REGISTER_TYPE(VECTOR2, "vector2")
TYPEDESC_REGISTER_TYPE(VECTOR3, "vector3")
TYPEDESC_REGISTER_TYPE(VECTOR4, "vector4")
TYPEDESC_REGISTER_TYPE(COLOR3, "color3")
TYPEDESC_REGISTER_TYPE(COLOR4, "color4")
TYPEDESC_REGISTER_TYPE(MATRIX33, "matrix33")
TYPEDESC_REGISTER_TYPE(MATRIX44, "matrix44")
TYPEDESC_REGISTER_TYPE(STRING, "string")
TYPEDESC_REGISTER_TYPE(FILENAME, "filename")
TYPEDESC_REGISTER_TYPE(BSDF, "BSDF")
TYPEDESC_REGISTER_TYPE(EDF, "EDF")
TYPEDESC_REGISTER_TYPE(VDF, "VDF")
TYPEDESC_REGISTER_TYPE(SURFACESHADER, "surfaceshader")
TYPEDESC_REGISTER_TYPE(VOLUMESHADER, "volumeshader")
TYPEDESC_REGISTER_TYPE(DISPLACEMENTSHADER, "displacementshader")
TYPEDESC_REGISTER_TYPE(LIGHTSHADER, "lightshader")
TYPEDESC_REGISTER_TYPE(MATERIAL, "material")

} // namespace Type

//
// StructTypeDesc methods
//

void StructTypeDesc::addMember(const string& name, TypeDesc type, const string& defaultValueStr)
{
    _members.emplace_back(StructTypeDesc::StructMemberTypeDesc(name, type, defaultValueStr));
}

vector<string> StructTypeDesc::getStructTypeNames()
{
    StructTypeDescStorage& structs = structTypeStorage();
    vector<string> structNames;
    for (const auto& x : structs)
    {
        structNames.emplace_back(x.typeDesc().getName());
    }
    return structNames;
}

StructTypeDesc& StructTypeDesc::get(unsigned int index)
{
    StructTypeDescStorage& structs = structTypeStorage();
    return structs[index];
}

uint16_t StructTypeDesc::emplace_back(StructTypeDesc structTypeDesc)
{
    StructTypeDescStorage& structs = structTypeStorage();
    if (structs.size() >= std::numeric_limits<uint16_t>::max())
    {
        throw ExceptionShaderGenError("Maximum number of custom struct types has been exceeded.");
    }
    uint16_t index = static_cast<uint16_t>(structs.size());
    structs.emplace_back(structTypeDesc);
    return index;
}

void StructTypeDesc::clear()
{
    StructTypeDescStorage& structs = structTypeStorage();
    for (const auto& structType: structs)
    {
        // Need to add typeID to structTypeDesc - and use it here to reference back to typeDesc obj and remove it.
        TypeDesc::remove(structType.typeDesc().getName());
    }
    structs.clear();
}

const string& StructTypeDesc::getName() const
{
    return _typedesc.getName();
}

const vector<StructTypeDesc::StructMemberTypeDesc>& StructTypeDesc::getMembers() const
{
    return _members;
}

TypeDesc createStructTypeDesc(std::string_view name)
{
    return {name, TypeDesc::BASETYPE_STRUCT};
}

void registerStructTypeDesc(std::string_view name)
{
    auto structTypeDesc = createStructTypeDesc(name);
    TypeDescRegistry register_struct(structTypeDesc, string(name));
}

MATERIALX_NAMESPACE_END
