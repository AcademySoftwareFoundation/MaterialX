//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_WGSL_SYNTAX_H
#define MATERIALX_WGSL_SYNTAX_H

/// @file
/// WGSL (WebGPU Shading Language) syntax class

#include <MaterialXGenWgsl/Export.h>

#include <MaterialXGenShader/Syntax.h>

MATERIALX_NAMESPACE_BEGIN

/// Syntax class for the WebGPU Shading Language (WGSL).
///
/// Unlike the legacy GLSL-derived WGSL flavor, this class derives directly from the
/// base Syntax and registers native WGSL type names (f32, vec3f, mat4x4f, ...). It is
/// modeled on SlangSyntax, the sibling complete-shader, non-GLSL syntax class.
class MX_GENWGSL_API WgslSyntax : public Syntax
{
  public:
    WgslSyntax(TypeSystemPtr typeSystem);

    static SyntaxPtr create(TypeSystemPtr typeSystem) { return std::make_shared<WgslSyntax>(typeSystem); }

    const string& getInputQualifier() const override { return INPUT_QUALIFIER; }
    const string& getOutputQualifier() const override { return OUTPUT_QUALIFIER; }
    const string& getConstantQualifier() const override { return CONSTANT_QUALIFIER; };
    const string& getUniformQualifier() const override { return UNIFORM_QUALIFIER; };
    const string& getSourceFileExtension() const override { return SOURCE_FILE_EXTENSION; };

    void makeValidName(string& name) const override;

    /// Given an input specification attempt to remap this to an enumeration which is accepted by
    /// the shader generator. The enumeration may be converted to a different type than the input.
    bool remapEnumeration(const string& value, TypeDesc type, const string& enumNames, std::pair<TypeDesc, ValuePtr>& result) const override;

    StructTypeSyntaxPtr createStructSyntax(const string& structTypeName, const string& defaultValue,
                                           const string& uniformDefaultValue, const string& typeAlias,
                                           const string& typeDefinition) const override;

    static const string INPUT_QUALIFIER;
    static const string OUTPUT_QUALIFIER;
    static const string UNIFORM_QUALIFIER;
    static const string CONSTANT_QUALIFIER;
    static const string FLAT_QUALIFIER;
    static const string SOURCE_FILE_EXTENSION;

    static const StringVec VEC2_MEMBERS;
    static const StringVec VEC3_MEMBERS;
    static const StringVec VEC4_MEMBERS;
};

/// Specialization of TypeSyntax for aggregate (struct) types in WGSL.
class MX_GENWGSL_API WgslStructTypeSyntax : public StructTypeSyntax
{
  public:
    using StructTypeSyntax::StructTypeSyntax;

    string getValue(const Value& value, bool uniform) const override;
};

MATERIALX_NAMESPACE_END

#endif
