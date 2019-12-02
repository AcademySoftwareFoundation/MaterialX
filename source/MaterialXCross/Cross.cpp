//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include "Cross.h"
#include "MaterialXCore/Library.h"
#include "MaterialXGenShader/HwShaderGenerator.h"

#include "glslang/Public/ShaderLang.h"
#include "SPIRV/GlslangToSpv.h"

#include "spirv_hlsl.hpp"

namespace MaterialX
{
namespace Cross
{
namespace
{
// Copied from glslang/MachineIndependent/ShaderLang.cpp
//
const TBuiltInResource defaultTBuiltInResource = {
    /* .MaxLights = */ 32,
    /* .MaxClipPlanes = */ 6,
    /* .MaxTextureUnits = */ 32,
    /* .MaxTextureCoords = */ 32,
    /* .MaxVertexAttribs = */ 64,
    /* .MaxVertexUniformComponents = */ 4096,
    /* .MaxVaryingFloats = */ 64,
    /* .MaxVertexTextureImageUnits = */ 32,
    /* .MaxCombinedTextureImageUnits = */ 80,
    /* .MaxTextureImageUnits = */ 32,
    /* .MaxFragmentUniformComponents = */ 4096,
    /* .MaxDrawBuffers = */ 32,
    /* .MaxVertexUniformVectors = */ 128,
    /* .MaxVaryingVectors = */ 8,
    /* .MaxFragmentUniformVectors = */ 16,
    /* .MaxVertexOutputVectors = */ 16,
    /* .MaxFragmentInputVectors = */ 15,
    /* .MinProgramTexelOffset = */ -8,
    /* .MaxProgramTexelOffset = */ 7,
    /* .MaxClipDistances = */ 8,
    /* .MaxComputeWorkGroupCountX = */ 65535,
    /* .MaxComputeWorkGroupCountY = */ 65535,
    /* .MaxComputeWorkGroupCountZ = */ 65535,
    /* .MaxComputeWorkGroupSizeX = */ 1024,
    /* .MaxComputeWorkGroupSizeY = */ 1024,
    /* .MaxComputeWorkGroupSizeZ = */ 64,
    /* .MaxComputeUniformComponents = */ 1024,
    /* .MaxComputeTextureImageUnits = */ 16,
    /* .MaxComputeImageUniforms = */ 8,
    /* .MaxComputeAtomicCounters = */ 8,
    /* .MaxComputeAtomicCounterBuffers = */ 1,
    /* .MaxVaryingComponents = */ 60,
    /* .MaxVertexOutputComponents = */ 64,
    /* .MaxGeometryInputComponents = */ 64,
    /* .MaxGeometryOutputComponents = */ 128,
    /* .MaxFragmentInputComponents = */ 128,
    /* .MaxImageUnits = */ 8,
    /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
    /* .MaxCombinedShaderOutputResources = */ 8,
    /* .MaxImageSamples = */ 0,
    /* .MaxVertexImageUniforms = */ 0,
    /* .MaxTessControlImageUniforms = */ 0,
    /* .MaxTessEvaluationImageUniforms = */ 0,
    /* .MaxGeometryImageUniforms = */ 0,
    /* .MaxFragmentImageUniforms = */ 8,
    /* .MaxCombinedImageUniforms = */ 8,
    /* .MaxGeometryTextureImageUnits = */ 16,
    /* .MaxGeometryOutputVertices = */ 256,
    /* .MaxGeometryTotalOutputComponents = */ 1024,
    /* .MaxGeometryUniformComponents = */ 1024,
    /* .MaxGeometryVaryingComponents = */ 64,
    /* .MaxTessControlInputComponents = */ 128,
    /* .MaxTessControlOutputComponents = */ 128,
    /* .MaxTessControlTextureImageUnits = */ 16,
    /* .MaxTessControlUniformComponents = */ 1024,
    /* .MaxTessControlTotalOutputComponents = */ 4096,
    /* .MaxTessEvaluationInputComponents = */ 128,
    /* .MaxTessEvaluationOutputComponents = */ 128,
    /* .MaxTessEvaluationTextureImageUnits = */ 16,
    /* .MaxTessEvaluationUniformComponents = */ 1024,
    /* .MaxTessPatchComponents = */ 120,
    /* .MaxPatchVertices = */ 32,
    /* .MaxTessGenLevel = */ 64,
    /* .MaxViewports = */ 16,
    /* .MaxVertexAtomicCounters = */ 0,
    /* .MaxTessControlAtomicCounters = */ 0,
    /* .MaxTessEvaluationAtomicCounters = */ 0,
    /* .MaxGeometryAtomicCounters = */ 0,
    /* .MaxFragmentAtomicCounters = */ 8,
    /* .MaxCombinedAtomicCounters = */ 8,
    /* .MaxAtomicCounterBindings = */ 1,
    /* .MaxVertexAtomicCounterBuffers = */ 0,
    /* .MaxTessControlAtomicCounterBuffers = */ 0,
    /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
    /* .MaxGeometryAtomicCounterBuffers = */ 0,
    /* .MaxFragmentAtomicCounterBuffers = */ 1,
    /* .MaxCombinedAtomicCounterBuffers = */ 1,
    /* .MaxAtomicCounterBufferSize = */ 16384,
    /* .MaxTransformFeedbackBuffers = */ 4,
    /* .MaxTransformFeedbackInterleavedComponents = */ 64,
    /* .MaxCullDistances = */ 8,
    /* .MaxCombinedClipAndCullDistances = */ 8,
    /* .MaxSamples = */ 4,
    /* .maxMeshOutputVerticesNV = */ 256,
    /* .maxMeshOutputPrimitivesNV = */ 512,
    /* .maxMeshWorkGroupSizeX_NV = */ 32,
    /* .maxMeshWorkGroupSizeY_NV = */ 1,
    /* .maxMeshWorkGroupSizeZ_NV = */ 1,
    /* .maxTaskWorkGroupSizeX_NV = */ 32,
    /* .maxTaskWorkGroupSizeY_NV = */ 1,
    /* .maxTaskWorkGroupSizeZ_NV = */ 1,
    /* .maxMeshViewCountNV = */ 4,

    /* .limits = */ {
    /* .nonInductiveForLoops = */ 1,
    /* .whileLoops = */ 1,
    /* .doWhileLoops = */ 1,
    /* .generalUniformIndexing = */ 1,
    /* .generalAttributeMatrixVectorIndexing = */ 1,
    /* .generalVaryingIndexing = */ 1,
    /* .generalSamplerIndexing = */ 1,
    /* .generalVariableIndexing = */ 1,
    /* .generalConstantMatrixVectorIndexing = */ 1,
} };

/// Parse GLSL sources with glslang API and generate a SPIR-V binary blob.
/// See glslToHlsl() for parameter meanings.
///
std::vector<uint32_t> glslToSpirv(
    const std::string& glslPrivateUniforms,
    const std::string& glslFragment
)
{
    // All tools in the SPIR-V ecosystem operate on complete shader stages
    // with valid entry points. When providing source code for parsing, the
    // easiest workaround for this is to provide a dummy entry point.
    static const std::string dummyMain =
        "void main()\n"
        "{\n"
        "}\n\n";

    const char* shaderStrings[]{
        glslPrivateUniforms.data(),
        glslFragment.data(),
        dummyMain.data()
    };

    const int stringLengths[]{
        static_cast<int>(glslPrivateUniforms.size()),
        static_cast<int>(glslFragment.size()),
        static_cast<int>(dummyMain.size())
    };

    glslang::TShader shader(EShLangFragment);
    shader.setStringsWithLengths(shaderStrings, stringLengths, 3);
    shader.setEntryPoint("main");

    // Permits uniforms without explicit layout locations
    shader.setAutoMapLocations(true);

    constexpr auto messages = static_cast<EShMessages>(
        EShMsgSpvRules | EShMsgKeepUncalled | EShMsgDebugInfo
    );

    {
        // The MaterialX GLSL generator is hardcoded to output GLSL 4.0
        constexpr int defaultVersion = 400;
        constexpr bool forwardCompatible = false;
        
        // Don't support includes in the GLSL source.
        glslang::TShader::ForbidIncluder forbidIncluder;

        if (!shader.parse(
            &defaultTBuiltInResource,
            defaultVersion,
            forwardCompatible,
            messages,
            forbidIncluder
        ))
        {
            const char* const log = shader.getInfoLog();
            throw Exception(
                std::string("glslang failed to parse the GLSL fragment:\n") + log
            );
        }
    }

    glslang::TProgram program;
    program.addShader(&shader);
    if (!program.link(messages))
    {
        const char* const log = program.getInfoLog();
        throw Exception(
            std::string("glslang failed to link the GLSL fragment:\n") + log
        );
    }

    std::vector<uint32_t> spirv;

    {
        glslang::SpvOptions options;
        options.generateDebugInfo = true;   // necessary to preserve symbols
        options.disableOptimizer = true; 
        options.optimizeSize = false;

        glslang::GlslangToSpv(
            *program.getIntermediate(EShLangFragment), spirv, &options
        );
    }

    return spirv;
}

class HlslFragmentCrossCompiler : public spirv_cross::CompilerHLSL
{
public:
    using spirv_cross::CompilerHLSL::CompilerHLSL;

