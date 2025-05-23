//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenGlsl/WgslShaderGenerator.h>
#include <MaterialXGenGlsl/WgslSyntax.h>

#include <MaterialXGenShader/Nodes/HwImageNode.h>

MATERIALX_NAMESPACE_BEGIN

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
}

void WgslShaderGenerator::emitDirectives(GenContext& context, ShaderStage& stage) const
{
    VkShaderGenerator::emitDirectives(context, stage);
    emitLine("#define HW_SEPARATE_SAMPLERS", stage, false);
    emitLineBreak(stage);
}

// Called by CompoundNode::emitFunctionDefinition()
void WgslShaderGenerator::emitFunctionDefinitionParameter(const ShaderPort* shaderPort, GenContext& context, ShaderStage& stage) const
{
    if (shaderPort->getType() == Type::FILENAME) {
        emitString("texture2D " + shaderPort->getVariable() + "_texture, sampler "+shaderPort->getVariable() + "_sampler", stage);
    }
    else {
        VkShaderGenerator::emitFunctionDefinitionParameter(shaderPort, context, stage);
    }
}

// Called by SourceCodeNode::emitFunctionCall()
void WgslShaderGenerator::emitInput(const ShaderInput* input, GenContext& context, ShaderStage& stage) const
{
    if (input->getType() == Type::FILENAME) {
        emitString(getUpstreamResult(input, context)+"_texture, "+getUpstreamResult(input, context)+"_sampler", stage);
    }
    else {
        VkShaderGenerator::emitInput(input, context, stage);
    }
}

HwResourceBindingContextPtr WgslShaderGenerator::getResourceBindingContext(GenContext& /*context*/) const
{
    return _resourceBindingCtx;
}

MATERIALX_NAMESPACE_END
