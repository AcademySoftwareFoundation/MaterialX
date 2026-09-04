//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenWgsl/Nodes/WgslSourceCodeNode.h>

#include <MaterialXGenShader/Exception.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{

// Matches the inline-expression markers used by the shared SourceCodeNode.
const string INLINE_VARIABLE_PREFIX("{{");
const string INLINE_VARIABLE_SUFFIX("}}");

} // anonymous namespace

ShaderNodeImplPtr WgslSourceCodeNode::create()
{
    return std::make_shared<WgslSourceCodeNode>();
}

void WgslSourceCodeNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    // Only the inlined path needs WGSL-specific handling for its constant
    // temporaries; the ordinary function-call path defers to the base class,
    // whose argument emission already routes through the WGSL generator's
    // emitInput / emitOutput overrides.
    if (!_inlined)
    {
        SourceCodeNode::emitFunctionCall(node, context, stage);
        return;
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const ShaderGenerator& shadergen = context.getShaderGenerator();

        if (nodeOutputIsClosure(node))
        {
            // Emit calls for any closure dependencies upstream from this nodedef.
            shadergen.emitDependentFunctionCalls(node, context, stage, ShaderNode::Classification::CLOSURE);
        }

        size_t pos = 0;
        size_t i = _functionSource.find(INLINE_VARIABLE_PREFIX);
        StringSet variableNames;
        StringVec code;
        while (i != string::npos)
        {
            code.push_back(_functionSource.substr(pos, i - pos));

            size_t j = _functionSource.find(INLINE_VARIABLE_SUFFIX, i + 2);
            if (j == string::npos)
            {
                throw ExceptionShaderGenError("Malformed inline expression in implementation for node " + node.getName());
            }

            const string variable = _functionSource.substr(i + 2, j - i - 2);
            const ShaderInput* input = node.getInput(variable);
            if (!input)
            {
                throw ExceptionShaderGenError("Could not find an input named '" + variable +
                                              "' on node '" + node.getName() + "'");
            }

            if (input->getConnection())
            {
                code.push_back(shadergen.getUpstreamResult(input, context));
            }
            else
            {
                string variableName = node.getName() + "_" + input->getName() + "_tmp";
                if (!variableNames.count(variableName))
                {
                    // Emit the constant temporary in WGSL declaration order via the
                    // generator's emitVariableDeclaration override ("const name: type = value").
                    ShaderPort v(nullptr, input->getType(), variableName, input->getValue());
                    shadergen.emitLineBegin(stage);
                    shadergen.emitVariableDeclaration(&v, shadergen.getSyntax().getConstantQualifier(), context, stage);
                    shadergen.emitLineEnd(stage);
                    variableNames.insert(variableName);
                }
                code.push_back(variableName);
            }

            pos = j + 2;
            i = _functionSource.find(INLINE_VARIABLE_PREFIX, pos);
        }
        code.push_back(_functionSource.substr(pos));

        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        shadergen.emitString(" = ", stage);
        for (const string& c : code)
        {
            shadergen.emitString(c, stage);
        }
        shadergen.emitLineEnd(stage);
    }
}

MATERIALX_NAMESPACE_END
