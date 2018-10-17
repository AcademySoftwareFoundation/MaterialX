#include <MaterialXGenShader/Nodes/Compound.h>
#include <MaterialXGenShader/HwShader.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <MaterialXCore/Library.h>
#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Document.h>

namespace MaterialX
{

GenImplementationPtr Compound::create()
{
    return std::make_shared<Compound>();
}

void Compound::initialize(ElementPtr implementation, ShaderGenerator& shadergen)
{
    GenImplementation::initialize(implementation, shadergen);

    NodeGraphPtr graph = implementation->asA<NodeGraph>();
    if (!graph)
    {
        throw ExceptionShaderGenError("Element '" + implementation->getName() + "' is not a node graph implementation");
    }

    _dag = Dag::create(graph, shadergen);
    _functionName = graph->getName();
}

void Compound::createVariables(const DagNode& /*node*/, ShaderGenerator& shadergen, Shader& shader)
{
    // Gather shader inputs from all child nodes
    for (DagNode* childNode : _dag->getNodes())
    {
        GenImplementation* impl = childNode->getImplementation();
        impl->createVariables(*childNode, shadergen, shader);
    }
}

void Compound::emitFunctionDefinition(const DagNode& node, ShaderGenerator& shadergen, Shader& shader)
{
    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

    // Make the compound root graph the active graph
    shader.pushActiveDag(_dag.get());

    const Syntax* syntax = shadergen.getSyntax();

    // Emit functions for all child nodes
    for (DagNode* childNode : _dag->getNodes())
    {
        shader.addFunctionDefinition(childNode, shadergen);
    }

    // Emit function definitions for each context used by this compound node
    for (int id : node.getContextIDs())
    {
        const GenContext* context = shadergen.getNodeContext(id);
        if (!context)
        {
            throw ExceptionShaderGenError("Node '" + node.getName() + "' has an implementation context that is undefined for shader generator '" + 
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
        for (DagInputSocket* inputSocket : _dag->getInputSockets())
        {
            shader.addStr(delim + syntax->getTypeName(inputSocket->type) + " " + inputSocket->name);
            delim = ", ";
        }

        // Add all outputs
        for (DagOutputSocket* outputSocket : _dag->getOutputSockets())
        {
            shader.addStr(delim + syntax->getOutputTypeName(outputSocket->type) + " " + outputSocket->name);
            delim = ", ";
        }

        // End function call
        shader.addStr(")");
        shader.endLine(false);

        shader.beginScope();

        // Add function body, with all child node function calls
        shadergen.emitFunctionCalls(*context, shader);

        // Emit final results
        for (DagOutputSocket* outputSocket : _dag->getOutputSockets())
        {
            // Check for the rare case where the output is not internally connected
            if (!outputSocket->connection)
            {
                shader.addLine(outputSocket->name + " = " + (outputSocket->value ?
                    syntax->getValue(outputSocket->type, *outputSocket->value) :
                    syntax->getDefaultValue(outputSocket->type)));
            }
            else
            {
                shader.addLine(outputSocket->name + " = " + outputSocket->connection->name);
            }
        }

        shader.endScope();
        shader.newLine();
    }

    // Restore active graph
    shader.popActiveDag();

    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

void Compound::emitFunctionCall(const DagNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader)
{
    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

    // Emit function calls for all child nodes to the vertex shader stage
    for (DagNode* childNode : _dag->getNodes())
    {
        shader.addFunctionCall(childNode, context, shadergen);
    }

    END_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

    // Declare the output variable
    shader.beginLine();
    shadergen.emitOutput(context, node.getOutput(), true, true, shader);
    shader.endLine();

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
    for (DagInput* input : node.getInputs())
    {
        shader.addStr(delim);
        shadergen.emitInput(context, input, shader);
        delim = ", ";
    }

    // Emit function output
    shader.addStr(delim);
    shadergen.emitOutput(context, node.getOutput(), false, false, shader);

    // End function call
    shader.addStr(")");
    shader.endLine();

    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
