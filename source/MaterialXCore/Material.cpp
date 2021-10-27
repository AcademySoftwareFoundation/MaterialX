//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Node.h>
#include <MaterialXCore/Material.h>

#include <unordered_set>

namespace MaterialX
{

vector<NodePtr> getShaderNodes(NodePtr materialNode, const string& nodeType, const string& target)
{
    vector<NodePtr> shaderNodeVec;
    std::set<NodePtr> shaderNodeSet;

    vector<InputPtr> inputs = materialNode->getActiveInputs();
    for (InputPtr input : inputs) 
    {
        // Scan for a node directly connected to the input.
        // Note that this will handle traversing through interfacename associations.
        NodePtr shaderNode = input->getConnectedNode();
        if (shaderNode && !shaderNodeSet.count(shaderNode))
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

            shaderNodeVec.push_back(shaderNode);
            shaderNodeSet.insert(shaderNode);
        }
        else if (input->hasNodeGraphString())
        {
            // Check upstream nodegraph connected to the input.
            // If no explicit output name given then scan all outputs on the nodegraph.
            ElementPtr parent = materialNode->getParent();
            NodeGraphPtr nodeGraph = parent->getChildOfType<NodeGraph>(input->getNodeGraphString());
            if (!nodeGraph)
            {
                continue;
            }
            vector<OutputPtr> outputs;
            if (input->hasOutputString())
            {
                outputs.push_back(nodeGraph->getOutput(input->getOutputString()));
            }
            else
            {
                outputs = nodeGraph->getOutputs();
            }
            for (OutputPtr output : outputs)
            {
                NodePtr upstreamNode = output->getConnectedNode();
                if (upstreamNode && !shaderNodeSet.count(upstreamNode))
                {
                    if (!target.empty() && !upstreamNode->getNodeDef(target))
                    {
                        continue;
                    }
                    shaderNodeVec.push_back(upstreamNode);
                    shaderNodeSet.insert(upstreamNode);
                }
            }
        }
    }

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
                for (OutputPtr defOutput : materialNodeDef->getOutputs())
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
                                for (NodePtr shaderNode : getShaderNodes(upstreamNode, nodeType, target))
                                {
                                    if (!shaderNodeSet.count(shaderNode))
                                    {
                                        shaderNodeVec.push_back(shaderNode);
                                        shaderNodeSet.insert(shaderNode);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return shaderNodeVec;
}

vector<OutputPtr> getConnectedOutputs(NodePtr node)
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

MX_CORE_API vector<NodePtr> getMaterialNodes(ElementPtr root, bool addUnconnectedNodes, bool skipIncludes)
{
    vector<NodePtr> materialNodes;
    std::unordered_set<ElementPtr> processedSources;

    // Handle the case that the root is a graph element. In this either
    // a document or nodegraph
    GraphElementPtr graphElement = root->asA<GraphElement>();
    if (graphElement)
    {
        DocumentPtr document = graphElement->asA<Document>();
        NodeGraphPtr nodeGraph = graphElement->asA<NodeGraph>();

        // Look for any material nodes exposed as outputs
        if (nodeGraph)
        {
            for (auto graphOutput : nodeGraph->getOutputs())
            {
                if (graphOutput->getType() == MATERIAL_TYPE_STRING)
                {
                    NodePtr node = graphOutput->getConnectedNode();
                    if (node && node->getType() == MATERIAL_TYPE_STRING)
                    {
                        if (!processedSources.count(node))
                        {
                            materialNodes.push_back(node);
                            processedSources.insert(node);
                        }
                    }
                }
            }

            // Q: Should there be an option to check for stray material nodes ?
            if (addUnconnectedNodes)
            {
                vector<NodePtr> allMaterialNodes = nodeGraph->getMaterialNodes();
                for (auto node : allMaterialNodes)
                {
                    if (!processedSources.count(node))
                    {
                        materialNodes.push_back(node);
                        processedSources.insert(node);
                    }
                }
            }
        }

        // Find all material nodes in the document at the top level or inside a nodegraph.
        // Finds all nodes in a top level graph or connected to a top level output first.
        // Optionally looks for stray nodes.
        else if (document)
        {
            const std::string documentUri = document->getSourceUri();

            // Look for a nodegraph which outputs materials
            vector<OutputPtr> testOutputs;
            for (NodeGraphPtr docNodeGraph : document->getNodeGraphs())
            {
                // Skip anything from an include file including libraries.
                // Skip any nodegraph which is a definition
                if (!docNodeGraph->hasAttribute(InterfaceElement::NODE_DEF_ATTRIBUTE))
                {
                    if (skipIncludes && (documentUri != docNodeGraph->getSourceUri()))
                    {
                        continue;
                    }
                    for (auto graphOutput : docNodeGraph->getOutputs())
                    {
                        if (graphOutput->getType() == MATERIAL_TYPE_STRING)
                        {
                            testOutputs.push_back(graphOutput);
                        }
                    }
                }
            }

            // Add in all top-level outputs not already processed.
            auto docOutputs = document->getOutputs();
            for (auto docOutput : docOutputs)
            {
                if (skipIncludes && (documentUri != docOutput->getSourceUri()))
                {
                    continue;
                }
                if (docOutput->getType() == MATERIAL_TYPE_STRING)
                {
                    testOutputs.push_back(docOutput);
                }
            }

            for (OutputPtr output : testOutputs)
            {
                if (processedSources.count(output))
                {
                    continue;
                }
                processedSources.insert(output);

                NodePtr node = output->getConnectedNode();
                if (node->getType() == MATERIAL_TYPE_STRING)
                {
                    if (!processedSources.count(node))
                    {
                        materialNodes.push_back(node);
                        processedSources.insert(node);
                    }
                }
            }

            // Add in stray material nodes
            if (addUnconnectedNodes)
            {
                vector<NodePtr> allMaterialNodes = document->getMaterialNodes();
                for (auto node : allMaterialNodes)
                {
                    if (!processedSources.count(node))
                    {
                        materialNodes.push_back(node);
                        processedSources.insert(node);
                    }
                }
            }
        }
    }

    // Handle the case where it's a material assignment
    MaterialAssignPtr materialAssign = root->asA<MaterialAssign>();
    if (materialAssign)
    {
        NodePtr materialNode = materialAssign->getReferencedMaterial();
        if (materialNode && (materialNode->getType() == MATERIAL_TYPE_STRING) &&
            !processedSources.count(materialNode) )
        {
            materialNodes.push_back(materialNode);
        }
        else
        {
            NodeGraphPtr materialGraph = materialAssign->resolveRootNameReference<NodeGraph>(materialAssign->getMaterial());
            if (materialGraph)
            {
                for (auto graphOutput : materialGraph->getOutputs())
                {
                    if (graphOutput->getType() == MATERIAL_TYPE_STRING)
                    {
                        NodePtr node = graphOutput->getConnectedNode();
                        if (node && node->getType() == MATERIAL_TYPE_STRING)
                        {
                            if (!processedSources.count(node))
                            {
                                materialNodes.push_back(node);
                                processedSources.insert(node);
                            }
                        }
                    }
                }
            }
        }
    }

    return materialNodes;
}


} // namespace MaterialX
