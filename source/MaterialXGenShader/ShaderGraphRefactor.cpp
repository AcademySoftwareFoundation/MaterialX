//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/ShaderGraphRefactor.h>

#include <MaterialXGenShader/Exception.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderGraph.h>
#include <MaterialXGenShader/ShaderNodeImpl.h>

#include <MaterialXCore/Document.h>

#include <string>

MATERIALX_NAMESPACE_BEGIN

namespace
{

bool isBsdfMixNode(ShaderNode* node)
{
    return node->hasClassification(ShaderNode::Classification::BSDF |
                                   ShaderNode::Classification::CLOSURE |
                                   ShaderNode::Classification::MIX);
}

bool isLayerWithMixTop(ShaderNode* node)
{
    if (!node->hasClassification(ShaderNode::Classification::LAYER))
    {
        return false;
    }
    ShaderInput* top = node->getInput("top");
    if (!top || !top->getConnection())
    {
        return false;
    }
    return isBsdfMixNode(top->getConnection()->getNode());
}

// Fold a mix weight into one side of a BSDF mix node, inserting a multiply
// node to combine the existing weight (connected or constant) with the mix
// weight.  Return the upstream output to connect to the replacement add node.
ShaderOutput* foldWeightIntoBsdf(ShaderGraph& graph, GenContext& context,
                                 ShaderInput* bsdfInput, ShaderOutput* weightSource,
                                 const string& namePrefix,
                                 NodeDefPtr mulFloatDef, NodeDefPtr mulBsdfDef)
{
    ShaderOutput* bsdfUpstream = bsdfInput->getConnection();
    if (!bsdfUpstream)
    {
        return nullptr;
    }

    ShaderNode* bsdfNode = bsdfUpstream->getNode();
    ShaderInput* weightInput = bsdfNode->getInput("weight");
    if (weightInput)
    {
        // Create a multiply node to combine existing weight with mix weight.
        string mulName = namePrefix + "_weight";
        ShaderNode* mulNode = graph.createNode(mulName, mulName, mulFloatDef, context);
        ShaderInput* mulIn1 = mulNode->getInput("in1");
        ShaderInput* mulIn2 = mulNode->getInput("in2");
        if (mulIn1 && mulIn2)
        {
            ShaderOutput* existingSource = weightInput->getConnection();
            if (existingSource)
            {
                weightInput->breakConnection();
                mulIn1->makeConnection(existingSource);
            }
            else if (weightInput->getValue())
            {
                mulIn1->setValue(weightInput->getValue());
            }
            else
            {
                mulIn1->setValue(Value::createValue<float>(1.0f));
            }
            mulIn2->makeConnection(weightSource);
            weightInput->makeConnection(mulNode->getOutput());
        }
        return bsdfUpstream;
    }
    else
    {
        // Wrap the BSDF with a BSDF*float multiply when no weight input exists.
        string mulName = namePrefix + "_mul";
        ShaderNode* mulNode = graph.createNode(mulName, mulName, mulBsdfDef, context);
        ShaderInput* mulIn1 = mulNode->getInput("in1");
        ShaderInput* mulIn2 = mulNode->getInput("in2");
        if (mulIn1 && mulIn2)
        {
            mulIn1->makeConnection(bsdfUpstream);
            mulIn2->makeConnection(weightSource);
            return mulNode->getOutput();
        }
        return bsdfUpstream;
    }
}

} // anonymous namespace

//
// NodeElisionRefactor
//

const string& NodeElisionRefactor::getName() const
{
    static const string name = "nodeElision";
    return name;
}

size_t NodeElisionRefactor::execute(ShaderGraph& graph, GenContext& context)
{
    size_t numEdits = 0;
    for (ShaderNode* node : graph.getNodes())
    {
        if (node->hasClassification(ShaderNode::Classification::CONSTANT))
        {
            if (node->numInputs() != 1 || node->numOutputs() != 1)
            {
                // Constant node doesn't follow expected interface, cannot elide.
                continue;
            }
            // Constant nodes can be elided by moving their value downstream.
            bool canElide = context.getOptions().elideConstantNodes;
            if (!canElide)
            {
                // We always elide filename constant nodes regardless of the
                // option. See DOT below.
                ShaderInput* in = node->getInput("value");
                if (in && in->getType() == Type::FILENAME)
                {
                    canElide = true;
                }
            }
            if (canElide)
            {
                graph.bypass(node, 0);
                ++numEdits;
            }
        }
        else if (node->hasClassification(ShaderNode::Classification::DOT))
        {
            if (node->numOutputs() != 1)
            {
                // Dot node doesn't follow expected interface, cannot elide.
                continue;
            }
            // Filename dot nodes must be elided so they do not create extra samplers.
            ShaderInput* in = node->getInput("in");
            if (in && in->getType() == Type::FILENAME)
            {
                graph.bypass(node, 0);
                ++numEdits;
            }
        }
    }
    return numEdits;
}

