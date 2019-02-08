#include <MaterialXGenShader/Nodes/CompoundNode.h>
#include <MaterialXGenShader/HwShader.h>
#include <MaterialXGenShader/ShaderGenerator.h>
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

void CompoundNode::createVariables(const ShaderNode& /*node*/, ShaderGenerator& shadergen, Shader& shader)
{
    // Gather shader inputs from all child nodes
    for (ShaderNode* childNode : _rootGraph->getNodes())
    {
        ShaderNodeImpl* impl = childNode->getImplementation();
        impl->createVariables(*childNode, shadergen, shader);
    }
}

void CompoundNode::emitFunctionDefinition(const ShaderNode& node, ShaderGenerator& shadergen, Shader& shader)
{
    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

    // Make the compound root graph the active graph
    shader.pushActiveGraph(_rootGraph.get());

    const Syntax* syntax = shadergen.getSyntax();

    // Emit functions for all child nodes
    for (ShaderNode* childNode : _rootGraph->getNodes())
    {
        shader.addFunctionDefinition(childNode, shadergen);
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
        shader.beginLine();
        shader.addStr("void " + _functionName + context->getFunctionSuffix() + "(");

        // Emit function inputs
        string delim = "";

        // Add any extra argument inputs first
        for (const Argument& arg : context->getArguments())
        {
            shader.addStr(delim + arg.first + " " + arg.second);
            delim = ", ";
        }

        // Add all inputs
        for (ShaderGraphInputSocket* inputSocket : _rootGraph->getInputSockets())
        {
            shader.addStr(delim + syntax->getTypeName(inputSocket->type) + " " + inputSocket->variable);
            delim = ", ";
        }

        // Add all outputs
        for (ShaderGraphOutputSocket* outputSocket : _rootGraph->getOutputSockets())
        {
            shader.addStr(delim + syntax->getOutputTypeName(outputSocket->type) + " " + outputSocket->variable);
            delim = ", ";
        }

        // End function call
        shader.addStr(")");
        shader.endLine(false);

        shader.beginScope();

        // Add function body, with all child node function calls
        shadergen.emitFunctionCalls(*context, shader);

        // Emit final results
        for (ShaderGraphOutputSocket* outputSocket : _rootGraph->getOutputSockets())
        {
            // Check for the rare case where the output is not internally connected
            if (!outputSocket->connection)
            {
                shader.addLine(outputSocket->variable + " = " + (outputSocket->value ?
                    syntax->getValue(outputSocket->type, *outputSocket->value) :
                    syntax->getDefaultValue(outputSocket->type)));
            }
            else
            {
                shader.addLine(outputSocket->variable + " = " + outputSocket->connection->variable);
            }
        }

        shader.endScope();
        shader.newLine();
    }

    // Restore active graph
    shader.popActiveGraph();

    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

void CompoundNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader)
{
    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

    // Emit function calls for all child nodes to the vertex shader stage
    for (ShaderNode* childNode : _rootGraph->getNodes())
    {
        shader.addFunctionCall(childNode, context, shadergen);
    }

    END_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

    // Declare the output variables
    for (size_t i = 0; i < node.numOutputs(); ++i)
    {
        shader.beginLine();
        shadergen.emitOutput(context, node.getOutput(i), true, true, shader);
        shader.endLine();
    }

    shader.beginLine();

    // Emit function name
    shader.addStr(_functionName + context.getFunctionSuffix() + "(");

    // Emit function inputs
    string delim = "";

    // Add any extra argument inputs first...
    for (const Argument& arg : context.getArguments())
    {
        shader.addStr(delim + arg.second);
        delim = ", ";
    }

    // ...and then all inputs on the node
    for (ShaderInput* input : node.getInputs())
    {
        shader.addStr(delim);
        shadergen.emitInput(context, input, shader);
        delim = ", ";
    }

    // Emit function outputs
    for (size_t i = 0; i < node.numOutputs(); ++i)
    {
        shader.addStr(delim);
        shadergen.emitOutput(context, node.getOutput(i), false, false, shader);
        delim = ", ";
    }

    // End function call
    shader.addStr(")");
    shader.endLine();

    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
