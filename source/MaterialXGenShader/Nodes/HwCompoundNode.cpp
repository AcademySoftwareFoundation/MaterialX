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
    shadergen.emitFunctionDefinitions(stage, *_rootGraph, context);

    // Find any closure contexts used by this node
    // and emit the function for each context.
    vector<const HwClosureContext*> ccxs;
    shadergen.getClosureContexts(node, ccxs);
    if (ccxs.empty())
    {
        emitFunctionDefinition(stage, shadergen, context, nullptr);
    }
    else
    {
        for (const HwClosureContext* ccx : ccxs)
        {
            emitFunctionDefinition(stage, shadergen, context, ccx);
        }
    }

END_SHADER_STAGE(stage, HW::PIXEL_STAGE)
}

void HwCompoundNode::emitFunctionDefinition(ShaderStage& stage, const HwShaderGenerator& shadergen, 
    GenContext& context, const HwClosureContext* ccx) const
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
            shadergen.emitString(stage, delim + arg.first + " " + arg.second);
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
        shadergen.emitString(stage, delim + syntax->getTypeName(inputSocket->type) + " " + inputSocket->variable);
        delim = ", ";
    }

    // Add all outputs
    for (ShaderGraphOutputSocket* outputSocket : _rootGraph->getOutputSockets())
    {
        shadergen.emitString(stage, delim + syntax->getOutputTypeName(outputSocket->type) + " " + outputSocket->variable);
        delim = ", ";
    }

    // End function sginature
    shadergen.emitString(stage, ")");
    shadergen.emitLineEnd(stage, false);

    // Begin function body
    shadergen.emitScopeBegin(stage);

    if (ccx)
    {
        context.pushUserData(HW::CLOSURE_CONTEXT, ccx);
        shadergen.emitFunctionCalls(stage, *_rootGraph, context);
        context.popUserData(HW::CLOSURE_CONTEXT);
    }
    else
    {
        shadergen.emitFunctionCalls(stage, *_rootGraph, context);
    }

    // Emit final results
    for (ShaderGraphOutputSocket* outputSocket : _rootGraph->getOutputSockets())
    {
        // Check for the rare case where the output is not internally connected
        if (!outputSocket->connection)
        {
            shadergen.emitLine(stage, outputSocket->variable + " = " + (outputSocket->value ?
                syntax->getValue(outputSocket->type, *outputSocket->value) :
                syntax->getDefaultValue(outputSocket->type)));
        }
        else
        {
            shadergen.emitLine(stage, outputSocket->variable + " = " + outputSocket->connection->variable);
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
    shadergen.emitFunctionCalls(stage, *_rootGraph, context);

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
    const HwClosureContext* ccx = context.getUserData<HwClosureContext>(HW::CLOSURE_CONTEXT);

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