//
// PremultipliedBsdfAddRefactor
//

const string& PremultipliedBsdfAddRefactor::getName() const
{
    static const string name = "premultipliedBsdfAdd";
    return name;
}

size_t PremultipliedBsdfAddRefactor::execute(ShaderGraph& graph, GenContext& context)
{
    if (!context.getOptions().premultipliedBsdfAdd)
    {
        return 0;
    }

    // Look up all required node definitions up front.
    ConstDocumentPtr doc = graph.getDocument();
    NodeDefPtr invertDef = doc->getNodeDef("ND_invert_float");
    NodeDefPtr mulFloatDef = doc->getNodeDef("ND_multiply_float");
    NodeDefPtr mulBsdfDef = doc->getNodeDef("ND_multiply_bsdfF");
    NodeDefPtr addBsdfDef = doc->getNodeDef("ND_add_bsdf");
    if (!invertDef || !mulFloatDef || !mulBsdfDef || !addBsdfDef)
    {
        return 0;
    }

    // Collect mix nodes with connected weights (avoid modifying the graph while iterating).
    vector<ShaderNode*> mixNodes;
    for (ShaderNode* node : graph.getNodes())
    {
        if (isBsdfMixNode(node))
        {
            ShaderInput* mix = node->getInput("mix");
            if (mix && mix->getConnection())
            {
                mixNodes.push_back(node);
            }
        }
    }

    size_t numEdits = 0;

    for (ShaderNode* mixNode : mixNodes)
    {
        ShaderInput* fgInput = mixNode->getInput("fg");
        ShaderInput* bgInput = mixNode->getInput("bg");
        ShaderInput* mixInput = mixNode->getInput("mix");
        ShaderOutput* mixOutput = mixNode->getOutput();

        if (!fgInput || !bgInput || !mixInput || !mixOutput)
        {
            continue;
        }

        ShaderOutput* mixWeightSource = mixInput->getConnection();

        // Create an invert node to compute (1 - mix).
        string invertName = mixNode->getName() + "_mix_inv";
        ShaderNode* invertNode = graph.createNode(invertName, invertName, invertDef, context);
        ShaderInput* invertIn = invertNode->getInput("in");
        if (invertIn)
        {
            invertIn->makeConnection(mixWeightSource);
        }
        ShaderOutput* invertOutput = invertNode->getOutput();

        // Fold mix weights into each BSDF side.
        string namePrefix = mixNode->getName();
        ShaderOutput* fgUpstream = foldWeightIntoBsdf(graph, context, fgInput, mixWeightSource,
                                                      namePrefix + "_fg", mulFloatDef, mulBsdfDef);
        ShaderOutput* bgUpstream = foldWeightIntoBsdf(graph, context, bgInput, invertOutput,
                                                      namePrefix + "_bg", mulFloatDef, mulBsdfDef);

        // Create an add node to replace the mix.
        string addName = mixNode->getName() + "_add";
        ShaderNode* addNode = graph.createNode(addName, addName, addBsdfDef, context);
        ShaderInput* addIn1 = addNode->getInput("in1");
        ShaderInput* addIn2 = addNode->getInput("in2");
        if (!addIn1 || !addIn2)
        {
            continue;
        }

        if (fgUpstream)
        {
            addIn1->makeConnection(fgUpstream);
        }
        if (bgUpstream)
        {
            addIn2->makeConnection(bgUpstream);
        }

        // Rewire all downstream connections from the mix output to the add output.
        graph.replaceOutput(mixOutput, addNode->getOutput());

        // Disconnect the mix node's inputs.
        fgInput->breakConnection();
        bgInput->breakConnection();
        mixInput->breakConnection();

        ++numEdits;
    }

    return numEdits;
}

