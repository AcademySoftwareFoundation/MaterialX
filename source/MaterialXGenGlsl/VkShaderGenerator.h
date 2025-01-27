//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_VKSHADERGENERATOR_H
#define MATERIALX_VKSHADERGENERATOR_H

/// @file
/// Vulkan GLSL shader generator

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenGlsl/VkResourceBindingContext.h>

MATERIALX_NAMESPACE_BEGIN

using VkShaderGeneratorPtr = shared_ptr<class VkShaderGenerator>;

/// @class VkShaderGenerator
/// A Vulkan GLSL shader generator
class MX_GENGLSL_API VkShaderGenerator : public GlslShaderGenerator
{
  public:
    /// Constructor.
    VkShaderGenerator(TypeSystemPtr typeSystem);

    /// Creator function.
    /// If a TypeSystem is not provided it will be created internally.
    /// Optionally pass in an externally created TypeSystem here, 
    /// if you want to keep type descriptions alive after the lifetime
    /// of the shader generator. 
    static ShaderGeneratorPtr create(TypeSystemPtr typeSystem = nullptr)
    {
        return std::make_shared<VkShaderGenerator>(typeSystem ? typeSystem : TypeSystem::create());
    }

    /// Return a unique identifier for the target this generator is for
    const string& getTarget() const override { return TARGET; }

    /// Return the version string for the GLSL version this generator is for
    const string& getVersion() const override { return VERSION; }

    string getVertexDataPrefix(const VariableBlock& vertexData) const override;

    /// Unique identifier for this generator target
    static const string TARGET;
    static const string VERSION;

    // Emit directives for stage
    void emitDirectives(GenContext& context, ShaderStage& stage) const override;

    void emitInputs(GenContext& context, ShaderStage& stage) const override;

    void emitOutputs(GenContext& context, ShaderStage& stage) const override;

  protected:
    HwResourceBindingContextPtr getResourceBindingContext(GenContext&) const override;

    VkResourceBindingContextPtr _resourceBindingCtx = nullptr;
};

MATERIALX_NAMESPACE_END

#endif
