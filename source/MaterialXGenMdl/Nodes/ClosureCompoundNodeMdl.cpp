//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenMdl/Nodes/ClosureCompoundNodeMdl.h>

#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <MaterialXCore/Definition.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr ClosureCompoundNodeMdl::create()
{
    return std::make_shared<ClosureCompoundNodeMdl>();
}

void ClosureCompoundNodeMdl::addClassification(ShaderNode& node) const
{
    // Add classification from the graph implementation.
    node.addClassification(_rootGraph->getClassification());
}

void ClosureCompoundNodeMdl::emitFunctionDefinition(const ShaderNode&, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        const Syntax& syntax = shadergen.getSyntax();

        const bool isMaterialExpr = (
            _rootGraph->hasClassification(ShaderNode::Classification::CLOSURE) ||
            _rootGraph->hasClassification(ShaderNode::Classification::SHADER)
        );

        // Emit functions for all child nodes
        shadergen.emitFunctionDefinitions(*_rootGraph, context, stage);

        if (!_returnStruct.empty())
        {
            // Define the output struct.
            shadergen.emitLine("struct " + _returnStruct, stage, false);
            shadergen.emitScopeBegin(stage, Syntax::CURLY_BRACKETS);
            for (const ShaderGraphOutputSocket* output : _rootGraph->getOutputSockets())
            {
                shadergen.emitLine(syntax.getTypeName(output->getType()) + " mxp_" + output->getName(), stage);
            }
            shadergen.emitScopeEnd(stage, true);
            shadergen.emitLineBreak(stage);

            // Begin function signature.
            shadergen.emitLine(_returnStruct + " " + _functionName, stage, false);
        }
        else
        {
            // Begin function signature.
            const ShaderGraphOutputSocket* outputSocket = _rootGraph->getOutputSocket();
            const string& outputType = syntax.getTypeName(outputSocket->getType());
            shadergen.emitLine(outputType + " " + _functionName, stage, false);
        }

        shadergen.emitScopeBegin(stage, Syntax::PARENTHESES);

        const string uniformPrefix = syntax.getUniformQualifier() + " ";

        // Emit all inputs
        int count = int(_rootGraph->numInputSockets());
        for (ShaderGraphInputSocket* input : _rootGraph->getInputSockets())
        {
            const string& qualifier = input->isUniform() || input->getType() == Type::FILENAME ? uniformPrefix : EMPTY_STRING;
            const string& type = syntax.getTypeName(input->getType());
            const string value = (input->getValue() ?
                syntax.getValue(input->getType(), *input->getValue()) :
                syntax.getDefaultValue(input->getType()));

            const string& delim = --count > 0 ? Syntax::COMMA : EMPTY_STRING;
            shadergen.emitLine(qualifier + type + " " + input->getVariable() + " = " + value + delim, stage, false);
        }

        // End function signature.
        shadergen.emitScopeEnd(stage);

        // Special case for material expresions.
        if (isMaterialExpr)
        {
            shadergen.emitLine(" = let", stage, false);
        }

        // Function body.
        shadergen.emitScopeBegin(stage);

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
        if (isMaterialExpr)
        {
            shadergen.emitScopeEnd(stage);
            const ShaderGraphOutputSocket* outputSocket = _rootGraph->getOutputSocket();
            const string result = shadergen.getUpstreamResult(outputSocket, context);
            shadergen.emitLine("in material(" + result + ")", stage);
        }
        else
        {
            if (!_returnStruct.empty())
            {
                const string resultVariableName = "result__";
                shadergen.emitLine(_returnStruct + " " + resultVariableName, stage);
                for (const ShaderGraphOutputSocket* output : _rootGraph->getOutputSockets())
                {
                    const string result = shadergen.getUpstreamResult(output, context);
                    shadergen.emitLine(resultVariableName + ".mxp_" + output->getName() + " = " + result, stage);
                }
                shadergen.emitLine("return " + resultVariableName, stage);
            }
            else
            {
                const ShaderGraphOutputSocket* outputSocket = _rootGraph->getOutputSocket();
                const string result = shadergen.getUpstreamResult(outputSocket, context);
                shadergen.emitLine("return " + result, stage);
            }
            shadergen.emitScopeEnd(stage);
        }

        shadergen.emitLineBreak(stage);

    END_SHADER_STAGE(stage, Stage::PIXEL)
}

void ClosureCompoundNodeMdl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        const ShaderGenerator& shadergen = context.getShaderGenerator();

        // Emit calls for any closure dependencies upstream from this node.
        shadergen.emitDependentFunctionCalls(node, context, stage, ShaderNode::Classification::CLOSURE);

        // Begin function call.
        if (!_returnStruct.empty())
        {
            // Emit the struct multioutput.
            const string resultVariableName = node.getName() + "_result";
            shadergen.emitLineBegin(stage);
            shadergen.emitString(_returnStruct + " " + resultVariableName + " = ", stage);
        }
        else
        {
            // Emit the single output.
            shadergen.emitLineBegin(stage);
            shadergen.emitOutput(node.getOutput(0), true, false, context, stage);
            shadergen.emitString(" = ", stage);
        }

        shadergen.emitString(_functionName + "(", stage);

        // Emit inputs.
        string delim = "";
        for (ShaderInput* input : node.getInputs())
        {
            shadergen.emitString(delim, stage);
            shadergen.emitInput(input, context, stage);
            delim = ", ";
        }

        // End function call
        shadergen.emitString(")", stage);
        shadergen.emitLineEnd(stage);
    END_SHADER_STAGE(stage, Stage::PIXEL)
}

MATERIALX_NAMESPACE_END
