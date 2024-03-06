//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/TypeDesc.h>

#include <MaterialXCore/Exception.h>

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

TypeDescRegistry::TypeDescRegistry(TypeDesc type, const std::string& name)
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

}

MATERIALX_NAMESPACE_END
