//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/Nodes/ClosureCompoundNode.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr ClosureCompoundNode::create()
{
    return std::make_shared<ClosureCompoundNode>();
}

void ClosureCompoundNode::addClassification(ShaderNode& node) const
{
    // Add classification from the graph implementation.
    node.addClassification(_rootGraph->getClassification());
}

void ClosureCompoundNode::emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        const Syntax& syntax = shadergen.getSyntax();

        // Emit functions for all child nodes
        shadergen.emitFunctionDefinitions(*_rootGraph, context, stage);

        string delim = "";

        // Begin function signature
        shadergen.emitLineBegin(stage);

        shadergen.emitString("void " + _functionName  + "(", stage);

        if (context.getShaderGenerator().nodeNeedsClosureData(node))
        {
            shadergen.emitString(delim + HW::CLOSURE_DATA_TYPE + " " + HW::CLOSURE_DATA_ARG + ", ", stage);
        }

        // Add all inputs
        for (ShaderGraphInputSocket* inputSocket : _rootGraph->getInputSockets())
        {
            shadergen.emitString(delim + syntax.getTypeName(inputSocket->getType()) + " " + inputSocket->getVariable(), stage);
            delim = ", ";
        }

        // Add all outputs
        for (ShaderGraphOutputSocket* outputSocket : _rootGraph->getOutputSockets())
        {
            shadergen.emitString(delim + syntax.getOutputTypeName(outputSocket->getType()) + " " + outputSocket->getVariable(), stage);
            delim = ", ";
        }

        // End function signature
        shadergen.emitString(")", stage);
        shadergen.emitLineEnd(stage, false);

        // Begin function body
        shadergen.emitFunctionBodyBegin(*_rootGraph, context, stage);

        // Emit all texturing nodes. These are inputs to the
        // closure nodes and need to be emitted first.
        shadergen.emitFunctionCalls(*_rootGraph, context, stage, ShaderNode::Classification::TEXTURE);

        // Emit function calls for internal closures nodes connected to the graph sockets.
        // These will in turn emit function calls for any dependent closure nodes upstream.
        for (ShaderGraphOutputSocket* outputSocket : _rootGraph->getOutputSockets())
        {
            if (outputSocket->getConnection())
            {
                const ShaderNode* upstream = outputSocket->getConnection()->getNode();
                if (upstream->getParent() == _rootGraph.get() &&
                    (upstream->hasClassification(ShaderNode::Classification::CLOSURE) || upstream->hasClassification(ShaderNode::Classification::SHADER)))
                {
                    shadergen.emitFunctionCall(*upstream, context, stage);
                }
            }
        }

        // Emit final results
        for (ShaderGraphOutputSocket* outputSocket : _rootGraph->getOutputSockets())
        {
            const string result = shadergen.getUpstreamResult(outputSocket, context);
            shadergen.emitLine(outputSocket->getVariable() + " = " + result, stage);
        }

        // End function body
        shadergen.emitFunctionBodyEnd(*_rootGraph, context, stage);
    }
}
void ClosureCompoundNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const ShaderGenerator& shadergen = context.getShaderGenerator();

    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        // Emit function calls for all child nodes to the vertex shader stage
        shadergen.emitFunctionCalls(*_rootGraph, context, stage);
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        // Emit calls for any closure dependencies upstream from this node.
        shadergen.emitDependentFunctionCalls(node, context, stage, ShaderNode::Classification::CLOSURE);

        // Declare the output variables
        emitOutputVariables(node, context, stage);

        shadergen.emitLineBegin(stage);
        string delim = "";

        // Emit function name.
        shadergen.emitString(_functionName + "(", stage);

        // Check if we have a closure context to modify the function call.
        if (context.getShaderGenerator().nodeNeedsClosureData(node))
        {
            shadergen.emitString(delim + HW::CLOSURE_DATA_ARG + ", ", stage);
        }

        // Emit all inputs.
        for (ShaderInput* input : node.getInputs())
        {
            shadergen.emitString(delim, stage);
            shadergen.emitInput(input, context, stage);
            delim = ", ";
        }

        // Emit all outputs.
        for (size_t i = 0; i < node.numOutputs(); ++i)
        {
            shadergen.emitString(delim, stage);
            shadergen.emitOutput(node.getOutput(i), false, false, context, stage);
            delim = ", ";
        }

        // End function call
        shadergen.emitString(")", stage);
        shadergen.emitLineEnd(stage);
    }
}

MATERIALX_NAMESPACE_END
