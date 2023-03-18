//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXFormat/GraphIo.h>

#include <iostream>

MATERIALX_NAMESPACE_BEGIN

static string GRAPH_INDENT = "    ";
static string GRAPH_QUOTE = "\"";

// Base class methods

string GraphIo::addNodeToSubgraph(std::unordered_map<string, StringSet>& subGraphs, 
                                  const ElementPtr node, const string& label) const
{   
    if (!node)
    {
        return EMPTY_STRING;
    }

    string subgraphNodeName = label;
    if (!_restrictedMap.empty())
    {
        subgraphNodeName = replaceSubstrings(subgraphNodeName, _restrictedMap);
    }
    const ElementPtr subgraph = node->getParent();
    if (!subgraph)
    {
        return subgraphNodeName;
    }

    // Use full path to identify sub-graphs
    // A Document has no path so even though it is a GraphElement it will not be added here
    string graphId = createValidName(subgraph->getNamePath());
    if (!graphId.empty())
    {
        subgraphNodeName = graphId + "_" + subgraphNodeName;
        if (subGraphs.count(graphId))
        {
            subGraphs[graphId].insert(subgraphNodeName);
        }
        else
        {
            StringSet newSet;
            newSet.insert(subgraphNodeName);
            subGraphs[graphId] = newSet;
        }
    }

    return subgraphNodeName;
}

