//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenGlsl/WgslShaderGenerator.h>
#include <MaterialXGenGlsl/WgslSyntax.h>

#include <MaterialXGenHw/HwConstants.h>

MATERIALX_NAMESPACE_BEGIN

const string WgslShaderGenerator::LIGHTDATA_TYPEVAR_STRING = "light_type";

WgslShaderGenerator::WgslShaderGenerator(TypeSystemPtr typeSystem) :
    VkShaderGenerator(typeSystem)
{
    _syntax = WgslSyntax::create(typeSystem);
    
    // Set binding context to handle resource binding layouts
    _resourceBindingCtx = std::make_shared<MaterialX::WgslResourceBindingContext>(0);

    // For functions described in ::emitSpecularEnvironment()
    // override map value from HwShaderGenerator
    _tokenSubstitutions[HW::T_ENV_RADIANCE]             = HW::ENV_RADIANCE_SPLIT; 
    _tokenSubstitutions[HW::T_ENV_RADIANCE_SAMPLER2D]   = HW::ENV_RADIANCE_SAMPLER2D_SPLIT;
    _tokenSubstitutions[HW::T_ENV_IRRADIANCE]           = HW::ENV_IRRADIANCE_SPLIT;
    _tokenSubstitutions[HW::T_ENV_IRRADIANCE_SAMPLER2D] = HW::ENV_IRRADIANCE_SAMPLER2D_SPLIT;
    _tokenSubstitutions[HW::T_TEX_SAMPLER_SAMPLER2D]    = HW::TEX_SAMPLER_SAMPLER2D_SPLIT;
    _tokenSubstitutions[HW::T_TEX_SAMPLER_SIGNATURE]    = HW::TEX_SAMPLER_SIGNATURE_SPLIT;
}

void WgslShaderGenerator::emitDirectives(GenContext& context, ShaderStage& stage) const
{
    VkShaderGenerator::emitDirectives(context, stage);
    // Add additional directives and #define statements here
    //   Example: emitLine("#define HW_SEPARATE_SAMPLERS", stage, false);
    emitLineBreak(stage);
}

// Called by CompoundNode::emitFunctionDefinition()
void WgslShaderGenerator::emitFunctionDefinitionParameter(const ShaderPort* shaderPort, bool isOutput, GenContext& context, ShaderStage& stage) const
{
    if (shaderPort->getType() == Type::FILENAME)
    {
        emitString("texture2D " + shaderPort->getVariable() + "_texture, sampler "+shaderPort->getVariable() + "_sampler", stage);
    }
    else
    {
        VkShaderGenerator::emitFunctionDefinitionParameter(shaderPort, isOutput, context, stage);
    }
}

// Called by SourceCodeNode::emitFunctionCall()
void WgslShaderGenerator::emitInput(const ShaderInput* input, GenContext& context, ShaderStage& stage) const
{
    if (input->getType() == Type::FILENAME)
    {
        emitString(getUpstreamResult(input, context)+"_texture, "+getUpstreamResult(input, context)+"_sampler", stage);
    }
    else
    {
        VkShaderGenerator::emitInput(input, context, stage);
    }
}

MATERIALX_NAMESPACE_END