    /// Override this method to omit all uniforms except light data in the HLSL
    /// fragment. VP2 will generate these uniforms in the final HLSL shader
    /// based on XML wrapper properties, so emitting these would lead to double
    /// definition compilation errors.
    /// Please note that the base class method had to be made protected in our
    /// SPIRV-Cross fork to be able to delegate to it.
    /// The light data uniform needs to be emitted because it's not part of the
    /// fragment's XML interface. It's not actually used in the fragment so
    /// future Maya lighting integration work may remove the need for the
    /// SPIRV-Cross change.
    void emit_uniform(const spirv_cross::SPIRVariable& var) override
    {
        if (to_name(var.self) == HW::LIGHT_DATA_INSTANCE)
        {
            spirv_cross::CompilerHLSL::emit_uniform(var);
        }
    }
};

/// Generate HLSL fragment code from a SPIR-V binary blob.
std::string spirvToHlsl(
    std::vector<uint32_t>&& spirv,
    const std::string& fragmentName
)
{
    auto crossCompiler = std::make_unique<HlslFragmentCrossCompiler>(std::move(spirv));
    crossCompiler->set_entry_point("main", spv::ExecutionModelFragment);

    spirv_cross::CompilerHLSL::Options hlslOptions = crossCompiler->get_hlsl_options();

    // Shader model 5.0 is required by DirectX 11
    hlslOptions.shader_model = 50;

    // Another custom modification in our SPIRV-Cross fork. The cross-compiler
    // will generate code for call graphs rooted at the functions specified in
    // this list even if they are not called from the entry point.
    // The default behavior is to traverse from the entry point only.
    hlslOptions.exported_functions.insert(fragmentName);

    crossCompiler->set_hlsl_options(hlslOptions);
    return crossCompiler->compile();
}

} // anonymous namespace

void initialize()
{
    glslang::InitializeProcess();
}

void finalize()
{
    glslang::FinalizeProcess();
}

std::string glslToHlsl(
    const std::string& glslPrivateUniforms,
    const std::string& glslFragment,
    const std::string& fragmentName
)
{
    std::vector<uint32_t> spirv = glslToSpirv(glslPrivateUniforms, glslFragment);
    return spirvToHlsl(std::move(spirv), fragmentName);
}

}
}