void GraphIo::emitGraph(GraphElementPtr graph, const StringVec& rootPaths)
{
    DocumentPtr doc = graph->getDocument();

    _graphResult.clear();

    // Write out all connections.
    std::set<Edge> processedEdges;
    StringSet processedInterfaces;
    std::unordered_map<string, StringSet> subGraphs;

    std::vector<ElementPtr> roots;
    if (!rootPaths.empty())
    {
        for (const string& rootPath : rootPaths)
        {
            ElementPtr root = doc->getDescendant(rootPath);
            if (root)
            {
                roots.push_back(root);
            }
        }
    }
    else
    {
        // Add in any outputs or materials found
        for (OutputPtr out : graph->getOutputs())
        {
            roots.push_back(out);
        }
        for (NodePtr material : graph->getMaterialNodes())
        {
            roots.push_back(material);
        }
    }

    bool writeCategoryNames = _genOptions.getWriteCategories();
    for (ElementPtr root : roots)
    {
        bool processedAny = false;
        for (Edge edge : root->traverseGraph())
        {
            if (!processedEdges.count(edge))
            {
                processedEdges.insert(edge);

                ElementPtr upstreamElem = edge.getUpstreamElement();
                ElementPtr downstreamElem = edge.getDownstreamElement();
                ElementPtr connectingElem = edge.getConnectingElement();

                processedAny = true;

                // Add upstream nodes. 
                // - Add list of unique nodes to parent subgraph if it exists
                // - Output upstream node + label (id or category)
                string upstreamId = addNodeToSubgraph(subGraphs, upstreamElem, upstreamElem->getName());
                NodeIO nodeIO;                
                NodePtr upstreamNode = upstreamElem->asA<Node>();
                nodeIO.group = EMPTY_STRING;
                if (upstreamNode)
                {
                    NodeDefPtr upstreamNodeDef = upstreamNode->getNodeDef();                    
                    nodeIO.group = upstreamNodeDef ? upstreamNodeDef->getNodeGroup() : EMPTY_STRING;
                }
                nodeIO.identifier = upstreamId;
                nodeIO.uilabel = writeCategoryNames ? upstreamElem->getCategory() : upstreamId;
                nodeIO.category = upstreamElem->getCategory();
                nodeIO.uishape = NodeIO::NodeShape::BOX;
                emitUpstreamNode(nodeIO);

                // Add connecting edges
                //
                string outputPort;
                string outputLabel;
                string inputName;
                string channelName;
                if (connectingElem)
                {
                    inputName = connectingElem->getName();
                    // Check for channel
                    channelName = connectingElem->getAttribute(PortElement::CHANNELS_ATTRIBUTE);

                    // Check for an explicit output name
                    outputLabel = connectingElem->getAttribute(PortElement::OUTPUT_ATTRIBUTE);
                    if (!outputLabel.empty())
                    {
                        // Add the output to parent subgraph if any
                        // Upstream to Output connection
                        outputPort = addNodeToSubgraph(subGraphs, upstreamElem, upstreamId + outputLabel);                        
                    }
                }
                emitConnection(outputPort, outputLabel, inputName, channelName);

                // Add downstream
                string downstreamCategory = downstreamElem->getCategory();
                string downstreamName = downstreamElem->getName();
                string downstreamId = addNodeToSubgraph(subGraphs, downstreamElem, downstreamName);                        

                nodeIO.identifier = downstreamId;
                nodeIO.uilabel = (writeCategoryNames && (downstreamCategory != Output::CATEGORY)) ? downstreamCategory : downstreamName;
                nodeIO.category = downstreamCategory;
                nodeIO.uishape = NodeIO::NodeShape::BOX;
                nodeIO.group = EMPTY_STRING;
                NodePtr downstreamNode = downstreamElem->asA<Node>();
                if (downstreamNode)
                {
                    NodeDefPtr downstreamNodeDef = downstreamNode->getNodeDef();
                    nodeIO.group = downstreamNodeDef ? downstreamNodeDef->getNodeGroup() : EMPTY_STRING;
                }
                emitDownstreamNode(nodeIO, inputName);

                const string upstreamNodeName = upstreamNode ? upstreamNode->getName() : EMPTY_STRING;
                if (upstreamNode && !processedInterfaces.count(upstreamNodeName))
                {
                    processedInterfaces.insert(upstreamNodeName);

                    for (InputPtr input : upstreamNode->getInputs())
                    {
                        if (input->hasInterfaceName())
                        {
                            const string interfaceName = input->getInterfaceName();
                            const ElementPtr upstreamParent = upstreamNode->getParent() ? upstreamNode->getParent() : nullptr;
                            NodeGraphPtr upstreamGraph = nullptr;
                            if (upstreamNode->getParent())
                            {
                                upstreamGraph = upstreamNode->getParent()->asA<NodeGraph>();
                            }
                            const InputPtr interfaceInput = upstreamGraph ? upstreamGraph->getInput(interfaceName) : nullptr;
                            if (!interfaceInput || // Will occur with functional graphs
                                (interfaceInput && !interfaceInput->getConnectedNode()))
                            {
                                // Append an extra string on to the interfacename in case the interface name
                                // is the same as an interior node name. Note that this issue also occurs with
                                // shader generation.
                                string graphInterfaceName = addNodeToSubgraph(subGraphs, upstreamNode, input->getInterfaceName() + "INT");

                                const string interiorNodeId = createValidName(upstreamElem->getNamePath());
                                const string interiorNodeCategory = upstreamElem->getCategory();
                                const string interiorNodeLabel = writeCategoryNames ? interiorNodeCategory : interiorNodeId;

                                const string interfaceInputName = input->getInterfaceName();
                                const string interiorInputName = input->getName();

                                nodeIO.identifier = interiorNodeId;
                                nodeIO.uilabel = writeCategoryNames ? interiorNodeCategory : interiorNodeLabel;
                                nodeIO.category = interiorNodeCategory;
                                nodeIO.uishape = NodeIO::NodeShape::ROUNDEDBOX;
                                emitInterfaceConnection(graphInterfaceName, interfaceInputName,
                                                        interiorInputName, nodeIO);
                            }
                        }
                    }
                }

            }
        }

        if (!processedAny)
        {
            // Only add in the root node if no connections found during traversal
            NodeIO nodeIO;
            nodeIO.identifier = createValidName(root->getNamePath());
            nodeIO.category = root->getCategory();
            nodeIO.uilabel = writeCategoryNames ? nodeIO.category : nodeIO.identifier;
            nodeIO.uishape = NodeIO::NodeShape::BOX;
            emitRootNode(nodeIO);
        }
    }

    // Add output for nodes in subgraphs if option set
    if (_genOptions.getWriteSubgraphs())
    {
        emitSubgraphs(subGraphs);
    }

    // Output entire graph
    emitGraphString();
}

// dot class methods

DotGraphIoPtr DotGraphIo::create()
{
    return std::shared_ptr<DotGraphIo>(new DotGraphIo());
}

void DotGraphIo::emitRootNode(const NodeIO& root)
{
    _graphResult += GRAPH_INDENT + root.identifier + " [label= \"" + root.uilabel + "\"]\n";
    _graphResult += GRAPH_INDENT + root.identifier + "[shape = box];\n";
    _graphResult += GRAPH_INDENT + root.identifier;
}

void DotGraphIo::emitUpstreamNode(
    const NodeIO& node)
{
    _graphResult += GRAPH_INDENT + node.identifier + " [label= \"" + node.uilabel + "\"];\n";
    const string shape = (node.group == NodeDef::CONDITIONAL_NODE_GROUP) ? "diamond" : "box";
    _graphResult += GRAPH_INDENT + node.identifier + "[shape = " + shape + "];\n";
    _graphResult += GRAPH_INDENT + node.identifier;
}

