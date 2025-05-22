//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenGlsl/WgslShaderGenerator.h>
#include <MaterialXGenGlsl/WgslSyntax.h>

#include <MaterialXGenShader/Nodes/HwImageNode.h>

MATERIALX_NAMESPACE_BEGIN

const string WgslShaderGenerator::TARGET = "genglsl_wgsl";
const string WgslShaderGenerator::VERSION = "450";

WgslShaderGenerator::WgslShaderGenerator(TypeSystemPtr typeSystem) :
    VkShaderGenerator(typeSystem)
{
    _syntax = WgslSyntax::create(typeSystem);
    
    // Set binding context to handle resource binding layouts
    _resourceBindingCtx = std::make_shared<MaterialX::WgslResourceBindingContext>(0);

    // For functions described in ::emitSpecularEnvironment()
    _tokenSubstitutions[HW::T_ENV_RADIANCE+"_texture"] = HW::ENV_RADIANCE+"_texture";
    _tokenSubstitutions[HW::T_ENV_RADIANCE+"_sampler"] = HW::ENV_RADIANCE+"_sampler";
    _tokenSubstitutions[HW::T_ENV_IRRADIANCE+"_texture"] = HW::ENV_IRRADIANCE+"_texture";
    _tokenSubstitutions[HW::T_ENV_IRRADIANCE+"_sampler"] = HW::ENV_IRRADIANCE+"_sampler";

    // image overrides
    // Registering these Implementations as done with Implementations in GlslShaderGenerator
    // See also stdlib_genglsl_impl.mtlx
    StringVec elementNames = {
        "IM_image_float_" + WgslShaderGenerator::TARGET,
        "IM_image_color3_" + WgslShaderGenerator::TARGET,
        "IM_image_color4_" + WgslShaderGenerator::TARGET,
        "IM_image_vector2_" + WgslShaderGenerator::TARGET,
        "IM_image_vector3_" + WgslShaderGenerator::TARGET,
        "IM_image_vector4_" + WgslShaderGenerator::TARGET,
    };
    registerImplementation(elementNames, HwImageNode::create);
}


// Called by SourceCodeNode::emitFunctionCall()
// TODO: Refactor CompoundNode::emitFunctionDefinition to have a hook for ShaderGenerator class.
void WgslShaderGenerator::emitInput(const ShaderInput* input, GenContext& context, ShaderStage& stage) const
{
    if (input->getType() == Type::FILENAME) {
        emitString(getUpstreamResult(input, context)+"_texture, "+getUpstreamResult(input, context)+"_sampler", stage);
    }
    else {
        VkShaderGenerator::emitInput(input, context, stage);
    }
}


void WgslShaderGenerator::emitSpecularEnvironment(GenContext& context, ShaderStage& stage) const
{
    int specularMethod = context.getOptions().hwSpecularEnvironmentMethod;
    if (specularMethod == SPECULAR_ENVIRONMENT_FIS)
    {
        emitLibraryInclude("pbrlib/genglsl/lib/mx_latlong_wgsl.glsl", context, stage);
        emitLineBreak(stage);
        emitLibraryInclude("pbrlib/genglsl/lib/mx_environment_fis_wgsl.glsl", context, stage);
    }
    else if (specularMethod == SPECULAR_ENVIRONMENT_PREFILTER)
    {
        emitLibraryInclude("pbrlib/genglsl/lib/mx_latlong_wgsl.glsl", context, stage);
        emitLineBreak(stage);
        emitLibraryInclude("pbrlib/genglsl/lib/mx_environment_prefilter_wgsl.glsl", context, stage);
    }
    else if (specularMethod == SPECULAR_ENVIRONMENT_NONE)
    {
        emitLibraryInclude("pbrlib/genglsl/lib/mx_environment_none.glsl", context, stage);
    }
    else
    {
        throw ExceptionShaderGenError("Invalid hardware specular environment method specified: '" + std::to_string(specularMethod) + "'");
    }
    emitLineBreak(stage);
}


HwResourceBindingContextPtr WgslShaderGenerator::getResourceBindingContext(GenContext& /*context*/) const
{
    return _resourceBindingCtx;
}

MATERIALX_NAMESPACE_END
