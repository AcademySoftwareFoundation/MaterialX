//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenWgsl/Nodes/WgslCompoundNode.h>

#include <MaterialXGenHw/HwConstants.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr WgslCompoundNode::create()
{
    return std::make_shared<WgslCompoundNode>();
}

void WgslCompoundNode::emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const ShaderGenerator& shadergen = context.getShaderGenerator();

        // Emit function definitions for all child nodes.
        shadergen.emitFunctionDefinitions(*_rootGraph, context, stage);

        shadergen.emitLineBegin(stage);
        shadergen.emitString("fn " + _functionName + "(", stage);

        // Closure data parameter (emits a trailing ", " when present).
        shadergen.emitClosureDataParameter(node, context, stage);

        // Thread vertex data into closure/surface-shader functions.
        const bool needsVertexData = nodeOutputIsClosure(node);
        if (needsVertexData)
        {
            const VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
            const string vdTypeName = vertexData.empty() ? string("VertexData") : vertexData.getName();
            shadergen.emitString(vertexData.getInstance() + ": " + vdTypeName + ", ", stage);
        }

        string delim;
        for (ShaderGraphInputSocket* inputSocket : _rootGraph->getInputSockets())
        {
            shadergen.emitString(delim, stage);
            shadergen.emitFunctionDefinitionParameter(inputSocket, false, context, stage);
            delim = ", ";
        }

        // Output parameters become ptr<function, T> so the caller passes &outVar.
        for (ShaderGraphOutputSocket* outputSocket : _rootGraph->getOutputSockets())
        {
            const string typeName = shadergen.getSyntax().getTypeName(outputSocket->getType());
            shadergen.emitString(delim + outputSocket->getVariable() + ": ptr<function, " + typeName + ">", stage);
            delim = ", ";
        }

        shadergen.emitString(")", stage);
        shadergen.emitLineEnd(stage, false);

        shadergen.emitFunctionBodyBegin(*_rootGraph, context, stage);

        if (nodeOutputIsClosure(node))
        {
            shadergen.emitFunctionCalls(*_rootGraph, context, stage, ShaderNode::Classification::TEXTURE);
            for (ShaderGraphOutputSocket* outputSocket : _rootGraph->getOutputSockets())
            {
                if (outputSocket->getConnection())
                {
                    const ShaderNode* upstream = outputSocket->getConnection()->getNode();
                    if (upstream->getParent() == _rootGraph.get() &&
                        (upstream->hasClassification(ShaderNode::Classification::CLOSURE) ||
                         upstream->hasClassification(ShaderNode::Classification::SHADER) ||
                         upstream->hasClassification(ShaderNode::Classification::MATERIAL)))
                    {
                        shadergen.emitFunctionCall(*upstream, context, stage);
                    }
                }
            }
        }
        else
        {
            shadergen.emitFunctionCalls(*_rootGraph, context, stage);
        }

        // Write results through the output pointers.
        for (ShaderGraphOutputSocket* outputSocket : _rootGraph->getOutputSockets())
        {
            const string result = shadergen.getUpstreamResult(outputSocket, context);
            shadergen.emitLine("(*" + outputSocket->getVariable() + ") = " + result, stage);
        }

        shadergen.emitFunctionBodyEnd(*_rootGraph, context, stage);
    }
}

void WgslCompoundNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const ShaderGenerator& shadergen = context.getShaderGenerator();

    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        shadergen.emitFunctionCalls(*_rootGraph, context, stage);
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        if (nodeOutputIsClosure(node))
        {
            shadergen.emitDependentFunctionCalls(node, context, stage, ShaderNode::Classification::CLOSURE);
        }

        // Declare the output variables.
        emitOutputVariables(node, context, stage);

        shadergen.emitLineBegin(stage);
        shadergen.emitString(_functionName + "(", stage);

        // Closure data argument (emits trailing ", " when present).
        shadergen.emitClosureDataArg(node, context, stage);

        // Pass the vertex data instance into closure/surface-shader functions.
        if (nodeOutputIsClosure(node))
        {
            const VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
            shadergen.emitString(vertexData.getInstance() + ", ", stage);
        }

        string delim;
        for (ShaderInput* input : node.getInputs())
        {
            shadergen.emitString(delim, stage);
            shadergen.emitInput(input, context, stage);
            delim = ", ";
        }
        for (size_t i = 0; i < node.numOutputs(); ++i)
        {
            shadergen.emitString(delim, stage);
            shadergen.emitOutput(node.getOutput(i), false, false, context, stage);
            delim = ", ";
        }

        shadergen.emitString(")", stage);
        shadergen.emitLineEnd(stage);
    }
}

MATERIALX_NAMESPACE_END
