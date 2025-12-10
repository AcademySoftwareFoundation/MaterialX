//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_GLSLSHADERGENERATOR_H
#define MATERIALX_GLSLSHADERGENERATOR_H

/// @file
/// GLSL shader generator

#include <MaterialXGenGlsl/Export.h>

#include <MaterialXGenHw/HwResourceBindingContext.h>
#include <MaterialXGenHw/HwShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

using GlslShaderGeneratorPtr = shared_ptr<class GlslShaderGenerator>;

/// Base class for GLSL (OpenGL Shading Language) code generation.
/// A generator for a specific GLSL target should be derived from this class.
class MX_GENGLSL_API GlslShaderGenerator : public HwShaderGenerator
{
  public:
    /// Constructor.
    GlslShaderGenerator(TypeSystemPtr typeSystem);

    /// Creator function.
    /// If a TypeSystem is not provided it will be created internally.
    /// Optionally pass in an externally created TypeSystem here, 
    /// if you want to keep type descriptions alive after the lifetime
    /// of the shader generator. 
    static ShaderGeneratorPtr create(TypeSystemPtr typeSystem = nullptr)
    {
        return std::make_shared<GlslShaderGenerator>(typeSystem ? typeSystem : TypeSystem::create());
    }

    /// Generate a shader starting from the given element, translating
    /// the element and all dependencies upstream into shader code.
    ShaderPtr generate(const string& name, ElementPtr element, GenContext& context) const override;

    /// Return a unique identifier for the target this generator is for
    const string& getTarget() const override { return TARGET; }

    /// Return the version string for the GLSL version this generator is for
    virtual const string& getVersion() const { return VERSION; }

    /// Emit a shader variable.
    void emitVariableDeclaration(const ShaderPort* variable, const string& qualifier, GenContext& context, ShaderStage& stage,
                                 bool assignValue = true) const override;

    /// Determine the prefix of vertex data variables.
    string getVertexDataPrefix(const VariableBlock& vertexData) const override;

  public:
    /// Unique identifier for this generator target
    static const string TARGET;

    /// Version string for the generator target
    static const string VERSION;

  protected:
    virtual void emitVertexStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const;
    virtual void emitPixelStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const;

    virtual void emitDirectives(GenContext& context, ShaderStage& stage) const;
    virtual void emitConstants(GenContext& context, ShaderStage& stage) const;
    virtual void emitUniforms(GenContext& context, ShaderStage& stage) const;
    virtual void emitLightData(GenContext& context, ShaderStage& stage) const;
    virtual void emitInputs(GenContext& context, ShaderStage& stage) const;
    virtual void emitOutputs(GenContext& context, ShaderStage& stage) const;

    virtual HwResourceBindingContextPtr getResourceBindingContext(GenContext& context) const;

    /// Emit specular environment lookup code
    virtual void emitSpecularEnvironment(GenContext& context, ShaderStage& stage) const;

    /// Emit transmission rendering code
    virtual void emitTransmissionRender(GenContext& context, ShaderStage& stage) const;

    /// Emit function definitions for lighting code
    virtual void emitLightFunctionDefinitions(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const;

    /// Nodes used internally for light sampling.
    vector<ShaderNodePtr> _lightSamplingNodes;
};

MATERIALX_NAMESPACE_END

#endif
