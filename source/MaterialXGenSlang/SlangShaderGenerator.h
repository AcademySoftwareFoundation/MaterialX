//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_SLANGSHADERGENERATOR_H
#define MATERIALX_SLANGSHADERGENERATOR_H

/// @file
/// Slang shader generator

#include <MaterialXGenSlang/Export.h>

#include <MaterialXGenHw/HwShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

using SlangShaderGeneratorPtr = shared_ptr<class SlangShaderGenerator>;

/// Base class for Slang code generation.
/// A generator for a specific Slang target should be derived from this class.
class MX_GENSLANG_API SlangShaderGenerator : public HwShaderGenerator
{
  public:
    /// Constructor.
    SlangShaderGenerator(TypeSystemPtr typeSystem);

    /// Creator function.
    /// If a TypeSystem is not provided it will be created internally.
    /// Optionally pass in an externally created TypeSystem here,
    /// if you want to keep type descriptions alive after the lifetime
    /// of the shader generator.
    static ShaderGeneratorPtr create(TypeSystemPtr typeSystem = nullptr)
    {
        return std::make_shared<SlangShaderGenerator>(typeSystem ? typeSystem : TypeSystem::create());
    }

    /// Generate a shader starting from the given element, translating
    /// the element and all dependencies upstream into shader code.
    ShaderPtr generate(const string& name, ElementPtr element, GenContext& context) const override;

    /// Return a unique identifier for the target this generator is for
    const string& getTarget() const override { return TARGET; }

    /// Return the version string for the Slang version this generator is for
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
    virtual void emitUniforms(GenContext& context, ShaderStage& stage, bool emitLighting) const;
    virtual void emitInputs(GenContext& context, ShaderStage& stage) const;
    virtual void emitOutputs(GenContext& context, ShaderStage& stage) const;

    /// Logic to indicate whether code to support direct lighting should be emitted.
    /// By default if the graph is classified as a shader, or BSDF node then lighting is assumed to be required.
    /// Derived classes can override this logic.
    virtual bool requiresLighting(const ShaderGraph& graph) const override;

    /// Emit specular environment lookup code
    virtual void emitSpecularEnvironment(GenContext& context, ShaderStage& stage) const;

    /// Emit transmission rendering code
    virtual void emitTransmissionRender(GenContext& context, ShaderStage& stage) const;

    /// Emit function definitions for lighting code
    virtual void emitLightFunctionDefinitions(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const;

    static void toVec4(TypeDesc type, string& variable);
    [[deprecated]] static void toVec4(const TypeDesc* type, string& variable) { toVec4(*type, variable); }

    void SlangSyntaxFromGlsl(ShaderStage& shaderStage) const;

    /// Nodes used internally for light sampling.
    vector<ShaderNodePtr> _lightSamplingNodes;
};

MATERIALX_NAMESPACE_END

#endif
