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

const string INLINE_VARIABLE_PREFIX("{{");
const string INLINE_VARIABLE_SUFFIX("}}");

// Shader expression for a connected inline placeholder; boolean ports are wrapped in bool().
string inlineInputExpression(const ShaderInput* input, const ShaderGenerator& shadergen, GenContext& context)
{
    string result = shadergen.getUpstreamResult(input, context);
    if (input->getType() == Type::BOOLEAN)
    {
        result = "bool(" + result + ")";
    }
    return result;
}

} // anonymous namespace

ShaderNodeImplPtr WgslSourceCodeNode::create()
{
    return std::make_shared<WgslSourceCodeNode>();
}

void WgslSourceCodeNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
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
                code.push_back(inlineInputExpression(input, shadergen, context));
            }
            else
            {
                string variableName = node.getName() + "_" + input->getName() + "_tmp";
                if (!variableNames.count(variableName))
                {
                    ShaderPort v(nullptr, input->getType(), variableName, input->getValue());
                    shadergen.emitLineBegin(stage);
                    shadergen.emitVariableDeclaration(&v, shadergen.getSyntax().getConstantQualifier(), context, stage);
                    shadergen.emitLineEnd(stage);
                    variableNames.insert(variableName);
                }
                code.push_back(input->getType() == Type::BOOLEAN ? "bool(" + variableName + ")" : variableName);
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
