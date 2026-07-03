//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_WGSLSHADERGENERATOR_H
#define MATERIALX_WGSLSHADERGENERATOR_H

/// @file
/// Native WGSL (WebGPU Shading Language) shader generator.
///
/// Produces complete, standalone WGSL vertex + fragment shaders (with @vertex / @fragment
/// entry points and standard @group/@binding resources) from MaterialX node graphs, backed by
/// hand-written WGSL node implementation libraries under the `genwgsl` target.
///
/// The class derives directly from HwShaderGenerator (not the GLSL hierarchy) and emits native
/// WGSL syntax. Its complete-shader structure mirrors SlangShaderGenerator, while the WGSL token
/// mechanics (var declarations, pointer outputs, split texture/sampler) follow the WebGPU spec.

#include <MaterialXGenWgsl/Export.h>

#include <MaterialXGenHw/HwShaderGenerator.h>
#include <MaterialXGenHw/HwResourceBindingContext.h>

MATERIALX_NAMESPACE_BEGIN

using WgslShaderGeneratorPtr = shared_ptr<class WgslShaderGenerator>;

/// @class WgslShaderGenerator
/// Native WGSL shader generator for the `genwgsl` target.
class MX_GENWGSL_API WgslShaderGenerator : public HwShaderGenerator
{
  public:
    WgslShaderGenerator(TypeSystemPtr typeSystem);

    /// Creator function.
    static ShaderGeneratorPtr create(TypeSystemPtr typeSystem = nullptr)
    {
        return std::make_shared<WgslShaderGenerator>(typeSystem ? typeSystem : TypeSystem::create());
    }

    /// Generate a complete WGSL shader starting from the given element.
    ShaderPtr generate(const string& name, ElementPtr element, GenContext& context) const override;

    /// Return the target identifier ("genwgsl").
    const string& getTarget() const override { return TARGET; }

    /// Return the WGSL version string.
    virtual const string& getVersion() const { return VERSION; }

    /// "type" is a WGSL reserved word, so the LightData type member is renamed to "light_type".
    const string& getLightDataTypevarString() const override { return LIGHTDATA_TYPEVAR_STRING; }

    /// Emit a WGSL variable declaration: "var name: type [= value]" / "const name: type = value".
    void emitVariableDeclaration(const ShaderPort* variable, const string& qualifier, GenContext& context, ShaderStage& stage,
                                 bool assignValue = true) const override;

    /// Emit a shader output, in WGSL syntax. As a function-call argument an output is passed by
    /// pointer (&var) to match the ptr<function, T> out-parameter convention.
    void emitOutput(const ShaderOutput* output, bool includeType, bool assignValue, GenContext& context, ShaderStage& stage) const override;

    /// Emit a WGSL function parameter ("name: type"); output params are skipped (handled by callers).
    /// For Type::FILENAME emit a texture_2d<f32> + sampler pair.
    void emitFunctionDefinitionParameter(const ShaderPort* shaderPort, bool isOutput, GenContext& context, ShaderStage& stage) const override;

    /// Emit a function-call input argument. For Type::FILENAME emit (var_texture, var_sampler).
    void emitInput(const ShaderInput* input, GenContext& context, ShaderStage& stage) const override;

    /// Emit the closure-data function parameter in WGSL syntax ("closureData: ClosureData, ").
    void emitClosureDataParameter(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    static const string TARGET;
    static const string VERSION;

  protected:
    static const string LIGHTDATA_TYPEVAR_STRING;

    /// Emit the complete WGSL vertex stage (struct IO + @vertex entry point).
    virtual void emitVertexStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const;

    /// Emit the complete WGSL pixel stage (type defs, uniforms, lighting, @fragment entry point).
    virtual void emitPixelStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const;

    /// Emit the vertex input struct (VERTEX) or the shared vertex-data struct (PIXEL).
    virtual void emitInputs(GenContext& context, ShaderStage& stage) const;

    /// Emit the shared vertex-data output struct (VERTEX) or the pixel output declaration (PIXEL).
    virtual void emitOutputs(GenContext& context, ShaderStage& stage) const;

    /// Emit value/texture uniform resource bindings (skips the LightData block).
    virtual void emitUniforms(GenContext& context, ShaderStage& stage) const;

    /// Emit the LightData struct + uniform array binding and MAX_LIGHT_SOURCES constant.
    virtual void emitLightData(GenContext& context, ShaderStage& stage) const;

    /// Emit module-level constants (M_PI, M_FLOAT_EPS, CLOSURE_TYPE_*, graph constants).
    virtual void emitConstants(GenContext& context, ShaderStage& stage) const;

    /// Emit closure / shader struct type definitions (surfaceshader, displacementshader, lightshader)
    /// plus the closure type library (ClosureData, BSDF, FresnelData, makeClosureData).
    void emitTypeDefinitions(GenContext& context, ShaderStage& stage) const override;

    /// Vertex data is accessed via the "vd." prefix.
    string getVertexDataPrefix(const VariableBlock& vertexData) const override;

    bool requiresLighting(const ShaderGraph& graph) const override;

    /// Emit the specular environment library (FIS by default for standalone shaders).
    virtual void emitSpecularEnvironment(GenContext& context, ShaderStage& stage) const;

    /// Emit the transmission rendering library (refraction or opacity).
    virtual void emitTransmissionRender(GenContext& context, ShaderStage& stage) const;

    /// WGSL has no preprocessor directives; this is a structural no-op.
    virtual void emitDirectives(GenContext& context, ShaderStage& stage) const;

    /// Emit light sampling function definitions when the graph requires lighting.
    void emitLightFunctionDefinitions(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const;

    /// Override emitBlock to track emitted WGSL functions and avoid duplicate definitions.
    void emitBlock(const string& str, const FilePath& sourceFilename, GenContext& context, ShaderStage& stage) const override;

    /// Use the WGSL-native compound / light-compound node implementations.
    ShaderNodeImplPtr createShaderNodeImplForNodeGraph(const NodeGraph& nodegraph) const override;

    /// Use the WGSL-native source-code node implementation (WGSL declaration order).
    ShaderNodeImplPtr createShaderNodeImplForImplementation(const Implementation& implementation) const override;

    /// Get the resource binding context from the GenContext.
    HwResourceBindingContextPtr getResourceBindingContext(GenContext& context) const;

    /// Register the genwgsl node implementations.
    void registerImplementations(const string& target);

    vector<ShaderNodePtr> _lightSamplingNodes;
};

MATERIALX_NAMESPACE_END

#endif
