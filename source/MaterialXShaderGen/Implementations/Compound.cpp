#include <MaterialXCore/Library.h>
#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Document.h>

#include <MaterialXShaderGen/Implementations/Compound.h>
#include <MaterialXShaderGen/Shader.h>
#include <MaterialXShaderGen/ShaderGenerator.h>
#include <MaterialXShaderGen/Util.h>

#include <cstdarg>

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

    _sgNodeGraph = SgNodeGraph::creator(graph, shadergen);
    _functionName = graph->getName();
}

void Compound::emitFunction(const SgNode&, ShaderGenerator& shadergen, Shader& shader, int numArgs, ...)
{
    const Syntax* syntax = shadergen.getSyntax().get();

    // Emit functions for all child nodes
    for (SgNode* childNode : _sgNodeGraph->getNodes())
    {
        childNode->getImplementation()->emitFunction(*childNode, shadergen, shader);
    }

    // Emit function name
    shader.beginLine();
    shader.addStr("void " + _functionName + "(");

    // Emit function inputs
    string delim = "";

    // Add any extra argument inputs first
    va_list argsList;
    va_start(argsList, numArgs);
    for (int i = 0; i < numArgs; i++)
    {
        const char* arg = va_arg(argsList, const char*);
        shader.addStr(delim + arg);
        delim = ", ";
    }
    va_end(argsList);

    // Add all inputs
    for (SgInput* input : _sgNodeGraph->getInputs())
    {
        shader.addStr(delim + syntax->getTypeName(input->type) + " " + syntax->getVariableName(input));
        delim = ", ";
    }

    // Add all outputs
    for (SgOutput* output : _sgNodeGraph->getOutputs())
    {
        shader.addStr(delim + syntax->getOutputTypeName(output->type) + " " + syntax->getVariableName(output));
        delim = ", ";
    }

    // End function call
    shader.addStr(")");
    shader.endLine(false);

    shader.beginScope();

    const bool debugOutput = true;

    // Emit function calls for all child nodes
    for (SgNode* childNode : _sgNodeGraph->getNodes())
    {
        // Omit node if it's only used inside a conditional branch
        if (childNode->referencedConditionally())
        {
            if (debugOutput)
            {
                std::stringstream str;
                str << "// Omitted node '" << childNode->getName() << "'. Only used in conditional node '" << childNode->getScopeInfo().conditionalNode->getName() << "'";
                shader.addLine(str.str(), false);
            }
            // Omit this node
            continue;
        }

        childNode->getImplementation()->emitFunctionCall(*childNode, shadergen, shader);
    }

    for (SgOutput* output : _sgNodeGraph->getOutputs())
    {
        const string outputVariable = syntax->getVariableName(output);
        SgInput* outputSocket = _sgNodeGraph->getOutputSocket(output->name);
        string finalResult = syntax->getVariableName(outputSocket->connection);

        if (outputSocket->channels != EMPTY_STRING)
        {
            finalResult = syntax->getSwizzledVariable(finalResult, output->type, outputSocket->connection->type, outputSocket->channels);
        }

        shader.addLine(outputVariable + " = " + finalResult);
    }

    shader.endScope();
    shader.newLine();
}

void Compound::emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader, int numArgs, ...)
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
    va_list argsList;
    va_start(argsList, numArgs);
    for (int i = 0; i < numArgs; i++)
    {
        const char* arg = va_arg(argsList, const char*);
        shader.addStr(delim + arg);
        delim = ", ";
    }
    va_end(argsList);

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