void DotGraphIo::emitConnection(const string& outputName,
                                   const string& outputLabel,
                                   const string& inputName,
                                   const string& channelName)
{
    string result = " -> ";
    if (!inputName.empty())
    {
        if (!outputLabel.empty() && !outputName.empty())
        {
            result += outputName + ";\n";
            if (channelName.empty())
            {
                result += GRAPH_INDENT + outputName + " [label= \"" + outputLabel + "." + channelName + "\"];\n";
            }
            else
            {
                result += GRAPH_INDENT + outputName + " [label= \"" + outputLabel + "\"];\n";
            }
            result += GRAPH_INDENT + outputName + " [label= \"" + outputLabel + "\"];\n";
            result += GRAPH_INDENT + outputName + " [shape = ellipse];\n";
            result += GRAPH_INDENT + outputName + " -> ";
        }
    }
    
    _graphResult += result;
}

void DotGraphIo::emitInterfaceConnection(const string& interfaceId,
                                         const string& interfaceInputName,
                                         const string& inputName,
                                         const NodeIO& interiorNode)
{
    string result;
    result += GRAPH_INDENT + interfaceId + " [label=\"" + interfaceInputName + "\"];\n";
    result += GRAPH_INDENT + interfaceId + " [shape = ellipse];\n";
    result += GRAPH_INDENT + interiorNode.identifier + " [label=\"" + interiorNode.uilabel + "\"];\n";
    result += GRAPH_INDENT + interfaceId + " -> " + interiorNode.identifier +
        " [label=" + GRAPH_QUOTE + "." + inputName + GRAPH_QUOTE + "];\n";

    _graphResult += result;
}

void DotGraphIo::emitDownstreamNode(const NodeIO& node, const string& inputLabel)
{
    string result;
    result += GRAPH_INDENT + node.identifier;
    if (!inputLabel.empty())
    {
        result += " [label= \"" + inputLabel + "\"]";
    }
    result += ";\n";
    
    result += GRAPH_INDENT + node.identifier + " [label= \"" + node.uilabel + "\"];\n";
    const string shape = (node.group == NodeDef::CONDITIONAL_NODE_GROUP) ? "diamond" : "box";
    result += GRAPH_INDENT + node.identifier + "[shape = " + shape + "]; \n";

    _graphResult += result;
}

void DotGraphIo::emitSubgraphs(std::unordered_map<string, StringSet> subGraphs)
{
    if (!_genOptions.getWriteSubgraphs() || subGraphs.empty())
    {
        return;
    }

    string result = EMPTY_STRING;
    unsigned int clusterNumber = 1;
    const string CLUSTER_STRING = "cluster_";
    for (const auto& subGraph : subGraphs)
    {
        // Note that the graph must start with the prefix "cluster"
        result += GRAPH_INDENT + "subgraph " + CLUSTER_STRING + std::to_string(clusterNumber)+ "{\n";
        result += GRAPH_INDENT + "  style = filled;\n";
        result += GRAPH_INDENT + "  fillcolor = lightyellow;\n";
        result += GRAPH_INDENT + "  color = black;\n";
        result += GRAPH_INDENT + "  node[style = filled, fillcolor = white];\n";
        result += GRAPH_INDENT + "  label = \"" + subGraph.first + "\";\n";

        for (auto item : subGraph.second)
        {
            result += GRAPH_INDENT + "  " + item + "\n";
        }
        result += GRAPH_INDENT  + "}\n\n";

        clusterNumber++;
    }

    // Needs to emit subgraphs before the graph for GraphViz output.
    _graphResult = result + _graphResult;
}

void DotGraphIo::emitGraphString()
{
    string result;
    if (_genOptions.getWriteGraphHeader())
    {
        std::unordered_map<int, string> orientations;
        orientations[(int)GraphIoGenOptions::Orientation::TOP_DOWN] = "  rankdir = TD;\n";
        orientations[(int)GraphIoGenOptions::Orientation::BOTTOM_UP] = "  rankdir = BT;\n";
        orientations[(int)GraphIoGenOptions::Orientation::LEFT_RIGHT] = "  rankdir = LR;\n";
        orientations[(int)GraphIoGenOptions::Orientation::RIGHT_LEFT] = "  rankdir = RL\n";

        result = "digraph {\n";
        result += orientations[(int)_genOptions.getOrientation()];
    }
    
    result += _graphResult;

    if (_genOptions.getWriteGraphHeader())
    {
        result += "}\n";
    }

    _graphResult = result;
}

