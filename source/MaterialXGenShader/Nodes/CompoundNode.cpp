//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/Nodes/CompoundNode.h>
#include <MaterialXGenShader/Exception.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Document.h>
#include <MaterialXCore/Library.h>

#include <MaterialXTrace/Tracing.h>

#include <functional>
#include <set>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr CompoundNode::create()
{
    return std::make_shared<CompoundNode>();
}

void CompoundNode::addClassification(ShaderNode& node) const
{
    // Add classification from the graph implementation.
    node.addClassification(_rootGraph->getClassification());
}

void CompoundNode::initialize(const InterfaceElement& element, GenContext& context)
{
    MX_TRACE_FUNCTION(Tracing::Category::ShaderGen);
    MX_TRACE_SCOPE(Tracing::Category::ShaderGen, element.getName().c_str());

    ShaderNodeImpl::initialize(element, context);

    if (!element.isA<NodeGraph>())
    {
        throw ExceptionShaderGenError("Element '" + element.getName() + "' is not a node graph implementation");
    }

    const NodeGraph& graph = static_cast<const NodeGraph&>(element);

    _functionName = graph.getName();
    context.getShaderGenerator().getSyntax().makeValidName(_functionName);

    // For compounds we do not want to publish all internal inputs
    // so always use the reduced interface for this graph.
    const ShaderInterfaceType oldShaderInterfaceType = context.getOptions().shaderInterfaceType;
    context.getOptions().shaderInterfaceType = SHADER_INTERFACE_REDUCED;
    _rootGraph = ShaderGraph::create(nullptr, graph, context);
    context.getOptions().shaderInterfaceType = oldShaderInterfaceType;

    // Set hash using the function name.
    // TODO: Could be improved to include the full function signature.
    _hash = std::hash<string>{}(_functionName);
}

void CompoundNode::createVariables(const ShaderNode&, GenContext& context, Shader& shader) const
{
    // Gather shader inputs from all child nodes
    for (const ShaderNode* childNode : _rootGraph->getNodes())
    {
        childNode->getImplementation().createVariables(*childNode, context, shader);
    }
}

