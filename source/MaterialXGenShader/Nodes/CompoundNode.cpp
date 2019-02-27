#include <MaterialXGenShader/Nodes/CompoundNode.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <MaterialXCore/Library.h>
#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Document.h>

namespace MaterialX
{

ShaderNodeImplPtr CompoundNode::create()
{
    return std::make_shared<CompoundNode>();
}

void CompoundNode::initialize(GenContext& context, const ShaderGenerator& shadergen, ElementPtr implementation)
{
    ShaderNodeImpl::initialize(context, shadergen, implementation);

    NodeGraphPtr graph = implementation->asA<NodeGraph>();
    if (!graph)
    {
        throw ExceptionShaderGenError("Element '" + implementation->getName() + "' is not a node graph implementation");
    }

    _functionName = graph->getName();

    // For compounds we do not want to publish all internal inputs
    // so always use the reduced interface for this graph.
    const int shaderInterfaceType = context.getOptions().shaderInterfaceType;
    context.getOptions().shaderInterfaceType = SHADER_INTERFACE_REDUCED;
    _rootGraph = ShaderGraph::create(nullptr, graph, shadergen, context);
    context.getOptions().shaderInterfaceType = shaderInterfaceType;
}

void CompoundNode::createVariables(Shader& shader, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode&) const
{
    // Gather shader inputs from all child nodes
    for (const ShaderNode* childNode : _rootGraph->getNodes())
    {
        childNode->getImplementation().createVariables(shader, context, shadergen, *childNode);
    }
}

void CompoundNode::emitFunctionDefinition(ShaderStage& stage, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode&) const
{
BEGIN_SHADER_STAGE(stage, MAIN_STAGE)

    const Syntax* syntax = shadergen.getSyntax();

    // Emit functions for all child nodes
    shadergen.emitFunctionDefinitions(stage, context, *_rootGraph);

    // Begin function signature.
    shadergen.emitLineBegin(stage);
    shadergen.emitString(stage, "void " + _functionName + + "(");

    string delim = "";

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

    // End function signature.
    shadergen.emitString(stage, ")");
    shadergen.emitLineEnd(stage, false);

    // Begin function body.
    shadergen.emitScopeBegin(stage);
    shadergen.emitFunctionCalls(stage, context, *_rootGraph);

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

    // End function body.
    shadergen.emitScopeEnd(stage);
    shadergen.emitLineBreak(stage);

END_SHADER_STAGE(stage, MAIN_STAGE)
}

void CompoundNode::emitFunctionCall(ShaderStage& stage, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const
{
BEGIN_SHADER_STAGE(stage, MAIN_STAGE)

    // Declare the output variables
    for (size_t i = 0; i < node.numOutputs(); ++i)
    {
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(stage, context, node.getOutput(i), true, true);
        shadergen.emitLineEnd(stage);
    }

    // Begin function call.
    shadergen.emitLineBegin(stage);
    shadergen.emitString(stage, _functionName + "(");

    string delim = "";

    // Emit inputs.
    for (ShaderInput* input : node.getInputs())
    {
        shadergen.emitString(stage, delim);
        shadergen.emitInput(stage, context, input);
        delim = ", ";
    }

    // Emit outputs.
    for (size_t i = 0; i < node.numOutputs(); ++i)
    {
        shadergen.emitString(stage, delim);
        shadergen.emitOutput(stage, context, node.getOutput(i), false, false);
        delim = ", ";
    }

    // End function call
    shadergen.emitString(stage, ")");
    shadergen.emitLineEnd(stage);

END_SHADER_STAGE(stage, MAIN_STAGE)
}

} // namespace MaterialX