string DotGraphIo::write(GraphElementPtr graph, const StringVec& roots)
{
    emitGraph(graph, roots);
    return _graphResult;
}

// Mermaid graph methods

MermaidGraphIoPtr MermaidGraphIo::create()
{
    return std::shared_ptr<MermaidGraphIo>(new MermaidGraphIo());
}

void MermaidGraphIo::emitUpstreamNode(const NodeIO& node)
{
    string result;
    if (node.group == NodeDef::CONDITIONAL_NODE_GROUP)
    {
        result = GRAPH_INDENT + node.identifier + "{" + node.uilabel + "}";
    } 
    else
    {
        result = GRAPH_INDENT + node.identifier + "[" + node.uilabel + "]";
    }
    _graphResult += result;
}

void MermaidGraphIo::emitConnection(const string& outputName,
                                    const string& outputLabel, 
                                    const string& inputName, 
                                    const string& channelName)
{
    string result;

    if (!inputName.empty())
    {
        string fullLabel;
        if (!channelName.empty())
        {
            fullLabel = GRAPH_QUOTE + "." + channelName + " -> ." + inputName + GRAPH_QUOTE;
        }
        else
        {
            fullLabel = GRAPH_QUOTE + "." + inputName + GRAPH_QUOTE;
        }
        if (!outputLabel.empty() && !outputName.empty())
        {
            result = " --> " + outputName + "([" + outputLabel + "])\n";
            result += GRAPH_INDENT + "style " + outputName + " fill:#1b1, color:#111\n";
            result += GRAPH_INDENT + outputName + " --" + fullLabel + "--> ";
        }
        else
        {
            result = " --" + fullLabel + "--> ";
        }
    }
    else
    {
        result = " --> ";
    }

    _graphResult += result;
}

void MermaidGraphIo::emitDownstreamNode(const NodeIO& node, const string& /*inputLabel*/)
{
    string result;
    if (node.category != Output::CATEGORY)
    {
        if (node.group == NodeDef::CONDITIONAL_NODE_GROUP)
        {
            result = node.identifier + "{" + node.uilabel + "}" + "\n";
        }
        else
        {
            result = node.identifier + "[" + node.uilabel + "]" + "\n";
        }
    }
    else
    {
        result = node.identifier + "([" + node.uilabel + "])" + "\n";
        result += GRAPH_INDENT + "style " + node.identifier + " fill:#1b1, color:#111\n";
    }
    _graphResult += result;
}

void MermaidGraphIo::emitInterfaceConnection(const string& interfaceId,
                                             const string& interfaceInputName,
                                             const string& inputName,
                                             const NodeIO& interiorNode)
{
    string result;
    result = GRAPH_INDENT + interfaceId + "([" + interfaceInputName + "])";
    result += " ==." + inputName;
    result += "==> " + interiorNode.identifier + "[" + interiorNode.uilabel + "]" + "\n";
    result += GRAPH_INDENT + "style " + interfaceId + " fill:#0bb, color:#111\n";

    _graphResult += result;
}

void  MermaidGraphIo::emitRootNode(const NodeIO& root)
{
    string result = "   " + root.identifier + "[" + root.uilabel + "]\n";
    _graphResult += result;
}

void MermaidGraphIo::emitSubgraphs(std::unordered_map<string, StringSet> subGraphs)
{
    string result = EMPTY_STRING;
    if (!_genOptions.getWriteSubgraphs())
    {
        return;
    }

    for (auto subGraph : subGraphs)
    {
        result += "  subgraph " + subGraph.first + "\n";
        for (auto item : subGraph.second)
        {
            result += GRAPH_INDENT + item + "\n";
        }
        result += "  end\n";
    }

    _graphResult += result;
}

void MermaidGraphIo::emitGraphString()
{
    string result;
    if (_genOptions.getWriteGraphHeader())
    {
        std::unordered_map<int, string> orientations;
        orientations[(int)GraphIoGenOptions::Orientation::TOP_DOWN] = "TD";
        orientations[(int)GraphIoGenOptions::Orientation::BOTTOM_UP] = "BT";
        orientations[(int)GraphIoGenOptions::Orientation::LEFT_RIGHT] = "LR";
        orientations[(int)GraphIoGenOptions::Orientation::RIGHT_LEFT] = "RL";

        result = "graph " + orientations[(int)_genOptions.getOrientation()] + "; \n";
    }
    result += _graphResult;
    _graphResult = result;
}

string MermaidGraphIo::write(GraphElementPtr graph, const StringVec& roots)
{
    emitGraph(graph, roots);
    return _graphResult;
}

MATERIALX_NAMESPACE_END
