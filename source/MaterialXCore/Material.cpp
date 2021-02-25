//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Material.h>

namespace MaterialX
{

std::unordered_set<NodePtr> getShaderNodes(NodePtr materialNode, const string& nodeType, const string& target)
{
    ElementPtr parent = materialNode->getParent();
    if (!parent)
    {
        throw Exception("Could not find a parent for material node '" + (materialNode ? materialNode->getNamePath() : EMPTY_STRING) + "'");
    }

    std::unordered_set<NodePtr> shaderNodes;

    std::vector<InputPtr> inputs = materialNode->getActiveInputs();
    if (inputs.empty())
    {
        // Try to find material nodes in the implementation graph if any.
        // If a target is specified the nodedef for the given target is searched for.
        NodeDefPtr materialNodeDef = materialNode->getNodeDef(target);
        if (materialNodeDef)
        {
            InterfaceElementPtr impl = materialNodeDef->getImplementation();
            if (impl->isA<NodeGraph>())
            {
                NodeGraphPtr implGraph = impl->asA<NodeGraph>();
                for (auto defOutput : materialNodeDef->getOutputs())
                {
                    if (defOutput->getType() == MATERIAL_TYPE_STRING)
                    {
                        OutputPtr implGraphOutput = implGraph->getOutput(defOutput->getName());
                        for (GraphIterator it = implGraphOutput->traverseGraph().begin(); it != GraphIterator::end(); ++it)
                        {
                            ElementPtr upstreamElem = it.getUpstreamElement();
                            if (!upstreamElem)
                            {
                                it.setPruneSubgraph(true);
                                continue;
                            }
                            NodePtr upstreamNode = upstreamElem->asA<Node>();
                            if (upstreamNode && upstreamNode->getType() == MATERIAL_TYPE_STRING)
                            {
                                std::unordered_set<NodePtr> newShaderNodes = getShaderNodes(upstreamNode, nodeType, target);
                                if (!newShaderNodes.empty())
                                {
                                    shaderNodes.insert(newShaderNodes.begin(), newShaderNodes.end());
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    for (const InputPtr& input : inputs) 
    {
        // Scan for a node directly connected to the input.
        // Note that this will handle traversing through interfacename associations.
        //
        NodePtr shaderNode = input->getConnectedNode();
        if (shaderNode)
        {
            if (!nodeType.empty() && shaderNode->getType() != nodeType)
            {
                continue;
            }
                
            if (!target.empty())
            {
                NodeDefPtr nodeDef = shaderNode->getNodeDef(target);
                if (!nodeDef)
                {
                    continue;
                }
            }
            shaderNodes.insert(shaderNode);
        }

        // Check upstream nodegraph connected to the input.
        // If no explicit output name given then scan all outputs on the nodegraph.
        //
        else
        {
            const string& inputGraph = input->getNodeGraphString();
            if (!inputGraph.empty())
            {
                NodeGraphPtr nodeGraph = parent->getChildOfType<NodeGraph>(inputGraph);
                if (nodeGraph)
                {
                    const string& nodeGraphOutput = input->getOutputString();
                    std::vector<OutputPtr> outputs;
                    if (!nodeGraphOutput.empty())
                    {
                        outputs.push_back(nodeGraph->getOutput(nodeGraphOutput));
                    }
                    else
                    {
                        outputs = nodeGraph->getOutputs();
                    }
                    for (OutputPtr output : outputs)
                    {
                        NodePtr upstreamNode = output->getConnectedNode();
                        if (upstreamNode)
                        {
                            if (!target.empty())
                            {
                                NodeDefPtr nodeDef = upstreamNode->getNodeDef(target);
                                if (!nodeDef)
                                {
                                    continue;
                                }
                            }
                            shaderNodes.insert(upstreamNode);
                        }
                    }
                }
            }
        }
    }
    return shaderNodes;
}

vector<OutputPtr> getConnectedOutputs(const NodePtr& node)
{
    vector<OutputPtr> outputVec;
    std::set<OutputPtr> outputSet;
    for (InputPtr input : node->getInputs())
    {
        OutputPtr output = input->getConnectedOutput();
        if (output && !outputSet.count(output))
        {
            outputVec.push_back(output);
            outputSet.insert(output);
        }
    }
    return outputVec;
}

} // namespace MaterialX
