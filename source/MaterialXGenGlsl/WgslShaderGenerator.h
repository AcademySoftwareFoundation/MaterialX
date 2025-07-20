//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_WGSLSHADERGENERATOR_H
#define MATERIALX_WGSLSHADERGENERATOR_H

/// @file
/// Vulkan GLSL shader generator flavor for WGSL

#include <MaterialXGenGlsl/VkShaderGenerator.h>
#include <MaterialXGenGlsl/WgslResourceBindingContext.h>
#include <MaterialXGenShader/GenContext.h>

MATERIALX_NAMESPACE_BEGIN

using WgslShaderGeneratorPtr = shared_ptr<class WgslShaderGenerator>;

/// @class WgslShaderGenerator
/// WGSL Flavor of Vulkan GLSL shader generator 
class MX_GENGLSL_API WgslShaderGenerator : public VkShaderGenerator
{
  public:
    /// Constructor.
    WgslShaderGenerator(TypeSystemPtr typeSystem);

    /// Creator function.
    /// If a TypeSystem is not provided it will be created internally.
    /// Optionally pass in an externally created TypeSystem here, 
    /// if you want to keep type descriptions alive after the lifetime
    /// of the shader generator. 
    static ShaderGeneratorPtr create(TypeSystemPtr typeSystem = nullptr)
    {
        return std::make_shared<WgslShaderGenerator>(typeSystem ? typeSystem : TypeSystem::create());
    }

    void emitDirectives(GenContext& context, ShaderStage& stage) const override;

    const string& getLightDataTypevarString() const override { return LIGHTDATA_TYPEVAR_STRING; }

    void emitFunctionDefinitionParameter(const ShaderPort* shaderPort, bool isOutput, GenContext& context, ShaderStage& stage) const override;

    void emitInput(const ShaderInput* input, GenContext& context, ShaderStage& stage) const override;

  protected:
    static const string LIGHTDATA_TYPEVAR_STRING;
};

MATERIALX_NAMESPACE_END

#endif