//
// DistributeLayerOverMixRefactor
//

const string& DistributeLayerOverMixRefactor::getName() const
{
    static const string name = "distributeLayerOverMix";
    return name;
}

size_t DistributeLayerOverMixRefactor::execute(ShaderGraph& graph, GenContext& context)
{
    if (!context.getOptions().distributeLayerOverBsdfMix)
    {
        return 0;
    }

    // Look up all required node definitions up front.
    ConstDocumentPtr doc = graph.getDocument();
    NodeDefPtr layerBsdfDef = doc->getNodeDef("ND_layer_bsdf");
    NodeDefPtr mixBsdfDef = doc->getNodeDef("ND_mix_bsdf");
    if (!layerBsdfDef || !mixBsdfDef)
    {
        return 0;
    }

    // Collect layer nodes to process (avoid modifying the graph while iterating).
    vector<ShaderNode*> layerNodes;
    for (ShaderNode* node : graph.getNodes())
    {
        if (isLayerWithMixTop(node))
        {
            layerNodes.push_back(node);
        }
    }

    size_t numEdits = 0;

    for (ShaderNode* layerNode : layerNodes)
    {
        ShaderInput* topInput = layerNode->getInput("top");
        ShaderInput* baseInput = layerNode->getInput("base");
        ShaderOutput* layerOutput = layerNode->getOutput();

        if (!topInput || !baseInput || !layerOutput)
        {
            continue;
        }

        ShaderOutput* topConnection = topInput->getConnection();
        if (!topConnection)
        {
            continue;
        }

        ShaderNode* mixNode = topConnection->getNode();
        ShaderInput* fgInput = mixNode->getInput("fg");
        ShaderInput* bgInput = mixNode->getInput("bg");
        ShaderInput* mixInput = mixNode->getInput("mix");

        if (!fgInput || !bgInput || !mixInput)
        {
            continue;
        }

        ShaderOutput* fgUpstream = fgInput->getConnection();
        ShaderOutput* bgUpstream = bgInput->getConnection();
        ShaderOutput* baseUpstream = baseInput->getConnection();
        ShaderOutput* mixWeightSource = mixInput->getConnection();

        // Create layer(fg, base).
        string layer1Name = layerNode->getName() + "_tf";
        ShaderNode* layer1 = graph.createNode(layer1Name, layer1Name, layerBsdfDef, context);
        ShaderInput* layer1Top = layer1->getInput("top");
        ShaderInput* layer1Base = layer1->getInput("base");
        if (layer1Top && fgUpstream)
        {
            layer1Top->makeConnection(fgUpstream);
        }
        if (layer1Base && baseUpstream)
        {
            layer1Base->makeConnection(baseUpstream);
        }

        // Create layer(bg, base).
        string layer2Name = layerNode->getName() + "_notf";
        ShaderNode* layer2 = graph.createNode(layer2Name, layer2Name, layerBsdfDef, context);
        ShaderInput* layer2Top = layer2->getInput("top");
        ShaderInput* layer2Base = layer2->getInput("base");
        if (layer2Top && bgUpstream)
        {
            layer2Top->makeConnection(bgUpstream);
        }
        if (layer2Base && baseUpstream)
        {
            layer2Base->makeConnection(baseUpstream);
        }

        // Create mix(layer1, layer2, w).
        string newMixName = layerNode->getName();
        ShaderNode* newMixNode = graph.createNode(newMixName + "_mix", newMixName + "_mix", mixBsdfDef, context);
        ShaderInput* newFg = newMixNode->getInput("fg");
        ShaderInput* newBg = newMixNode->getInput("bg");
        ShaderInput* newMix = newMixNode->getInput("mix");
        if (newFg)
        {
            newFg->makeConnection(layer1->getOutput());
        }
        if (newBg)
        {
            newBg->makeConnection(layer2->getOutput());
        }
        if (newMix && mixWeightSource)
        {
            newMix->makeConnection(mixWeightSource);
        }
        else if (newMix && mixInput->getValue())
        {
            newMix->setValue(mixInput->getValue());
        }

        // Rewire downstream connections from the old layer to the new mix.
        graph.replaceOutput(layerOutput, newMixNode->getOutput());

        // Disconnect the old layer node's inputs.
        topInput->breakConnection();
        baseInput->breakConnection();

        ++numEdits;
    }

    return numEdits;
}

