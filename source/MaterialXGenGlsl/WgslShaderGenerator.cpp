//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenGlsl/WgslShaderGenerator.h>
#include <MaterialXGenGlsl/WgslSyntax.h>

#include <MaterialXGenHw/HwConstants.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/Util.h>

MATERIALX_NAMESPACE_BEGIN

const string WgslShaderGenerator::TARGET = "wgsl";
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

// Called by SourceCodeNode::emitFunctionCall() and CompoundNode::emitFunctionCall()
void WgslShaderGenerator::emitInput(const ShaderInput* input, GenContext& context, ShaderStage& stage) const
{
    if (input->getType() == Type::FILENAME)
    {
        emitString(getUpstreamResult(input, context)+"_texture, "+getUpstreamResult(input, context)+"_sampler", stage);
    }
    else if (input->getType() == Type::BOOLEAN)
    {
        const string result = getUpstreamResult(input, context);
        emitString("bool(" + result + ")", stage);
    }
    else
    {
        VkShaderGenerator::emitInput(input, context, stage);
    }
}

void WgslShaderGenerator::replaceTokens(const StringMap& substitutions, ShaderStage& stage) const
{
    // Bool-as-int uniform tokens. Add new entries when introducing more bool uniforms.
    // Local static avoids static initialization order issues with extern HW:: constants.
    static const vector<std::pair<string, string>> boolUniformTokens = {
        { HW::T_REFRACTION_TWO_SIDED, HW::REFRACTION_TWO_SIDED },
    };

    // Source code: bool-as-int uniform tokens get wrapped in bool() so that
    // uses like "if ($refractionTwoSided)" become "if (bool(u_refractionTwoSided))".
    const StringMap codeSubstitutions = [&]() {
        StringMap subs = substitutions;
        for (const auto& entry : boolUniformTokens)
            subs[entry.first] = "bool(" + entry.second + ")";
        return subs;
    }();

    string code = stage.getSourceCode();
    tokenSubstitution(codeSubstitutions, code);
    stage.setSourceCode(code);

    // Interface ports: bool-as-int uniform tokens stay as plain names so that
    // uniform declarations and application-side binding remain correct.
    const StringMap portSubstitutions = [&]() {
        StringMap subs = substitutions;
        for (const auto& entry : boolUniformTokens)
            subs[entry.first] = entry.second;
        return subs;
    }();

    auto replacePorts = [&portSubstitutions](VariableBlock& block)
    {
        for (size_t i = 0; i < block.size(); ++i)
        {
            ShaderPort* port = block[i];
            string name = port->getName();
            tokenSubstitution(portSubstitutions, name);
            port->setName(name);
            string variable = port->getVariable();
            tokenSubstitution(portSubstitutions, variable);
            port->setVariable(variable);
        }
    };

    replacePorts(stage.getConstantBlock());
    for (const auto& it : stage.getUniformBlocks())
        replacePorts(*it.second);
    for (const auto& it : stage.getInputBlocks())
        replacePorts(*it.second);
    for (const auto& it : stage.getOutputBlocks())
        replacePorts(*it.second);
}

MATERIALX_NAMESPACE_END
