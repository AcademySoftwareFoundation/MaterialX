//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/Util.h>

#include <MaterialXGenShader/HwShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{
    bool isEqual(const float& v1, const float& v2)
    {
        const float EPSILON = 0.00001f;
        return std::abs(v1 - v2) < EPSILON;
    }

    bool isEqual(ValuePtr value, const float& f)
    {
        if (value->isA<float>() && isEqual(value->asA<float>(), f))
        {
            return true;
        }
        else if (value->isA<Color3>())
        {
            const Color3& color = value->asA<Color3>();
            if (isEqual(color[0], f) && isEqual(color[1], f) && isEqual(color[2], f))
            {
                return true;
            }
        }
        return false;
    }

    using OpaqueTestPair = std::pair<string, float>;
    using OpaqueTestPairList = vector<OpaqueTestPair>;

    // Get corresponding input for an interfacename for a nodegraph. 
    // The check is done for any corresponding nodedef first and then for 
    // any direct child input of the nodegraph.
    InputPtr getInputInterface(const string& interfaceName , NodePtr node)
    {
        InputPtr interfaceInput = nullptr;
        ElementPtr parent = node->getParent();
        NodeGraphPtr nodeGraph = parent ? parent->asA<NodeGraph>() : nullptr;
        if (nodeGraph)
        {
            NodeDefPtr nodeDef = nodeGraph->getNodeDef();
            if (nodeDef)
            {
                interfaceInput = nodeDef->getInput(interfaceName);
            }
            else
            {
                interfaceInput = nodeGraph->getInput(interfaceName);
            }
        }
        return interfaceInput;
    }

    bool hasTransparentInputs(const OpaqueTestPairList& opaqueInputList, NodePtr node)
    {
        for (auto opaqueInput : opaqueInputList)
        {
            InputPtr interfaceInput = node->getInput(opaqueInput.first);
            if (interfaceInput)
            {
                if (interfaceInput->getConnectedNode())
                {
                    return true;
                }
                ValuePtr value = interfaceInput->getValue();
                if (value && !isEqual(value, opaqueInput.second))
                {
                    return true;
                }
            }
        }
        return false;
    }

    bool isTransparentShaderNode(NodePtr node, NodePtr interfaceNode)
    {
        if (!node || node->getType() != SURFACE_SHADER_TYPE_STRING)
        {
            return false;
        }

        // Inputs on a surface shader which are checked for transparency
        const OpaqueTestPairList inputPairList = { {"opacity", 1.0f},
                                                   {"existence", 1.0f},
                                                   {"alpha", 1.0f},
                                                   {"transmission", 0.0f} };


        // Check against the interface if a node is passed in to check against
        OpaqueTestPairList interfaceNames;
        if (interfaceNode)
        {
            for (auto inputPair : inputPairList)
            {
                InputPtr checkInput = node->getActiveInput(inputPair.first);
                if (checkInput)
                {
                    const string& interfaceName = checkInput->getInterfaceName();
                    if (!interfaceName.empty())
                    {
                        interfaceNames.push_back(std::make_pair(interfaceName, inputPair.second));
                    }
                }
            }
            if (!interfaceNames.empty())
            {
                if (hasTransparentInputs(interfaceNames, interfaceNode))
                {
                    return true;
                }
            }
        }

        // Check against the child input or the corresponding
        // functional nodegraph's interface if the input is mapped
        // via an interface name.
        for (auto inputPair : inputPairList)
        {
            InputPtr checkInput = node->getActiveInput(inputPair.first);
            if (checkInput)
            {
                const string& interfaceName = checkInput->getInterfaceName();
                if (!interfaceName.empty())
                {
                    InputPtr interfaceInput = getInputInterface(interfaceName, node);
                    if (interfaceInput)
                    {
                        checkInput = interfaceInput;
                    }
                    else
                    {
                        return false;
                    }
                }

                // If mapped but not an adjustment then assume transparency
                NodePtr inputNode = checkInput->getConnectedNode();
                if (inputNode)
                {
                    NodeDefPtr nodeDef = inputNode->getNodeDef();
                    if (nodeDef && nodeDef->getAttribute(NodeDef::NODE_GROUP_ATTRIBUTE) != NodeDef::ADJUSTMENT_NODE_GROUP)
                    {
                        return true;
                    }
                }
                else
                {
                    ValuePtr value = checkInput->getValue();
                    if (value && !isEqual(value, inputPair.second))
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool isTransparentShaderGraph(OutputPtr output, const string& target, NodePtr interfaceNode)
    {
        for (GraphIterator it = output->traverseGraph().begin(); it != GraphIterator::end(); ++it)
        {
            ElementPtr upstreamElem = it.getUpstreamElement();
            if (!upstreamElem)
            {
                continue;
            }

            if (upstreamElem->isA<Node>())
            {
                // Handle shader nodes.
                NodePtr node = upstreamElem->asA<Node>();
                if (isTransparentShaderNode(node, interfaceNode))
                {
                    return true;
                }

                // Handle graph definitions.
                NodeDefPtr nodeDef = node->getNodeDef();
                if (nodeDef)
                {
                    const TypeDesc* nodeDefType = TypeDesc::get(nodeDef->getType());
                    if (nodeDefType == Type::BSDF)
                    {
                        InterfaceElementPtr impl = nodeDef->getImplementation(target);
                        if (impl && impl->isA<NodeGraph>())
                        {
                            NodeGraphPtr graph = impl->asA<NodeGraph>();
                            vector<OutputPtr> outputs = graph->getActiveOutputs();
                            if (outputs.size() > 0)
                            {
                                const OutputPtr& graphOutput = outputs[0];
                                if (isTransparentShaderGraph(graphOutput, target, node))
                                {
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }

        return false;
    }
}

bool isTransparentSurface(ElementPtr element, const string& target)
{
    NodePtr node = element->asA<Node>();
    if (node)
    {
        // Handle material nodes.
        if (node->getCategory() == SURFACE_MATERIAL_NODE_STRING)
        {
            vector<NodePtr> shaderNodes = getShaderNodes(node);
            if (!shaderNodes.empty())
            {
                node = shaderNodes[0];
            }
        }

        // Handle shader nodes.
        if (isTransparentShaderNode(node, nullptr))
        {
            return true;
        }

        // Handle graph definitions.
        NodeDefPtr nodeDef = node->getNodeDef();
        InterfaceElementPtr impl = nodeDef ? nodeDef->getImplementation(target) : nullptr;
        if (impl && impl->isA<NodeGraph>())
        {
            NodeGraphPtr graph = impl->asA<NodeGraph>();

            vector<OutputPtr> outputs = graph->getActiveOutputs();
            if (!outputs.empty())
            {
                const OutputPtr& output = outputs[0];
                if (output->getType() == SURFACE_SHADER_TYPE_STRING)
                {
                    if (isTransparentShaderGraph(output, target, node))
                    {
                        return true;
                    }
                }
            }
        }
    }
    else if (element->isA<Output>())
    {
        // Handle output elements.
        OutputPtr output = element->asA<Output>();
        NodePtr outputNode = output->getConnectedNode();
        if (outputNode)
        {
            return isTransparentSurface(outputNode, target);
        }
    }

    return false;
}

void mapValueToColor(ConstValuePtr value, Color4& color)
{
    color = { 0.0, 0.0, 0.0, 1.0 };
    if (!value)
    {
        return;
    }
    if (value->isA<float>())
    {
        color[0] = value->asA<float>();
    }
    else if (value->isA<Color3>())
    {
        Color3 v = value->asA<Color3>();
        color[0] = v[0];
        color[1] = v[1];
        color[2] = v[2];
    }
    else if (value->isA<Color4>())
    {
        color = value->asA<Color4>();
    }
    else if (value->isA<Vector2>())
    {
        Vector2 v = value->asA<Vector2>();
        color[0] = v[0];
        color[1] = v[1];
    }
    else if (value->isA<Vector3>())
    {
        Vector3 v = value->asA<Vector3>();
        color[0] = v[0];
        color[1] = v[1];
        color[2] = v[2];
    }
    else if (value->isA<Vector4>())
    {
        Vector4 v = value->asA<Vector4>();
        color[0] = v[0];
        color[1] = v[1];
        color[2] = v[2];
        color[3] = v[3];
    }
}

bool requiresImplementation(ConstNodeDefPtr nodeDef)
{
    if (!nodeDef)
    {
        return false;
    }
    static string ORGANIZATION_STRING("organization");
    if (nodeDef->getNodeGroup() == ORGANIZATION_STRING)
    {
        return false;
    }
    static string TYPE_NONE("none");
    const string& typeAttribute = nodeDef->getType();
    return !typeAttribute.empty() && typeAttribute != TYPE_NONE;
}

bool elementRequiresShading(ConstTypedElementPtr element)
{
    string elementType(element->getType());
    static StringSet colorClosures =
    {
        "material", "surfaceshader", "volumeshader", "lightshader",
        "BSDF", "EDF", "VDF"
    };
    return colorClosures.count(elementType) > 0;
}

void findRenderableMaterialNodes(ConstDocumentPtr doc, 
                                 vector<TypedElementPtr>& elements, 
                                 bool includeReferencedGraphs,
                                 std::unordered_set<ElementPtr>& processedSources)
{
    for (const NodePtr& material : doc->getMaterialNodes())
    {
        // Scan for any upstream shader outputs and put them on the "processed" list
        // if we don't want to consider them for rendering.
        vector<NodePtr> shaderNodes = getShaderNodes(material);
        if (!shaderNodes.empty())
        {
            // Push the material node only once if any shader nodes are found
            elements.push_back(material);
            processedSources.insert(material);

            if (!includeReferencedGraphs)
            {
                for (NodePtr shaderNode : shaderNodes)
                {
                    for (InputPtr input : shaderNode->getActiveInputs())
                    {
                        OutputPtr outputPtr = input->getConnectedOutput();
                        if (outputPtr && !outputPtr->hasSourceUri() && !processedSources.count(outputPtr))
                        {
                            processedSources.insert(outputPtr);
                        }
                    }
                }
            }
        }
    }
}

void findRenderableElements(ConstDocumentPtr doc, vector<TypedElementPtr>& elements, bool includeReferencedGraphs)
{
    std::unordered_set<ElementPtr> processedSources;

    findRenderableMaterialNodes(doc, elements, includeReferencedGraphs, processedSources);

    // Find node graph outputs. Skip any light shaders
    vector<OutputPtr> testOutputs;
    for (NodeGraphPtr nodeGraph : doc->getNodeGraphs())
    {
        // Skip anything from an include file including libraries.
        // Skip any nodegraph which is a definition
        if (!nodeGraph->hasSourceUri() && !nodeGraph->hasAttribute(InterfaceElement::NODE_DEF_ATTRIBUTE))
        {
            for (auto graphOutput : nodeGraph->getOutputs())
            {
                testOutputs.push_back(graphOutput);
            }
        }
    }

    // Add in all top-level outputs not already processed.
    auto docOutputs = doc->getOutputs();
    for (auto docOutput : docOutputs)
    {
        if (!docOutput->hasSourceUri())
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
        NodePtr node = output->getConnectedNode();
        if (node && node->getType() != LIGHT_SHADER_TYPE_STRING)
        {
            NodeDefPtr nodeDef = node->getNodeDef();
            if (!nodeDef)
            {
                throw ExceptionShaderGenError("Could not find a nodedef for node '" + node->getNamePath() + "'");
            }
            if (requiresImplementation(nodeDef))
            {
                if (node->getType() == MATERIAL_TYPE_STRING)
                {
                    if (processedSources.count(node))
                    {
                        continue;
                    }
                    elements.push_back(node);
                    processedSources.insert(node);
                }
                else
                {
                    elements.push_back(output);
                }
            }
        }
        processedSources.insert(output);
    }
}

InputPtr getNodeDefInput(InputPtr nodeInput, const string& target)
{
    ElementPtr parent = nodeInput ? nodeInput->getParent() : nullptr;
    NodePtr node = parent ? parent->asA<Node>() : nullptr;
    if (node)
    {
        NodeDefPtr nodeDef = node->getNodeDef(target);
        if (nodeDef)
        {
            return nodeDef->getActiveInput(nodeInput->getName());
        }
    }

    return nullptr;
}

namespace
{
    const char TOKEN_PREFIX = '$';
}

void tokenSubstitution(const StringMap& substitutions, string& source)
{
    string buffer;
    size_t pos = 0, len = source.length();
    while (pos < len)
    {
        size_t p1 = source.find_first_of(TOKEN_PREFIX, pos);
        if (p1 != string::npos && p1 + 1 < len)
        {
            buffer += source.substr(pos, p1 - pos);
            pos = p1 + 1;
            string token = { TOKEN_PREFIX };
            while (pos < len && isalnum(source[pos]))
            {
                token += source[pos++];
            }
            auto it = substitutions.find(token);
            buffer += (it != substitutions.end() ? it->second : token);
        }
        else
        {
            buffer += source.substr(pos);
            break;
        }
    }
    source = buffer;
}

vector<Vector2> getUdimCoordinates(const StringVec& udimIdentifiers)
{
    vector<Vector2> udimCoordinates;
    if (udimIdentifiers.empty())
    {
        return udimCoordinates;
    }

    for (const string& udimIdentifier : udimIdentifiers)
    {
        if (udimIdentifier.empty())
        {
            continue;
        }

        int udimVal = std::stoi(udimIdentifier);
        if (udimVal <= 1000 || udimVal >= 2000)
        {
            throw Exception("Invalid UDIM identifier specified" + udimIdentifier);
        }

        // Compute UDIM coordinate and add to list to return
        udimVal -= 1000;
        int uVal = udimVal % 10;
        uVal = (uVal == 0) ? 9 : uVal - 1;
        int vVal = (udimVal - uVal - 1) / 10;
        udimCoordinates.emplace_back(static_cast<float>(uVal), static_cast<float>(vVal));
    }

    return udimCoordinates;
}

void getUdimScaleAndOffset(const vector<Vector2>& udimCoordinates, Vector2& scaleUV, Vector2& offsetUV)
{
    if (udimCoordinates.empty())
    {
        return;
    }

    // Find range for lower left corner of each tile based on coordinate
    Vector2 minUV = udimCoordinates[0];
    Vector2 maxUV = udimCoordinates[0];
    for (size_t i = 1; i < udimCoordinates.size(); i++)
    {
        if (udimCoordinates[i][0] < minUV[0])
        {
            minUV[0] = udimCoordinates[i][0];
        }
        if (udimCoordinates[i][1] < minUV[1])
        {
            minUV[1] = udimCoordinates[i][1];
        }
        if (udimCoordinates[i][0] > maxUV[0])
        {
            maxUV[0] = udimCoordinates[i][0];
        }
        if (udimCoordinates[i][1] > maxUV[1])
        {
            maxUV[1] = udimCoordinates[i][1];
        }
    }
    // Extend to upper right corner of a tile
    maxUV[0] += 1.0f;
    maxUV[1] += 1.0f;

    scaleUV[0] = 1.0f / (maxUV[0] - minUV[0]);
    scaleUV[1] = 1.0f / (maxUV[1] - minUV[1]);
    offsetUV[0] = -minUV[0];
    offsetUV[1] = -minUV[1];
}

NodePtr connectsToWorldSpaceNode(OutputPtr output)
{
    const StringSet WORLD_SPACE_NODE_CATEGORIES{ "normalmap" };
    NodePtr connectedNode = output ? output->getConnectedNode() : nullptr;
    if (connectedNode && WORLD_SPACE_NODE_CATEGORIES.count(connectedNode->getCategory()))
    {
        return connectedNode;
    }
    return nullptr;
}

bool hasElementAttributes(OutputPtr output, const StringVec& attributes)
{
    if (!output || attributes.empty())
    {
        return false;
    }

    for (GraphIterator it = output->traverseGraph().begin(); it != GraphIterator::end(); ++it)
    {
        ElementPtr upstreamElem = it.getUpstreamElement();
        NodePtr upstreamNode = upstreamElem ? upstreamElem->asA<Node>() : nullptr;
        if (!upstreamNode)
        {
            it.setPruneSubgraph(true);
            continue;
        }
        NodeDefPtr nodeDef = upstreamNode->getNodeDef();
        for (ValueElementPtr nodeDefElement : nodeDef->getActiveValueElements())
        {
            ValueElementPtr testElement = upstreamNode->getActiveValueElement(nodeDefElement->getName());
            if (!testElement)
            {
                testElement = nodeDefElement;
            }
            for (auto attr : attributes)
            {
                if (testElement->hasAttribute(attr))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

MATERIALX_NAMESPACE_END
