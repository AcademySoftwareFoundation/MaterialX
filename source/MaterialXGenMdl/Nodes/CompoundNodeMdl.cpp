//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenMdl/Nodes/CompoundNodeMdl.h>

#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <MaterialXCore/Definition.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr CompoundNodeMdl::create()
{
    return std::make_shared<CompoundNodeMdl>();
}

void CompoundNodeMdl::initialize(const InterfaceElement& element, GenContext& context)
{
    CompoundNode::initialize(element, context);

    _returnStruct = EMPTY_STRING;
    if (_rootGraph->getOutputSockets().size() > 1)
    {
        _returnStruct = _functionName + "__result";
    }
}

void CompoundNodeMdl::emitFunctionDefinition(const ShaderNode&, GenContext& context, ShaderStage& stage) const
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
        shadergen.emitFunctionCalls(*_rootGraph, context, stage);

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

void CompoundNodeMdl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        const ShaderGenerator& shadergen = context.getShaderGenerator();

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
