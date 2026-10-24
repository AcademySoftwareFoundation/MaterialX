//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenWgsl/WgslResourceBindingContext.h>
#include <MaterialXGenWgsl/WgslSyntax.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/Syntax.h>
#include <MaterialXGenShader/Util.h>

MATERIALX_NAMESPACE_BEGIN

WgslResourceBindingContext::WgslResourceBindingContext(size_t group) :
    _group(group),
    _binding(0)
{
}

void WgslResourceBindingContext::initialize()
{
    _binding = 0;
}

void WgslResourceBindingContext::emitDirectives(GenContext&, ShaderStage&)
{
    // WGSL has no preprocessor directives for resource binding.
}

void WgslResourceBindingContext::emitResourceBindings(GenContext& context, const VariableBlock& uniforms, ShaderStage& stage)
{
    const ShaderGenerator& generator = context.getShaderGenerator();
    const WgslSyntax& syntax = static_cast<const WgslSyntax&>(generator.getSyntax());
    const StringMap& tokenSubs = generator.getTokenSubstitutions();
    const string groupStr = std::to_string(_group);

    // Separate textures (individual bindings) from value uniforms (struct-packed UBO).
    std::vector<const ShaderPort*> textureUniforms;
    std::vector<const ShaderPort*> valueUniforms;
    for (const ShaderPort* uniform : uniforms.getVariableOrder())
    {
        const TypeDesc type = uniform->getType();
        if (type.isClosure())
            continue;
        if (type == Type::FILENAME)
            textureUniforms.push_back(uniform);
        else
            valueUniforms.push_back(uniform);
    }

    // Emit texture + sampler pairs as individual bindings.
    for (const ShaderPort* uniform : textureUniforms)
    {
        const string& name = uniform->getVariable();
        generator.emitLine("@group(" + groupStr + ") @binding(" + std::to_string(_binding++) +
                               ") var " + name + "_texture: texture_2d<f32>",
                           stage);
        generator.emitLine("@group(" + groupStr + ") @binding(" + std::to_string(_binding++) +
                               ") var " + name + "_sampler: sampler",
                           stage);
    }

    // Pack all value uniforms into a single struct bound once.
    if (!valueUniforms.empty())
    {
        const string structName = uniforms.getName();
        const string instanceName = uniforms.getInstance();

        // Resolve $-token variable names to literal identifiers for struct members. Tokens may be
        // prefixes of the variable name (e.g. "$geomprop_<name>"), so substitute as source code does.
        const string qualPrefix = instanceName + ".";
        bool hasArray = false;
        generator.emitLine("struct " + structName + " ", stage, false);
        generator.emitScopeBegin(stage);
        for (size_t i = 0; i < valueUniforms.size(); ++i)
        {
            const ShaderPort* port = valueUniforms[i];
            string memberName = port->getVariable();
            tokenSubstitution(tokenSubs, memberName);
            // Strip struct instance prefix added by token overrides (e.g. "u_prv.u_envMatrix" -> "u_envMatrix").
            if (memberName.size() > qualPrefix.size() &&
                memberName.compare(0, qualPrefix.size(), qualPrefix) == 0)
            {
                memberName = memberName.substr(qualPrefix.size());
            }
            string typeName = syntax.getTypeName(port->getType());
            if (port->getType() == Type::BOOLEAN)
                typeName = "u32";
            else if (WgslSyntax::structContainsBool(port->getType()))
                typeName = WgslSyntax::getHostStructName(port->getType());
            if (port->getType().isArray() && port->getValue())
            {
                typeName = syntax.getArrayTypeName(port->getType(), *port->getValue());
                hasArray = true;
            }
            const string comma = (i + 1 < valueUniforms.size()) ? "," : "";
            generator.emitLine("    " + memberName + ": " + typeName + comma, stage, false);
        }
        generator.emitScopeEnd(stage, false, false);
        generator.emitLineBreak(stage);

        // Bind the struct as a single buffer; scalar arrays need storage (uniform requires 16-byte strides).
        const string addressSpace = hasArray ? "<storage, read>" : "<uniform>";
        generator.emitLine("@group(" + groupStr + ") @binding(" + std::to_string(_binding++) +
                               ") var" + addressSpace + " " + instanceName + ": " + structName,
                           stage);
        generator.emitLineBreak(stage);
    }

    generator.emitLineBreak(stage);
}

void WgslResourceBindingContext::emitStructuredResourceBindings(GenContext& context, const VariableBlock& uniforms,
                                                                ShaderStage& stage, const string& structInstanceName,
                                                                const string& arraySuffix)
{
    const ShaderGenerator& generator = context.getShaderGenerator();
    const WgslSyntax& syntax = static_cast<const WgslSyntax&>(generator.getSyntax());
    const string groupStr = std::to_string(_group);

    // Emit the element struct definition.
    generator.emitLine("struct " + uniforms.getName() + " ", stage, false);
    generator.emitScopeBegin(stage);
    const auto& order = uniforms.getVariableOrder();
    for (size_t i = 0; i < order.size(); ++i)
    {
        const ShaderPort* port = order[i];
        string typeName = syntax.getTypeName(port->getType());
        if (port->getType() == Type::BOOLEAN)
        {
            typeName = "u32";
        }
        if (port->getType().isArray() && port->getValue())
        {
            typeName = syntax.getArrayTypeName(port->getType(), *port->getValue());
        }
        const string comma = (i + 1 < order.size()) ? "," : "";
        generator.emitLine("    " + port->getVariable() + ": " + typeName + comma, stage, false);
    }
    generator.emitScopeEnd(stage, false, false);
    generator.emitLineBreak(stage);

    // Convert a GLSL-style "[N]" suffix into a WGSL array<T, N> store type.
    string storeType = uniforms.getName();
    if (!arraySuffix.empty() && arraySuffix.front() == '[' && arraySuffix.back() == ']')
    {
        const string count = arraySuffix.substr(1, arraySuffix.size() - 2);
        storeType = "array<" + uniforms.getName() + ", " + count + ">";
    }

    // Use storage (not uniform) address space: uniform requires 16-byte aligned strides.
    generator.emitLine("@group(" + groupStr + ") @binding(" + std::to_string(_binding++) +
                           ") var<storage, read> " + structInstanceName + ": " + storeType,
                       stage);
    generator.emitLineBreak(stage);
}

MATERIALX_NAMESPACE_END
