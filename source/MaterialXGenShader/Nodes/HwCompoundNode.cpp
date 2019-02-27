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

void HwCompoundNode::emitFunctionDefinition(ShaderStage& stage, const ShaderNode& node, const ShaderGenerator& shadergen_, GenContext& context) const
{
BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)

    const HwShaderGenerator& shadergen = static_cast<const HwShaderGenerator&>(shadergen_);

    // Emit functions for all child nodes
    shadergen.emitFunctionDefinitions(stage, context, *_rootGraph);

    // Find any closure contexts used by this node
    // and emit the function for each context.
    vector<HwClosureContextPtr> ccxs;
    shadergen.getNodeClosureContexts(node, ccxs);
    if (ccxs.empty())
    {
        emitFunctionDefinition(stage, shadergen, context, nullptr);
    }
    else
    {
        for (HwClosureContextPtr ccx : ccxs)
        {
            emitFunctionDefinition(stage, shadergen, context, ccx);
        }
    }

END_SHADER_STAGE(stage, HW::PIXEL_STAGE)
}

void HwCompoundNode::emitFunctionDefinition(ShaderStage& stage, const HwShaderGenerator& shadergen, 
    GenContext& context, HwClosureContextPtr ccx) const
{
    const Syntax* syntax = shadergen.getSyntax();

    string delim = "";

    // Begin function signature
    shadergen.emitLineBegin(stage);
    if (ccx)
    {
        shadergen.emitString(stage, "void " + _functionName + ccx->getSuffix() + "(");

        // Add any extra argument inputs first
        for (const HwClosureContext::Argument& arg : ccx->getArguments())
        {
            const string& type = shadergen.getSyntax()->getTypeName(arg.first);
            shadergen.emitString(stage, delim + type + " " + arg.second);
            delim = ", ";
        }
    }
    else
    {
        shadergen.emitString(stage, "void " + _functionName + "(");
    }

    // Add all inputs
    for (ShaderGraphInputSocket* inputSocket : _rootGraph->getInputSockets())
    {
        shadergen.emitString(stage, delim + syntax->getTypeName(inputSocket->getType()) + " " + inputSocket->getVariable());
        delim = ", ";
    }

    // Add all outputs
    for (ShaderGraphOutputSocket* outputSocket : _rootGraph->getOutputSockets())
    {
        shadergen.emitString(stage, delim + syntax->getOutputTypeName(outputSocket->getType()) + " " + outputSocket->getVariable());
        delim = ", ";
    }

    // End function sginature
    shadergen.emitString(stage, ")");
    shadergen.emitLineEnd(stage, false);

    // Begin function body
    shadergen.emitScopeBegin(stage);

    if (ccx)
    {
        context.pushUserData(HW::USER_DATA_CLOSURE_CONTEXT, ccx);
        shadergen.emitFunctionCalls(stage, context, *_rootGraph);
        context.popUserData(HW::USER_DATA_CLOSURE_CONTEXT);
    }
    else
    {
        shadergen.emitFunctionCalls(stage, context, *_rootGraph);
    }

    // Emit final results
    for (ShaderGraphOutputSocket* outputSocket : _rootGraph->getOutputSockets())
    {
        // Check for the rare case where the output is not internally connected
        if (!outputSocket->getConnection())
        {
            shadergen.emitLine(stage, outputSocket->getVariable() + " = " + (outputSocket->getValue() ?
                syntax->getValue(outputSocket->getType(), *outputSocket->getValue()) :
                syntax->getDefaultValue(outputSocket->getType())));
        }
        else
        {
            shadergen.emitLine(stage, outputSocket->getVariable() + " = " + outputSocket->getConnection()->getVariable());
        }
    }

    // End function body
    shadergen.emitScopeEnd(stage);
    shadergen.emitLineBreak(stage);
}

void HwCompoundNode::emitFunctionCall(ShaderStage& stage, const ShaderNode& node, const ShaderGenerator& shadergen, GenContext& context) const
{
BEGIN_SHADER_STAGE(stage, HW::VERTEX_STAGE)

    // Emit function calls for all child nodes to the vertex shader stage
    // TODO: Is this ever usefull?
    shadergen.emitFunctionCalls(stage, context, *_rootGraph);

END_SHADER_STAGE(stage, HW::VERTEX_STAGE)

BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)

    // Declare the output variables
    for (size_t i = 0; i < node.numOutputs(); ++i)
    {
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(stage, context, node.getOutput(i), true, true);
        shadergen.emitLineEnd(stage);
    }

    shadergen.emitLineBegin(stage);
    string delim = "";

    // Check if we have a closure context to modify the function call.
    HwClosureContextPtr ccx = context.getUserData<HwClosureContext>(HW::USER_DATA_CLOSURE_CONTEXT);

    if (ccx)
    {
        // Emit function name.
        shadergen.emitString(stage, _functionName + ccx->getSuffix() + "(");

        // Emit extra argument.
        for (const HwClosureContext::Argument& arg : ccx->getArguments())
        {
            shadergen.emitString(stage, delim + arg.second);
            delim = ", ";
        }
    }
    else
    {
        // Emit function name.
        shadergen.emitString(stage, _functionName + "(");
    }

    // Emit all inputs.
    for (ShaderInput* input : node.getInputs())
    {
        shadergen.emitString(stage, delim);
        shadergen.emitInput(stage, context, input);
        delim = ", ";
    }

    // Emit all outputs.
    for (size_t i = 0; i < node.numOutputs(); ++i)
    {
        shadergen.emitString(stage, delim);
        shadergen.emitOutput(stage, context, node.getOutput(i), false, false);
        delim = ", ";
    }

    // End function call
    shadergen.emitString(stage, ")");
    shadergen.emitLineEnd(stage);

END_SHADER_STAGE(stage, HW::PIXEL_STAGE)
}

} // namespace MaterialX