void CompoundNode::emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    MX_TRACE_FUNCTION(Tracing::Category::ShaderGen);
    MX_TRACE_SCOPE(Tracing::Category::ShaderGen, _functionName.c_str());

    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        // When displacement is active, emit a vertex-stage displacement
        // function. This is the same pattern as the pixel stage: emit
        // function definitions for internal nodes, then emit the compound
        // function itself. Using a function call avoids all variable
        // collision and ordering issues that arise from inlining.
        if (context.getEmitVertexDisplacement())
        {
            const ShaderGenerator& shadergen = context.getShaderGenerator();

            // Collect displacement-relevant internal nodes
            std::set<const ShaderNode*> relevantNodes;
            std::function<void(const ShaderNode*)> collectUpstream = [&](const ShaderNode* n) {
                if (!n || relevantNodes.count(n)) return;
                relevantNodes.insert(n);
                for (ShaderInput* input : n->getInputs())
                {
                    const ShaderNode* upstream = input->getConnectedSibling();
                    if (upstream && upstream->getParent() == _rootGraph.get())
                        collectUpstream(upstream);
                }
            };
            for (ShaderGraphOutputSocket* outputSocket : _rootGraph->getOutputSockets())
            {
                const TypeDesc& outType = outputSocket->getType();
                if (outType == Type::DISPLACEMENTSHADER ||
                    (!outType.isClosure() && outType.getSemantic() != TypeDesc::SEMANTIC_SHADER))
                {
                    if (outputSocket->getConnection())
                        collectUpstream(outputSocket->getConnection()->getNode());
                }
            }

            // Emit function definitions for displacement-relevant nodes.
            // Use emitFunctionDefinition which handles library includes
            // (e.g. mx_math.glsl for mx_sin/mx_cos defines).
            for (const ShaderNode* internalNode : _rootGraph->getNodes())
            {
                if (relevantNodes.count(internalNode))
                {
                    shadergen.emitFunctionDefinition(*internalNode, context, stage);
                }
            }

            // Emit the compound displacement function definition.
            // This follows the same pattern as the pixel stage but only
            // includes displacement-relevant internal nodes.
            const string vertFuncName = _functionName + "_displacement";
            shadergen.emitLineBegin(stage);
            shadergen.emitString("void " + vertFuncName + "(", stage);

            string delim;
            // Add all inputs (interface parameters)
            for (ShaderGraphInputSocket* inputSocket : _rootGraph->getInputSockets())
            {
                shadergen.emitString(delim, stage);
                shadergen.emitFunctionDefinitionParameter(inputSocket, false, context, stage);
                delim = ", ";
            }
            // Add displacement-related outputs only
            for (ShaderGraphOutputSocket* outputSocket : _rootGraph->getOutputSockets())
            {
                const TypeDesc& outType = outputSocket->getType();
                if (outType == Type::DISPLACEMENTSHADER ||
                    (!outType.isClosure() && outType.getSemantic() != TypeDesc::SEMANTIC_SHADER))
                {
                    shadergen.emitString(delim, stage);
                    shadergen.emitFunctionDefinitionParameter(outputSocket, true, context, stage);
                    delim = ", ";
                }
            }
            shadergen.emitString(")", stage);
            shadergen.emitLineEnd(stage, false);

            // Function body
            shadergen.emitFunctionBodyBegin(*_rootGraph, context, stage);
            for (const ShaderNode* internalNode : _rootGraph->getNodes())
            {
                if (relevantNodes.count(internalNode))
                {
                    shadergen.emitFunctionCall(*internalNode, context, stage);
                }
            }
            // Assign outputs
            for (ShaderGraphOutputSocket* outputSocket : _rootGraph->getOutputSockets())
            {
                const TypeDesc& outType = outputSocket->getType();
                if (outType == Type::DISPLACEMENTSHADER ||
                    (!outType.isClosure() && outType.getSemantic() != TypeDesc::SEMANTIC_SHADER))
                {
                    const string result = shadergen.getUpstreamResult(outputSocket, context);
                    shadergen.emitLine(outputSocket->getVariable() + " = " + result, stage);
                }
            }
            shadergen.emitFunctionBodyEnd(*_rootGraph, context, stage);
        }
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const ShaderGenerator& shadergen = context.getShaderGenerator();

        // Emit functions for all child nodes
        shadergen.emitFunctionDefinitions(*_rootGraph, context, stage);

        // Begin function signature.
        shadergen.emitLineBegin(stage);
        shadergen.emitString("void " + _functionName + "(", stage);

        shadergen.emitClosureDataParameter(node, context, stage);
        // if (context.getShaderGenerator().nodeNeedsClosureData(node))
        // {
        //     shadergen.emitString(HW::CLOSURE_DATA_TYPE + " " + HW::CLOSURE_DATA_ARG + ", ", stage);
        // }

        string delim;

        // Add all inputs
        for (ShaderGraphInputSocket* inputSocket : _rootGraph->getInputSockets())
        {
            shadergen.emitString(delim, stage);
            shadergen.emitFunctionDefinitionParameter(inputSocket, false, context, stage);
            delim = ", ";
        }

        // Add all outputs
        for (ShaderGraphOutputSocket* outputSocket : _rootGraph->getOutputSockets())
        {
            shadergen.emitString(delim, stage);
            shadergen.emitFunctionDefinitionParameter(outputSocket, true, context, stage);
            delim = ", ";
        }

        // End function signature.
        shadergen.emitString(")", stage);
        shadergen.emitLineEnd(stage, false);

        // Begin function body.
        shadergen.emitFunctionBodyBegin(*_rootGraph, context, stage);

        if (nodeOutputIsClosure(node))
        {
            // Emit all texturing nodes. These are inputs to the
            // closure nodes and need to be emitted first.
            shadergen.emitFunctionCalls(*_rootGraph, context, stage, ShaderNode::Classification::TEXTURE);

            // Emit function calls for internal closures nodes connected to the graph sockets.
            // These will in turn emit function calls for any dependent closure nodes upstream.
            for (ShaderGraphOutputSocket* outputSocket : _rootGraph->getOutputSockets())
            {
                if (outputSocket->getConnection())
                {
                    const ShaderNode* upstream = outputSocket->getConnection()->getNode();
                    // Its important that the classification check here matches the logic inside
                    // nodeOutputIsClosure() used above.
                    if (upstream->getParent() == _rootGraph.get() &&
                        (upstream->hasClassification(ShaderNode::Classification::CLOSURE) ||
                            upstream->hasClassification(ShaderNode::Classification::SHADER) ||
                            upstream->hasClassification(ShaderNode::Classification::MATERIAL)))
                    {
                        shadergen.emitFunctionCall(*upstream, context, stage);
                    }
                }
            }
        }
        else
        {
            shadergen.emitFunctionCalls(*_rootGraph, context, stage);
        }

        // Emit final results
        for (ShaderGraphOutputSocket* outputSocket : _rootGraph->getOutputSockets())
        {
            const string result = shadergen.getUpstreamResult(outputSocket, context);
            shadergen.emitLine(outputSocket->getVariable() + " = " + result, stage);
        }

        // End function body.
        shadergen.emitFunctionBodyEnd(*_rootGraph, context, stage);
    }
}

void CompoundNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    MX_TRACE_FUNCTION(Tracing::Category::ShaderGen);
    MX_TRACE_SCOPE(Tracing::Category::ShaderGen, _functionName.c_str());

    const ShaderGenerator& shadergen = context.getShaderGenerator();

    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        if (context.getEmitVertexDisplacement())
        {
            // Call the vertex-stage displacement function (defined in
            // emitFunctionDefinition above). This avoids all variable
            // collision and ordering issues from inlining.
            const string vertFuncName = _functionName + "_displacement";

            // Declare output variables
            emitOutputVariables(node, context, stage);

            // Emit function call
            shadergen.emitLineBegin(stage);
            shadergen.emitString(vertFuncName + "(", stage);

            string delim;
            // Pass all inputs
            for (ShaderInput* input : node.getInputs())
            {
                shadergen.emitString(delim, stage);
                shadergen.emitInput(input, context, stage);
                delim = ", ";
            }
            // Pass displacement output variables
            for (size_t i = 0; i < node.numOutputs(); ++i)
            {
                const ShaderOutput* nodeOutput = node.getOutput(i);
                const TypeDesc& outType = nodeOutput->getType();
                if (outType == Type::DISPLACEMENTSHADER ||
                    (!outType.isClosure() && outType.getSemantic() != TypeDesc::SEMANTIC_SHADER))
                {
                    shadergen.emitString(delim, stage);
                    shadergen.emitOutput(nodeOutput, false, false, context, stage);
                    delim = ", ";
                }
            }

            shadergen.emitString(")", stage);
            shadergen.emitLineEnd(stage);
        }
        else
        {
            // Standard vertex stage: emit function calls for all child nodes
            shadergen.emitFunctionCalls(*_rootGraph, context, stage);
        }
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        if (nodeOutputIsClosure(node))
        {
            // Emit calls for any closure dependencies upstream from this nodedef
            shadergen.emitDependentFunctionCalls(node, context, stage, ShaderNode::Classification::CLOSURE);
        }

        // Declare the output variables.
        emitOutputVariables(node, context, stage);

        // Begin function call.
        shadergen.emitLineBegin(stage);
        shadergen.emitString(_functionName + "(", stage);

        // Add an argument for closure data if needed
        shadergen.emitClosureDataArg(node, context, stage);
        // if (context.getShaderGenerator().nodeNeedsClosureData(node))
        // {
        //     shadergen.emitString(HW::CLOSURE_DATA_ARG + ", ", stage);
        // }

        string delim;

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
    }
}

MATERIALX_NAMESPACE_END
