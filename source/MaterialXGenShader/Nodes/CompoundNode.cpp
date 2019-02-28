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

void CompoundNode::initialize(ElementPtr implementation, GenContext& context)
{
    ShaderNodeImpl::initialize(implementation, context);

    NodeGraphPtr graph = implementation->asA<NodeGraph>();
    if (!graph)
    {
        throw ExceptionShaderGenError("Element '" + implementation->getName() + "' is not a node graph implementation");
    }

    _functionName = graph->getName();
    context.getShaderGenerator().getSyntax().makeValidName(_functionName);

    // For compounds we do not want to publish all internal inputs
    // so always use the reduced interface for this graph.
    const int shaderInterfaceType = context.getOptions().shaderInterfaceType;
    context.getOptions().shaderInterfaceType = SHADER_INTERFACE_REDUCED;
    _rootGraph = ShaderGraph::create(nullptr, graph, context);
    context.getOptions().shaderInterfaceType = shaderInterfaceType;
}

void CompoundNode::createVariables(const ShaderNode&, GenContext& context, Shader& shader) const
{
    // Gather shader inputs from all child nodes
    for (const ShaderNode* childNode : _rootGraph->getNodes())
    {
        childNode->getImplementation().createVariables(*childNode, context, shader);
    }
}

void CompoundNode::emitFunctionDefinition(const ShaderNode&, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, MAIN_STAGE)
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        const Syntax& syntax = shadergen.getSyntax();

        // Emit functions for all child nodes
        shadergen.emitFunctionDefinitions(*_rootGraph, context, stage);

        // Begin function signature.
        shadergen.emitLineBegin(stage);
        shadergen.emitString("void " + _functionName + + "(", stage);

        string delim = "";

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

        // End function signature.
        shadergen.emitString(")", stage);
        shadergen.emitLineEnd(stage, false);

        // Begin function body.
        shadergen.emitScopeBegin(stage);
        shadergen.emitFunctionCalls(*_rootGraph, context, stage);

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

        // End function body.
        shadergen.emitScopeEnd(stage);
        shadergen.emitLineBreak(stage);
    END_SHADER_STAGE(stage, MAIN_STAGE)
}

void CompoundNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, MAIN_STAGE)
        const ShaderGenerator& shadergen = context.getShaderGenerator();

        // Declare the output variables
        for (size_t i = 0; i < node.numOutputs(); ++i)
        {
            shadergen.emitLineBegin(stage);
            shadergen.emitOutput(node.getOutput(i), true, true, context, stage);
            shadergen.emitLineEnd(stage);
        }

        // Begin function call.
        shadergen.emitLineBegin(stage);
        shadergen.emitString(_functionName + "(", stage);

        string delim = "";

        // Emit inputs.
        for (ShaderInput* input : node.getInputs())
        {
            shadergen.emitString(delim, stage);
            shadergen.emitInput(input, context, stage);
            delim = ", ";
        }

        // Emit outputs.
        for (size_t i = 0; i < node.numOutputs(); ++i)
        {
            shadergen.emitString(delim, stage);
            shadergen.emitOutput(node.getOutput(i), false, false, context, stage);
            delim = ", ";
        }

        // End function call
        shadergen.emitString(")", stage);
        shadergen.emitLineEnd(stage);
    END_SHADER_STAGE(stage, MAIN_STAGE)
}

} // namespace MaterialX
