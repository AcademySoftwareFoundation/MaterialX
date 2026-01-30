//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_MSLSHADERGENERATOR_H
#define MATERIALX_MSLSHADERGENERATOR_H

/// @file
/// MSL shader generator

#include <MaterialXGenMsl/Export.h>

#include <MaterialXGenHw/HwShaderGenerator.h>
#include <MaterialXGenHw/HwResourceBindingContext.h>

#define TEXTURE_NAME(t) ((t) + "_tex")
#define SAMPLER_NAME(t) ((t) + "_sampler")

MATERIALX_NAMESPACE_BEGIN

using MslShaderGeneratorPtr = shared_ptr<class MslShaderGenerator>;

/// Base class for MSL (OpenGL Shading Language) code generation.
/// A generator for a specific MSL target should be derived from this class.
class MX_GENMSL_API MslShaderGenerator : public HwShaderGenerator
{
  public:
    /// Constructor.
    MslShaderGenerator(TypeSystemPtr typeSystem);

    /// Creator function.
    /// If a TypeSystem is not provided it will be created internally.
    /// Optionally pass in an externally created TypeSystem here, 
    /// if you want to keep type descriptions alive after the lifetime
    /// of the shader generator. 
    static ShaderGeneratorPtr create(TypeSystemPtr typeSystem = nullptr)
    {
        return std::make_shared<MslShaderGenerator>(typeSystem ? typeSystem : TypeSystem::create());
    }

    /// Generate a shader starting from the given element, translating
    /// the element and all dependencies upstream into shader code.
    ShaderPtr generate(const string& name, ElementPtr element, GenContext& context) const override;

    /// Return a unique identifier for the target this generator is for
    const string& getTarget() const override { return TARGET; }

    /// Return the version string for the MSL version this generator is for
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

    virtual void emitMetalTextureClass(GenContext& context, ShaderStage& stage) const;
    virtual void emitDirectives(GenContext& context, ShaderStage& stage) const;
    virtual void emitConstants(GenContext& context, ShaderStage& stage) const;
    virtual void emitLightData(GenContext& context, ShaderStage& stage) const;
    virtual void emitInputs(GenContext& context, ShaderStage& stage) const;
    virtual void emitOutputs(GenContext& context, ShaderStage& stage) const;

    virtual void emitMathMatrixScalarMathOperators(GenContext& context, ShaderStage& stage) const;
    virtual void MetalizeGeneratedShader(ShaderStage& shaderStage) const;

    void emitConstantBufferDeclarations(GenContext& context,
                                        HwResourceBindingContextPtr resourceBindingCtx,
                                        ShaderStage& stage) const;

    enum EmitGlobalScopeContext
    {
        EMIT_GLOBAL_SCOPE_CONTEXT_ENTRY_FUNCTION_RESOURCES = 0,
        EMIT_GLOBAL_SCOPE_CONTEXT_MEMBER_INIT = 1,
        EMIT_GLOBAL_SCOPE_CONTEXT_MEMBER_DECL = 2,
        EMIT_GLOBAL_SCOPE_CONTEXT_CONSTRUCTOR_ARGS = 3,
        EMIT_GLOBAL_SCOPE_CONTEXT_CONSTRUCTOR_INIT = 4
    };

    void emitGlobalVariables(GenContext& context, ShaderStage& stage,
                             EmitGlobalScopeContext situation,
                             bool isVertexShader,
                             bool needsLightData) const;

    void emitInputs(GenContext& context, ShaderStage& stage,
                    const VariableBlock& inputs) const;

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
