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

MX_CORE_API vector<InterfaceElementPtr> getMaterialNodes(ElementPtr root, bool skipIncludes)
{
    vector<InterfaceElementPtr> materialNodes;
    std::unordered_set<ElementPtr> processedSources;

    // Handle the case where the root is a graph element ( document or a nodegraph )
    GraphElementPtr graphElement = root->asA<GraphElement>();
    if (graphElement)
    {
        DocumentPtr document = graphElement->asA<Document>();
        NodeGraphPtr nodeGraph = graphElement->asA<NodeGraph>();

        // Look for any material nodes exposed as outputs
        // on the nodegraph. Any other nodes are assumed to 
        // be "hidden" on purpose.
        if (nodeGraph)
        {
            for (auto graphOutput : nodeGraph->getOutputs())
            {
                if (graphOutput->getType() == MATERIAL_TYPE_STRING)
                {
                    NodePtr node = graphOutput->getConnectedNode();
                    if (node && (node->getType() == MATERIAL_TYPE_STRING) && 
                        !processedSources.count(node))
                    {
                        materialNodes.push_back(node);
                        processedSources.insert(node);
                    }
                }
            }
        }

        // Find all nodes which have a "material" output at the document level
        else if (document)
        {
            const std::string documentUri = document->getSourceUri();

            // Look for any nodegraphs with "material" outputs.
            vector<OutputPtr> testOutputs;
            for (NodeGraphPtr docNodeGraph : document->getNodeGraphs())
            {
                // Skip nodegraphs which are definitions or optionally 
                // skip any nodegraphs from an include file
                if (docNodeGraph->hasAttribute(InterfaceElement::NODE_DEF_ATTRIBUTE) ||
                    (skipIncludes && (documentUri != docNodeGraph->getSourceUri())))
                {
                    continue;
                }

                // Find if any output is connected to a material node
                // If so then include the nodegraph in the list to return.
                for (auto graphOutput : docNodeGraph->getOutputs())
                {
                    if (graphOutput->getType() == MATERIAL_TYPE_STRING)
                    {
                        NodePtr node = graphOutput->getConnectedNode();
                        if (node && node->getType() == MATERIAL_TYPE_STRING &&
                            !processedSources.count(docNodeGraph))
                        {
                            materialNodes.push_back(docNodeGraph);
                            processedSources.insert(node);
                            break;
                        }
                    }
                }
            }

            // Look for material nodes
            vector<NodePtr> docMaterialNodes = document->getMaterialNodes();
            for (auto node : docMaterialNodes)
            {
                if (!processedSources.count(node))
                {
                    materialNodes.push_back(node);
                }
            }
        }
    }

    // Look for materials associated with a material assignment
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
                        if (node && (node->getType() == MATERIAL_TYPE_STRING) && 
                            !processedSources.count(node))
                        {
                            materialNodes.push_back(node);
                            processedSources.insert(node);
                        }
                    }
                }
            }
        }
    }

    return materialNodes;
}

} // namespace MaterialX
