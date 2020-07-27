//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/MaterialNode.h>

namespace MaterialX
{

std::unordered_set<NodePtr> getShaderNodes(const NodePtr& materialNode, const string& nodeType, const string& target)
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
        //
        const string& inputShader = input->getNodeName();
        if (!inputShader.empty())
        {
            NodePtr shaderNode = parent->getChildOfType<Node>(inputShader);
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
        }

        // Check upstream nodegraph connected to the input.
        // If no explicit output name given then scan all outputs on the nodegraph.
        //
        else
        {
            const string& inputGraph = input->getNodeGraphName();
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

vector<MaterialAssignPtr> getGeometryBindings(const NodePtr& materialNode, const string& geom)
{
    vector<MaterialAssignPtr> matAssigns;
    for (LookPtr look : materialNode->getDocument()->getLooks())
    {
        for (MaterialAssignPtr matAssign : look->getMaterialAssigns())
        {
            if (matAssign->getReferencedMaterialNode() == materialNode)
            {
                if (geomStringsMatch(geom, matAssign->getActiveGeom()))
                {
                    matAssigns.push_back(matAssign);
                    continue;
                }
                CollectionPtr coll = matAssign->getCollection();
                if (coll && coll->matchesGeomString(geom))
                {
                    matAssigns.push_back(matAssign);
                    continue;
                }
            }
        }
    }
    return matAssigns;
}

} // namespace MaterialX
