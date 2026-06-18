//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_WGSLSHADERGENERATOR_H
#define MATERIALX_WGSLSHADERGENERATOR_H

/// @file
/// WGSL shader generator

#include <MaterialXGenGlsl/Export.h>
#include <MaterialXGenGlsl/vk/VkShaderGenerator.h>
#include <MaterialXGenGlsl/wgsl/WgslResourceBindingContext.h>

MATERIALX_NAMESPACE_BEGIN

// The GLSL->WGSL text utilities are internal to MaterialXGenGlsl (see wgsl/converter/GlslToWgsl.h);
// only the incomplete LineRewriter type is referenced here, so a forward declaration keeps
// that internal header out of the installed public API.
namespace GlslToWgsl
{
class LineRewriter;
}

using WgslShaderGeneratorPtr = shared_ptr<class WgslShaderGenerator>;

/// A WGSL (WebGPU Shading Language) shader generator.
/// Reuses the genglsl emit pipeline (via VkShaderGenerator) and converts emitted GLSL to WGSL
/// at emit time via the GlslToWgsl utilities.
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

    /// Return a unique identifier for the target this generator is for
    const string& getTarget() const override { return TARGET; }

    const string& getLightDataTypevarString() const override { return LIGHTDATA_TYPEVAR_STRING; }

    ShaderNodeImplPtr getImplementation(const NodeDef& nodedef, GenContext& context) const override;

    /// Unique identifier for this generator target
    static const string TARGET;

    /// Emit a shader variable.
    void emitVariableDeclaration(const ShaderPort* variable, const string& qualifier, GenContext& context, ShaderStage& stage, bool assignValue = true) const override;
    void emitFunctionDefinitionParameter(const ShaderPort* shaderPort, bool isOutput, GenContext& context, ShaderStage& stage) const override;
    void emitInput(const ShaderInput* input, GenContext& context, ShaderStage& stage) const override;
    void emitOutput(const ShaderOutput* output, bool includeType, bool assignValue, GenContext& context, ShaderStage& stage) const override;
    void emitString(const string& str, ShaderStage& stage) const override;
    void emitLine(const string& str, ShaderStage& stage, bool semicolon = true) const override;
    void emitLineEnd(ShaderStage& stage, bool semicolon = true) const override;
    void emitBlock(const string& str, const FilePath& sourceFilename, GenContext& context, ShaderStage& stage) const override;
    void emitLibraryInclude(const FilePath& filename, GenContext& context, ShaderStage& stage) const override;
    void emitTypeDefinitions(GenContext& context, ShaderStage& stage) const override;

  protected:
    void emitVertexStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const override;
    void emitPixelStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const override;

    void emitDirectives(GenContext& context, ShaderStage& stage) const override;
    void emitWgslSurfaceTypes(GenContext& context, ShaderStage& stage) const;
    void emitPixelUniforms(GenContext& context, ShaderStage& stage, bool lighting) const;
    void emitLightData(GenContext& context, ShaderStage& stage) const override;
    void emitInputs(GenContext& context, ShaderStage& stage) const override;
    void emitVertexDataStruct(const VariableBlock& vertexData, ShaderStage& stage) const;
    void emitOutputs(GenContext& context, ShaderStage& stage) const override;

    void finalizeStageSource(ShaderStage& stage) const;
    string expandAndRewriteGlsl(const string& source, const FilePath& sourceFilename,
                                GenContext& context, GlslToWgsl::LineRewriter& rewriter) const;

    static const string LIGHTDATA_TYPEVAR_STRING;
};

MATERIALX_NAMESPACE_END

#endif