//
// CompoundFlatteningRefactor
//

namespace
{

// Return the graph implemented by a compound node, or nullptr if the node is not a
// compound node. ShaderNodeImpl::getGraph() is overridden only by CompoundNode and
// its subclasses, so this identifies compound nodes without this file having to
// depend on any concrete implementation type.
ShaderGraph* getCompoundGraph(const ShaderNode* node)
{
    return node->getImplementation().getGraph();
}

// Carry the value on an unconnected port across to a port that is about to lose the
// connection linking the two. Used in both directions across a compound node boundary:
// from one of its inputs to the interior inputs that input feeds, and from an interior
// output socket to the external inputs the matching output feeds.
void propagateUnconnectedValue(const ShaderInput* from, ShaderInput* to)
{
    ValuePtr value = from->getValue();
    if (!value)
    {
        // There is no value to propagate, so leave whatever default the target port
        // already declares in place.
        return;
    }

    // Preserve the authored state of the source port. Overriding a default with an
    // unauthored value must not make that value look authored, since backends use the
    // authored state to decide which parameters to emit.
    to->setValue(value, from->hasAuthoredValue());

    // Only override the target's own metadata where the source has something to say,
    // matching how ShaderGraph::bypass pushes a value downstream.
    if (!from->getPath().empty())
    {
        to->setPath(from->getPath());
    }
    if (!from->getUnit().empty())
    {
        to->setUnit(from->getUnit());
    }
    if (!from->getColorSpace().empty())
    {
        to->setColorSpace(from->getColorSpace());
    }
}

// Build a node name of the form "<parent>_<child>", uniquified against the names
// already in use in the parent graph. Unlike unique ids, node names are visible to
// backends, so a copy must not be allowed to alias an unrelated sibling that happens
// to already be called "<parent>_<child>". The chosen name is added to usedNodeNames.
string makeUniqueNodeName(const string& parentNodeName, const string& childNodeName,
                          StringSet& usedNodeNames)
{
    const string base = parentNodeName + "_" + childNodeName;
    string name = base;
    for (size_t suffix = 1; usedNodeNames.count(name); ++suffix)
    {
        name = base + "_" + std::to_string(suffix);
    }
    usedNodeNames.insert(name);
    return name;
}

// Expand a single compound node in place. Copies of the nodes inside the compound
// graph are added to the parent graph and rewired to the compound node's external
// connections. The compound node itself is left isolated, for the caller to erase.
//
// Any copy that is itself a compound node is appended to newCompoundNodes, so that
// nested compound graphs are expanded in turn.
void expandCompoundNode(ShaderGraph& graph, ShaderNode* parentNode, ShaderGraph* compoundGraph,
                        StringSet& usedNodeNames, vector<ShaderNode*>& newCompoundNodes)
{
    const string parentNodeName = parentNode->getName();
    const string parentNodeId = parentNode->getUniqueId();

    // Node names are only unique within their parent graph, so the copies must be keyed
    // by unique id rather than by name. Otherwise sibling compound nodes sharing a name
    // would map to the same key, and adding the second set of copies would release the
    // first set while the graph still holds raw pointers to them.
    auto getNewNodeId = [&parentNodeId](const ShaderNode* node) -> string
    {
        return parentNodeId + "_" + node->getUniqueId();
    };

    // First create a copy of each node inside the compound graph, carrying over all
    // non-connection data. Connections are made in a second pass, once every copy exists.
    for (ShaderNode* node : compoundGraph->getNodes())
    {
        const string newNodeName = makeUniqueNodeName(parentNodeName, node->getName(), usedNodeNames);
        ShaderNode* newNode = graph.createNode(newNodeName, getNewNodeId(node),
                                               node->getImplementationPtr(), node->getClassification());
        newNode->setMetadata(node->getMetadata());

        for (const ShaderInput* port : node->getInputs())
        {
            port->copyToPort(newNode->addInput(port->getName(), port->getType()));
        }
        for (const ShaderOutput* port : node->getOutputs())
        {
            port->copyToPort(newNode->addOutput(port->getName(), port->getType()));
        }

        if (getCompoundGraph(newNode))
        {
            newCompoundNodes.push_back(newNode);
        }
    }

    // Reproduce the connections between nodes inside the compound graph. Connections to
    // the compound graph's own sockets are handled by the two socket loops below.
    for (ShaderNode* node : compoundGraph->getNodes())
    {
        ShaderNode* newNode = graph.getNode(getNewNodeId(node));

        for (const ShaderInput* port : node->getInputs())
        {
            const ShaderOutput* upstreamConnection = port->getConnection();
            if (!upstreamConnection || upstreamConnection->getNode() == compoundGraph)
            {
                continue;
            }

            ShaderInput* newDownstreamInput = newNode->getInput(port->getName());
            if (!newDownstreamInput)
            {
                throw ExceptionShaderGenError("Could not find expected input port '" + port->getName() +
                                              "' on expanded node '" + newNode->getName() + "'");
            }

            const ShaderNode* upstreamNode = upstreamConnection->getNode();
            ShaderNode* newUpstreamNode = graph.getNode(getNewNodeId(upstreamNode));
            if (!newUpstreamNode)
            {
                throw ExceptionShaderGenError("Could not find expected upstream node '" +
                                              parentNodeName + "_" + upstreamNode->getName() + "'");
            }

            ShaderOutput* newUpstreamOutput = newUpstreamNode->getOutput(upstreamConnection->getName());
            if (!newUpstreamOutput)
            {
                throw ExceptionShaderGenError("Could not find expected upstream output '" +
                                              newUpstreamNode->getName() + "." +
                                              upstreamConnection->getName() + "'");
            }

            newDownstreamInput->makeConnection(newUpstreamOutput);
        }
    }

    // Resolve the flattened inputs that a connection from an input socket should be
    // applied to. Normally this is the matching input on the copy of the interior node,
    // but an input socket may also connect straight to an output socket, in which case
    // the compound graph just passes the value through. Output sockets are stored as
    // inputs on the graph itself, so such a connection resolves to the inputs that are
    // downstream of the parent node's matching output.
    auto getNewDownstreamInputs =
        [&graph, compoundGraph, parentNode, &parentNodeName, &getNewNodeId]
        (const ShaderInput* downstreamConnection) -> ShaderInputVec
    {
        const ShaderNode* downstreamNode = downstreamConnection->getNode();
        if (downstreamNode == compoundGraph)
        {
            ShaderOutput* parentNodeOutput = parentNode->getOutput(downstreamConnection->getName());
            if (!parentNodeOutput)
            {
                throw ExceptionShaderGenError("Could not find expected output port '" + parentNodeName +
                                              "." + downstreamConnection->getName() + "'");
            }

            // Return a copy, since connecting to these inputs modifies the connection list.
            return parentNodeOutput->getConnections();
        }

        ShaderNode* newDownstreamNode = graph.getNode(getNewNodeId(downstreamNode));
        if (!newDownstreamNode)
        {
            throw ExceptionShaderGenError("Could not find expected downstream node '" +
                                          parentNodeName + "_" + downstreamNode->getName() + "'");
        }

        ShaderInput* newDownstreamInput = newDownstreamNode->getInput(downstreamConnection->getName());
        if (!newDownstreamInput)
        {
            throw ExceptionShaderGenError("Could not find expected downstream input '" +
                                          newDownstreamNode->getName() + "." +
                                          downstreamConnection->getName() + "'");
        }

        return ShaderInputVec{ newDownstreamInput };
    };

    // Splice the compound node's incoming connections and values onto the copies.
    for (ShaderGraphInputSocket* inputSocket : compoundGraph->getInputSockets())
    {
        const ShaderInputVec downstreamConnections = inputSocket->getConnections();
        if (downstreamConnections.empty())
        {
            continue;
        }

        ShaderInput* parentNodeInput = parentNode->getInput(inputSocket->getName());
        if (!parentNodeInput)
        {
            throw ExceptionShaderGenError("Could not find expected input port '" + parentNodeName + "." +
                                          inputSocket->getName() + "'");
        }

        ShaderOutput* upstreamConnectedOutput = parentNodeInput->getConnection();
        for (const ShaderInput* downstreamConnection : downstreamConnections)
        {
            for (ShaderInput* newDownstreamInput : getNewDownstreamInputs(downstreamConnection))
            {
                if (upstreamConnectedOutput)
                {
                    newDownstreamInput->makeConnection(upstreamConnectedOutput);
                }
                else
                {
                    propagateUnconnectedValue(parentNodeInput, newDownstreamInput);
                }
            }
        }
    }

    // Splice the compound node's outgoing connections onto the copies.
    for (ShaderGraphOutputSocket* outputSocket : compoundGraph->getOutputSockets())
    {
        ShaderOutput* parentNodeOutput = parentNode->getOutput(outputSocket->getName());
        if (!parentNodeOutput)
        {
            throw ExceptionShaderGenError("Could not find expected output port '" + parentNodeName + "." +
                                          outputSocket->getName() + "'");
        }

        ShaderOutput* upstreamConnection = outputSocket->getConnection();
        if (!upstreamConnection)
        {
            // An output socket can be left carrying a value rather than a connection:
            // eliding a constant inside the compound graph pushes the constant's value
            // onto the socket and breaks the connection. Forward that value to the
            // external consumers, which are about to be disconnected from the node being
            // expanded and would otherwise silently fall back to their own defaults.
            // No copy of the connection list is needed here, unlike below, since
            // propagating a value does not modify it.
            for (ShaderInput* downstreamConnectedInput : parentNodeOutput->getConnections())
            {
                propagateUnconnectedValue(outputSocket, downstreamConnectedInput);
            }
            continue;
        }

        // A pass-through output socket is fed by an input socket rather than by an
        // interior node, and has already been rewired by the input socket loop above.
        const ShaderNode* upstreamNode = upstreamConnection->getNode();
        if (upstreamNode == compoundGraph)
        {
            continue;
        }

        ShaderNode* newUpstreamNode = graph.getNode(getNewNodeId(upstreamNode));
        if (!newUpstreamNode)
        {
            throw ExceptionShaderGenError("Could not find expected upstream node '" +
                                          parentNodeName + "_" + upstreamNode->getName() + "'");
        }

        ShaderOutput* newUpstreamOutput = newUpstreamNode->getOutput(upstreamConnection->getName());
        if (!newUpstreamOutput)
        {
            throw ExceptionShaderGenError("Could not find expected upstream output '" +
                                          newUpstreamNode->getName() + "." +
                                          upstreamConnection->getName() + "'");
        }

        // Take a copy of the connection list, since reconnecting each input modifies it.
        const ShaderInputVec downstreamConnectedInputs = parentNodeOutput->getConnections();
        for (ShaderInput* downstreamConnectedInput : downstreamConnectedInputs)
        {
            downstreamConnectedInput->makeConnection(newUpstreamOutput);
        }
    }

    // Isolate the expanded node. Its external connections have been rewired onto the
    // copies above, but an input whose socket has no interior consumers, and an output
    // that only propagated a value, are both still linked to it. Those links are broken
    // here rather than when the node is finally erased, so that a later expansion can
    // never observe a connection to an already expanded node.
    for (ShaderInput* input : parentNode->getInputs())
    {
        input->breakConnection();
    }
    for (ShaderOutput* output : parentNode->getOutputs())
    {
        output->breakConnections();
    }
}

} // anonymous namespace

const string& CompoundFlatteningRefactor::getName() const
{
    static const string name = "compoundFlattening";
    return name;
}

size_t CompoundFlatteningRefactor::execute(ShaderGraph& graph, GenContext&)
{
    // Seed the worklist with the compound nodes currently in the graph. Expanding one
    // may introduce further compound nodes, which are appended as they are created.
    // Indexing rather than iterating keeps the loop valid as the worklist grows.
    vector<ShaderNode*> worklist;
    StringSet usedNodeNames;
    for (ShaderNode* node : graph.getNodes())
    {
        usedNodeNames.insert(node->getName());
        if (getCompoundGraph(node))
        {
            worklist.push_back(node);
        }
    }

    StringSet expandedNodeIds;
    for (size_t i = 0; i < worklist.size(); ++i)
    {
        ShaderNode* node = worklist[i];
        expandCompoundNode(graph, node, getCompoundGraph(node), usedNodeNames, worklist);
        expandedNodeIds.insert(node->getUniqueId());
    }

    // The expanded nodes are erased in a single pass at the end, so that compacting the
    // node order costs one traversal rather than one per expansion.
    graph.removeNodes(expandedNodeIds);

    return expandedNodeIds.size();
}

MATERIALX_NAMESPACE_END
