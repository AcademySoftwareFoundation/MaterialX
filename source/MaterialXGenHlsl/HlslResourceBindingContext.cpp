//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenHlsl/HlslResourceBindingContext.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/ShaderStage.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{

string formatRegisterSlot(char regClass, size_t index, const string& space)
{
    string slot = "register(";
    slot += regClass;
    slot += std::to_string(index);
    if (!space.empty())
    {
        slot += ", ";
        slot += space;
    }
    slot += ")";
    return slot;
}

} // namespace

//
// HlslResourceBindingContext methods
//

HlslResourceBindingContext::HlslResourceBindingContext(size_t cbufferRegister,
                                                       size_t textureRegister,
                                                       size_t samplerRegister) :
    _initCbufferRegister(cbufferRegister),
    _initTextureRegister(textureRegister),
    _initSamplerRegister(samplerRegister),
    _cbufferRegister(cbufferRegister),
    _textureRegister(textureRegister),
    _samplerRegister(samplerRegister)
{
}

void HlslResourceBindingContext::initialize()
{
    _cbufferRegister = _initCbufferRegister;
    _textureRegister = _initTextureRegister;
    _samplerRegister = _initSamplerRegister;
}

void HlslResourceBindingContext::emitDirectives(GenContext&, ShaderStage&)
{
    // HLSL does not require any preamble for explicit register binding.
}

void HlslResourceBindingContext::emitResourceBindings(GenContext& context, const VariableBlock& uniforms, ShaderStage& stage)
{
    const ShaderGenerator& generator = context.getShaderGenerator();
    const Syntax& syntax = generator.getSyntax();

    // Partition the block: HLSL forbids texture/sampler members inside a
    // cbuffer, so non-FILENAME uniforms go into the cbuffer and FILENAME
    // uniforms (our SamplerTexture2D handles) become separate top-level
    // resource declarations.
    bool hasValueUniforms = false;
    for (auto u : uniforms.getVariableOrder())
    {
        if (u->getType() != Type::FILENAME)
        {
            hasValueUniforms = true;
            break;
        }
    }

    if (hasValueUniforms)
    {
        const string slot = formatRegisterSlot('b', nextCbufferRegister(), _registerSpace);
        generator.emitLine("cbuffer " + uniforms.getName() + "_" + stage.getName() +
                           " : " + slot, stage, false);
        generator.emitScopeBegin(stage);
        for (auto u : uniforms.getVariableOrder())
        {
            if (u->getType() != Type::FILENAME)
            {
                generator.emitLineBegin(stage);
                generator.emitVariableDeclaration(u, syntax.getUniformQualifier(), context, stage, false);
                generator.emitString(Syntax::SEMICOLON, stage);
                generator.emitLineEnd(stage, false);
            }
        }
        generator.emitScopeEnd(stage, true);
        generator.emitLineBreak(stage);
    }

    // Texture / sampler handles are SamplerTexture2D structs at file scope.
    // DXC treats the contained Texture2D and SamplerState members as
    // independently-bound root parameters; we burn one t# and one s# per
    // SamplerTexture2D so the host's slot accounting matches.
    for (auto u : uniforms.getVariableOrder())
    {
        if (u->getType() == Type::FILENAME)
        {
            generator.emitLineBegin(stage);
            generator.emitVariableDeclaration(u, syntax.getUniformQualifier(), context, stage, false);
            generator.emitString(Syntax::SEMICOLON, stage);
            generator.emitLineEnd(stage, false);
            // Reserve the t# and s# slots that DXC will assign to the
            // struct's members. We do not annotate the struct with
            // register() because HLSL does not allow register() on
            // user-defined struct globals containing mixed resource types.
            (void)nextTextureRegister();
            (void)nextSamplerRegister();
        }
    }
}

void HlslResourceBindingContext::emitStructuredResourceBindings(GenContext& context, const VariableBlock& uniforms,
                                                                ShaderStage& stage, const std::string& structInstanceName,
                                                                const std::string& arraySuffix)
{
    const ShaderGenerator& generator = context.getShaderGenerator();
    const Syntax& syntax = generator.getSyntax();

    // Emit the struct definition.
    generator.emitLine("struct " + uniforms.getName(), stage, false);
    generator.emitScopeBegin(stage);
    for (size_t i = 0; i < uniforms.size(); ++i)
    {
        generator.emitLineBegin(stage);
        generator.emitVariableDeclaration(uniforms[i], EMPTY_STRING, context, stage, false);
        generator.emitString(Syntax::SEMICOLON, stage);
        generator.emitLineEnd(stage, false);
    }
    generator.emitScopeEnd(stage, true);
    generator.emitLineBreak(stage);

    // Wrap the struct array in a cbuffer with an explicit register slot.
    const string slot = formatRegisterSlot('b', nextCbufferRegister(), _registerSpace);
    generator.emitLine("cbuffer " + uniforms.getName() + "_" + stage.getName() +
                       " : " + slot, stage, false);
    generator.emitScopeBegin(stage);
    generator.emitLine(syntax.getUniformQualifier() + " " + uniforms.getName() + " " +
                       structInstanceName + arraySuffix, stage);
    generator.emitScopeEnd(stage, true);
}

MATERIALX_NAMESPACE_END
