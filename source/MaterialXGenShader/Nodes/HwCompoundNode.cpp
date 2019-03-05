#include <MaterialXGenShader/Nodes/HwCompoundNode.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <MaterialXCore/Library.h>
#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Document.h>

namespace MaterialX
{

ShaderNodeImplPtr HwCompoundNode::create()
{
    return std::make_shared<HwCompoundNode>();
}

void HwCompoundNode::emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
        const HwShaderGenerator& shadergen = static_cast<const HwShaderGenerator&>(context.getShaderGenerator());

        // Emit functions for all child nodes
        shadergen.emitFunctionDefinitions(*_rootGraph, context, stage);

        // Find any closure contexts used by this node
        // and emit the function for each context.
        vector<HwClosureContextPtr> ccxs;
        shadergen.getNodeClosureContexts(node, ccxs);
        if (ccxs.empty())
        {
            emitFunctionDefinition(nullptr, context, stage);
        }
        else
        {
            for (HwClosureContextPtr ccx : ccxs)
            {
                emitFunctionDefinition(ccx, context, stage);
            }
        }
    END_SHADER_STAGE(stage, HW::PIXEL_STAGE)
}

void HwCompoundNode::emitFunctionDefinition(HwClosureContextPtr ccx, GenContext& context, ShaderStage& stage) const
{
    const HwShaderGenerator& shadergen = static_cast<const HwShaderGenerator&>(context.getShaderGenerator());
    const Syntax& syntax = shadergen.getSyntax();

    string delim = "";

    // Begin function signature
    shadergen.emitLineBegin(stage);
    if (ccx)
    {
        shadergen.emitString("void " + _functionName + ccx->getSuffix() + "(", stage);

        // Add any extra argument inputs first
        for (const HwClosureContext::Argument& arg : ccx->getArguments())
        {
            const string& type = syntax.getTypeName(arg.first);
            shadergen.emitString(delim + type + " " + arg.second, stage);
            delim = ", ";
        }
    }
    else
    {
        shadergen.emitString("void " + _functionName + "(", stage);
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

    // End function sginature
    shadergen.emitString(")", stage);
    shadergen.emitLineEnd(stage, false);

    // Begin function body
    shadergen.emitScopeBegin(stage);

    if (ccx)
    {
        context.pushUserData(HW::USER_DATA_CLOSURE_CONTEXT, ccx);
        shadergen.emitFunctionCalls(*_rootGraph, context, stage);
        context.popUserData(HW::USER_DATA_CLOSURE_CONTEXT);
    }
    else
    {
        shadergen.emitFunctionCalls(*_rootGraph, context, stage);
    }

    // Emit final results
    for (ShaderGraphOutputSocket* outputSocket : _rootGraph->getOutputSockets())
    {
        // Check for the rare case where the output is not internally connected
        if (!outputSocket->getConnection())
        {
            shadergen.emitLine(outputSocket->getVariable() + " = " + (outputSocket->getValue() ?
                syntax.getValue(outputSocket->getType(), *outputSocket->getValue()) :
                syntax.getDefaultValue(outputSocket->getType())), stage);
        }
        else
        {
            shadergen.emitLine(outputSocket->getVariable() + " = " + outputSocket->getConnection()->getVariable(), stage);
        }
    }

    // End function body
    shadergen.emitScopeEnd(stage);
    shadergen.emitLineBreak(stage);
}

void HwCompoundNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const ShaderGenerator& shadergen = context.getShaderGenerator();

    BEGIN_SHADER_STAGE(stage, HW::VERTEX_STAGE)
        // Emit function calls for all child nodes to the vertex shader stage
        // TODO: Is this ever usefull?
        shadergen.emitFunctionCalls(*_rootGraph, context, stage);
    END_SHADER_STAGE(stage, HW::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
        // Declare the output variables
        for (size_t i = 0; i < node.numOutputs(); ++i)
        {
            shadergen.emitLineBegin(stage);
            shadergen.emitOutput(node.getOutput(i), true, true, context, stage);
            shadergen.emitLineEnd(stage);
        }

        shadergen.emitLineBegin(stage);
        string delim = "";

        // Check if we have a closure context to modify the function call.
        HwClosureContextPtr ccx = context.getUserData<HwClosureContext>(HW::USER_DATA_CLOSURE_CONTEXT);

        if (ccx)
        {
            // Emit function name.
            shadergen.emitString(_functionName + ccx->getSuffix() + "(", stage);

            // Emit extra argument.
            for (const HwClosureContext::Argument& arg : ccx->getArguments())
            {
                shadergen.emitString(delim + arg.second, stage);
                delim = ", ";
            }
        }
        else
        {
            // Emit function name.
            shadergen.emitString(_functionName + "(", stage);
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
    END_SHADER_STAGE(stage, HW::PIXEL_STAGE)
}

} // namespace MaterialX
