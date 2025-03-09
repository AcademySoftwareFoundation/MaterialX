//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_ESSLSHADERGENERATOR_H
#define MATERIALX_ESSLSHADERGENERATOR_H

/// @file
/// ESSL shader generator

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

using EsslShaderGeneratorPtr = shared_ptr<class EsslShaderGenerator>;

/// @class EsslShaderGenerator
/// An ESSL (OpenGL ES Shading Language) shader generator
class MX_GENGLSL_API EsslShaderGenerator : public GlslShaderGenerator
{
  public:
    /// Constructor.
    EsslShaderGenerator(TypeSystemPtr typeSystem);

    /// Creator function.
    /// If a TypeSystem is not provided it will be created internally.
    /// Optionally pass in an externally created TypeSystem here, 
    /// if you want to keep type descriptions alive after the lifetime
    /// of the shader generator. 
    static ShaderGeneratorPtr create(TypeSystemPtr typeSystem = nullptr)
    {
        return std::make_shared<EsslShaderGenerator>(typeSystem ? typeSystem : TypeSystem::create());
    }

    /// Return a unique identifier for the target this generator is for
    const string& getTarget() const override { return TARGET; }

    /// Return the version string for the ESSL version this generator is for
    const string& getVersion() const override { return VERSION; }

    string getVertexDataPrefix(const VariableBlock& vertexData) const override;

    /// Unique identifier for this generator target
    static const string TARGET;
    static const string VERSION;

  protected:
    void emitDirectives(GenContext& context, ShaderStage& stage) const override;
    void emitUniforms(GenContext& context, ShaderStage& stage) const override;
    void emitInputs(GenContext& context, ShaderStage& stage) const override;
    void emitOutputs(GenContext& context, ShaderStage& stage) const override;
    HwResourceBindingContextPtr getResourceBindingContext(GenContext& context) const override;
};

MATERIALX_NAMESPACE_END

#endif
