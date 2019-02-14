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

void CompoundNode::initialize(ElementPtr implementation, ShaderGenerator& shadergen, const GenOptions& options)
{
    ShaderNodeImpl::initialize(implementation, shadergen, options);

    NodeGraphPtr graph = implementation->asA<NodeGraph>();
    if (!graph)
    {
        throw ExceptionShaderGenError("Element '" + implementation->getName() + "' is not a node graph implementation");
    }

    // For compounds we do not want to publish all internal inputs
    // so always use the reduced interface for this graph.
    GenOptions compoundOptions(options);
    compoundOptions.shaderInterfaceType = SHADER_INTERFACE_REDUCED;

    _rootGraph = ShaderGraph::create(graph, shadergen, compoundOptions);
    _functionName = graph->getName();
}

void CompoundNode::createVariables(ShaderStage& stage, const ShaderNode&)
{
    // Gather shader inputs from all child nodes
    for (ShaderNode* childNode : _rootGraph->getNodes())
    {
        ShaderNodeImpl* impl = childNode->getImplementation();
        impl->createVariables(stage, *childNode);
    }
}

void CompoundNode::emitFunctionDefinition(ShaderStage& stage, const ShaderNode& node, ShaderGenerator& shadergen)
{
BEGIN_SHADER_STAGE(stage, MAIN_STAGE)

    // Make the compound root graph the active graph
    shadergen.pushActiveGraph(stage, _rootGraph.get());

    const Syntax* syntax = shadergen.getSyntax();

    // Emit functions for all child nodes
    for (ShaderNode* childNode : _rootGraph->getNodes())
    {
        shadergen.emitFunctionDefinition(stage, childNode);
    }

    // Emit function definitions for each context used by this compound node
    for (int id : node.getContextIDs())
    {
        const GenContext* context = shadergen.getContext(id);
        if (!context)
        {
            throw ExceptionShaderGenError("Node '" + node.getName() + "' has a context id that is undefined for shader generator '" + 
                shadergen.getLanguage() + "/" +shadergen.getTarget() + "'");
        }

        // Emit function name
        shadergen.emitLineBegin(stage);
        shadergen.emitString(stage, "void " + _functionName + context->getFunctionSuffix() + "(");

        // Emit function inputs
        string delim = "";

        // Add any extra argument inputs first
        for (const Argument& arg : context->getArguments())
        {
            shadergen.emitString(stage, delim + arg.first + " " + arg.second);
            delim = ", ";
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

        // End function call
        shadergen.emitString(stage, ")");
        shadergen.emitLineEnd(stage, false);

        shadergen.emitScopeBegin(stage);

        // Add function body, with all child node function calls
        for (ShaderNode* childNode : stage.getGraph()->getNodes())
        {
            shadergen.emitFunctionCall(stage, childNode, *context);
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

        shadergen.emitScopeEnd(stage);
        shadergen.emitLineBreak(stage);
    }

    // Restore active graph
    shadergen.popActiveGraph(stage);

END_SHADER_STAGE(stage, MAIN_STAGE)
}

void CompoundNode::emitFunctionCall(ShaderStage& stage, const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen)
{
BEGIN_SHADER_STAGE(stage, HW::VERTEX_STAGE)

    // Emit function calls for all child nodes to the vertex shader stage
    for (ShaderNode* childNode : _rootGraph->getNodes())
    {
        shadergen.emitFunctionCall(stage, childNode, context);
    }

END_SHADER_STAGE(stage, HW::VERTEX_STAGE)

BEGIN_SHADER_STAGE(stage, MAIN_STAGE)

    // Declare the output variables
    for (size_t i = 0; i < node.numOutputs(); ++i)
    {
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(stage, context, node.getOutput(i), true, true);
        shadergen.emitLineEnd(stage);
    }

    shadergen.emitLineBegin(stage);

    // Emit function name
    shadergen.emitString(stage, _functionName + context.getFunctionSuffix() + "(");

    // Emit function inputs
    string delim = "";

    // Add any extra argument inputs first...
    for (const Argument& arg : context.getArguments())
    {
        shadergen.emitString(stage, delim + arg.second);
        delim = ", ";
    }

    // ...and then all inputs on the node
    for (ShaderInput* input : node.getInputs())
    {
        shadergen.emitString(stage, delim);
        shadergen.emitInput(stage, context, input);
        delim = ", ";
    }

    // Emit function outputs
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
