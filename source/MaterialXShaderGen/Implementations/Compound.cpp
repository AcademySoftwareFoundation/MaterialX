#include <MaterialXCore/Library.h>
#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Document.h>

#include <MaterialXShaderGen/Implementations/Compound.h>
#include <MaterialXShaderGen/Shader.h>
#include <MaterialXShaderGen/ShaderGenerator.h>
#include <MaterialXShaderGen/Util.h>

namespace MaterialX
{

SgImplementationPtr Compound::creator()
{
    return std::make_shared<Compound>();
}

void Compound::initialize(ElementPtr implementation, ShaderGenerator& shadergen)
{
    SgImplementation::initialize(implementation, shadergen);

    NodeGraphPtr graph = implementation->asA<NodeGraph>();
    if (!graph)
    {
        throw ExceptionShaderGenError("Element '" + implementation->getName() + "' is not a node graph implementation");
    }

    _rootGraph = SgNodeGraph::creator(graph, shadergen);
    _functionName = graph->getName();
}

void Compound::emitFunctionDefinition(const SgNode& node, ShaderGenerator& shadergen, Shader& shader)
{
    // Make the compound root graph the active graph
    shader.pushActiveGraph(_rootGraph.get());

    const Syntax* syntax = shadergen.getSyntax().get();

    // Emit functions for all child nodes
    for (SgNode* childNode : _rootGraph->getNodes())
    {
        shader.addFunctionDefinition(childNode, shadergen);
    }

    // Emit function name
    shader.beginLine();
    shader.addStr("void " + _functionName + "(");

    // Emit function inputs
    string delim = "";

    // Add any extra argument inputs first
    const Arguments* args = shadergen.getExtraArguments(node);
    if (args)
    {
        for (size_t i = 0; i < args->size(); ++i)
        {
            const Argument& arg = (*args)[i];
            shader.addStr(delim + arg.first + " " + arg.second);
            delim = ", ";
        }
    }

    // Add all inputs
    for (SgInputSocket* inputSocket : _rootGraph->getInputSockets())
    {
        shader.addStr(delim + syntax->getTypeName(inputSocket->type) + " " + syntax->getVariableName(inputSocket));
        delim = ", ";
    }

    // Add all outputs
    for (SgOutputSocket* outputSocket : _rootGraph->getOutputSockets())
    {
        shader.addStr(delim + syntax->getOutputTypeName(outputSocket->type) + " " + syntax->getVariableName(outputSocket));
        delim = ", ";
    }

    // End function call
    shader.addStr(")");
    shader.endLine(false);

    shader.beginScope();

    // Add function body, with all child node function calls
    shadergen.emitShaderBody(shader);

    // Emit final results
    for (SgOutputSocket* outputSocket : _rootGraph->getOutputSockets())
    {
        const string outputVariable = syntax->getVariableName(outputSocket);
        string finalResult = syntax->getVariableName(outputSocket->connection);

        if (outputSocket->channels != EMPTY_STRING)
        {
            finalResult = syntax->getSwizzledVariable(finalResult, outputSocket->type, outputSocket->connection->type, outputSocket->channels);
        }

        shader.addLine(outputVariable + " = " + finalResult);
    }

    shader.endScope();
    shader.newLine();

    // Restore active graph
    shader.popActiveGraph();
}

void Compound::emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader)
{
    // An ordinary source code function call
    // TODO: Support multiple outputs

    // Declare the output variable
    shader.beginLine();
    shadergen.emitOutput(node.getOutput(), true, shader);
    shader.endLine();

    shader.beginLine();

    // Emit function name
    shader.addStr(_functionName + "(");

    // Emit function inputs
    string delim = "";

    // Add any extra argument inputs first...
    const Arguments* args = shadergen.getExtraArguments(node);
    if (args)
    {
        for (size_t i = 0; i < args->size(); ++i)
        {
            const Argument& arg = (*args)[i];
            shader.addStr(delim + arg.second);
            delim = ", ";
        }
    }

    // ...and then all inputs on the node
    for (SgInput* input : node.getInputs())
    {
        shader.addStr(delim);
        shadergen.emitInput(input, shader);
        delim = ", ";
    }

    // Emit function output
    shader.addStr(delim);
    shadergen.emitOutput(node.getOutput(), false, shader);

    // End function call
    shader.addStr(")");
    shader.endLine();
}

} // namespace MaterialX
