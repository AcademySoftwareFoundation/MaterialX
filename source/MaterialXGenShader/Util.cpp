//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/Util.h>

#include <MaterialXCore/MaterialNode.h>

#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>

#include <MaterialXFormat/XmlIo.h>
#include <MaterialXFormat/PugiXML/pugixml.hpp>

#include <list>

namespace MaterialX
{

namespace
{
    const float EPS_ZERO = 0.00001f;
    const float EPS_ONE  = 1.0f - EPS_ZERO;

    bool isZero(float v)
    {
        return v < EPS_ZERO;
    }

    bool isZero(const Color3& c)
    {
        return isZero(c[0]) && isZero(c[1]) && isZero(c[2]);
    }

    bool isZero(ValuePtr value)
    {
        if (value->isA<float>() && isZero(value->asA<float>()))
        {
            return true;
        }
        if (value->isA<Color3>() && isZero(value->asA<Color3>()))
        {
            return true;
        }
        return false;
    }

    bool isOne(float v)
    {
        return v > EPS_ONE;
    }

    bool isOne(const Color3& c)
    {
        return isOne(c[0]) && isOne(c[1]) && isOne(c[2]);
    }

    bool isOne(ValuePtr value)
    {
        if (value->isA<float>() && isOne(value->asA<float>()))
        {
            return true;
        }
        if (value->isA<Color3>() && isOne(value->asA<Color3>()))
        {
            return true;
        }
        return false;
    }

    bool isTransparentShaderGraph(OutputPtr output, const ShaderGenerator& shadergen,
                                  StringSet& opacityInterfaceNames, StringSet& transmissionInterfaceNames)
    {
        // Track how many nodes has the potential of being transparent
        // and how many of these we can say for sure are 100% opaque.
        size_t numCandidates = 0;
        size_t numOpaque = 0;

        for (GraphIterator it = output->traverseGraph().begin(); it != GraphIterator::end(); ++it)
        {
            ElementPtr upstreamElem = it.getUpstreamElement();
            if (!upstreamElem)
            {
                it.setPruneSubgraph(true);
                continue;
            }

            const string& typeName = upstreamElem->asA<TypedElement>()->getType();
            const TypeDesc* type = TypeDesc::get(typeName);
            bool isFourChannelOutput = type == Type::COLOR4 || type == Type::VECTOR4;
            if (type != Type::SURFACESHADER && type != Type::BSDF && !isFourChannelOutput)
            {
                it.setPruneSubgraph(true);
                continue;
            }

            if (upstreamElem->isA<Node>())
            {
                NodePtr node = upstreamElem->asA<Node>();

                const string& nodetype = node->getCategory();
                if (nodetype == "surface")
                {
                    // This is a candidate for transparency
                    ++numCandidates;

                    bool opaque = false;

                    InputPtr opacity = node->getInput("opacity");
                    if (!opacity)
                    {
                        opaque = true;
                    }
                    else if (opacity->getNodeName().empty() && opacity->getInterfaceName().empty())
                    {
                        ValuePtr value = opacity->getValue();
                        if (!value || (value->isA<float>() && isOne(value->asA<float>())))
                        {
                            opaque = true;
                        }
                    }

                    if (opaque)
                    {
                        ++numOpaque;
                    }
                }
                else if (nodetype == "dielectricbtdf")
                {
                    // This is a candidate for transparency
                    ++numCandidates;

                    bool opaque = false;

                    // First check the weight
                    InputPtr weight = node->getInput("weight");
                    if (weight && weight->getNodeName() == EMPTY_STRING && weight->getInterfaceName() == EMPTY_STRING)
                    {
                        // Unconnected, check the value
                        ValuePtr value = weight->getValue();
                        if (value && isZero(value))
                        {
                            opaque = true;
                        }
                    }

                    if (!opaque)
                    {
                        // Second check the tint
                        InputPtr tint = node->getInput("tint");
                        if (tint && tint->getNodeName() == EMPTY_STRING && tint->getInterfaceName() == EMPTY_STRING)
                        {
                            // Unconnected, check the value
                            ValuePtr value = tint->getValue();
                            if (!value || isZero(value))
                            {
                                opaque = true;
                            }
                        }
                    }

                    if (opaque)
                    {
                        ++numOpaque;
                    }
                }
                else if (nodetype == "standard_surface")
                {
                    // This is a candidate for transparency
                    ++numCandidates;

                    bool opaque = false;

                    // First check the transmission weight
                    InputPtr transmission = node->getInput("transmission");
                    if (!transmission)
                    {
                        opaque = true;
                    }
                    else
                    {
                        const string& tranmsInterfaceName = transmission->getInterfaceName();
                        if (!tranmsInterfaceName.empty())
                        {
                            transmissionInterfaceNames.insert(tranmsInterfaceName);
                        }
                        if (transmission->getNodeName().empty())
                        {
                            // Unconnected, check the value
                            ValuePtr value = transmission->getValue();
                            if (!value || isZero(value))
                            {
                                opaque = true;
                            }
                        }
                    }

                    // Second check the opacity
                    if (opaque)
                    {
                        opaque = false;

                        InputPtr opacity = node->getInput("opacity");
                        if (!opacity)
                        {
                            opaque = true;
                        }
                        else
                        {
                            const string& opacityInterfaceName = opacity->getInterfaceName();
                            if (!opacityInterfaceName.empty())
                            {
                                opacityInterfaceNames.insert(opacityInterfaceName);
                            }
                            if (opacity->getNodeName().empty())
                            {
                                // Unconnected, check the value
                                ValuePtr value = opacity->getValue();
                                if (!value || isOne(value))
                                {
                                    opaque = true;
                                }
                            }
                        }
                    }

                    if (opaque)
                    {
                        // We know for sure this is opaque
                        ++numOpaque;
                    }
                }
                else
                {
                    // If node is nodedef which references a node graph.
                    // If so, then try to examine that node graph.
                    NodeDefPtr nodeDef = node->getNodeDef();
                    if (nodeDef)
                    {
                        const TypeDesc* nodeDefType = TypeDesc::get(nodeDef->getType());
                        if (nodeDefType == Type::BSDF)
                        {
                            InterfaceElementPtr impl = nodeDef->getImplementation(shadergen.getTarget(), shadergen.getLanguage());
                            if (impl && impl->isA<NodeGraph>())
                            {
                                NodeGraphPtr graph = impl->asA<NodeGraph>();

                                vector<OutputPtr> outputs = graph->getActiveOutputs();
                                if (outputs.size() > 0)
                                {
                                    const OutputPtr& graphOutput = outputs[0];
                                    bool isTransparent = isTransparentShaderGraph(graphOutput, shadergen, opacityInterfaceNames, transmissionInterfaceNames);
                                    if (isTransparent)
                                    {
                                        return true;
                                    }
                                }
                            }
                        }
                        else if (isFourChannelOutput)
                        {
                            ++numCandidates;
                        }
                    }
                }

                if (numOpaque != numCandidates)
                {
                    // We found at least one candidate that we can't
                    // say for sure is opaque. So we might need transparency.
                    return true;
                }
            }
        }

        return numCandidates > 0 ? numOpaque != numCandidates : false;
    }
}

FileSearchPath getDefaultSearchPath()
{
    FileSearchPath searchPath;

    // Default search path for installed binaries.
    FilePath installSearchPath = FilePath::getModulePath().getParentPath();
    if (installSearchPath.exists())
    {
        searchPath.append(installSearchPath);
        searchPath.append(installSearchPath / "libraries");
    }

    // Default search path for development environments.
    FilePath devSearchPath = FilePath(__FILE__).getParentPath().getParentPath().getParentPath();
    if (devSearchPath.exists())
    {
        searchPath.append(devSearchPath);
        searchPath.append(devSearchPath / "libraries");
    }

    return searchPath;
}

bool isTransparentSurface(ElementPtr element, const ShaderGenerator& shadergen)
{
    // Handle shader nodes
    NodePtr shaderNode = element->asA<Node>();
    if (shaderNode && shaderNode->getType() == SURFACE_SHADER_TYPE_STRING)
    {
        NodeDefPtr nodeDef = shaderNode->getNodeDef();
        if (!nodeDef)
        {
            throw ExceptionShaderGenError("Could not find a nodedef for shader node '" + shaderNode->getNamePath());
        }

        // Check opacity
        InputPtr opacity = shaderNode->getActiveInput("opacity");
        if (opacity)
        {
            if (opacity->getConnectedOutput())
            {
                return true;
            }
            else
            {
                ValuePtr value = opacity->getValue();
                if (value && !isOne(value))
                {
                    return true;
                }
            }
        }

        // Check existence
        InputPtr existence = shaderNode->getActiveInput("existence");
        if (existence)
        {
            if (existence->getConnectedOutput())
            {
                return true;
            }
            else
            {
                ValuePtr value = existence->getValue();
                if (value && !isOne(value))
                {
                    return true;
                }
            }
        }

        // Check transmission
        InputPtr transmission = shaderNode->getActiveInput("transmission");
        if (transmission)
        {
            if (transmission->getConnectedOutput())
            {
                return true;
            }
            else
            {
                ValuePtr value = transmission->getValue();
                if (value && !isZero(value))
                {
                    return true;
                }
            }
        }

        // Check subsurface
        InputPtr subsurface = shaderNode->getActiveInput("subsurface");
        if (subsurface)
        {
            if (subsurface->getConnectedOutput())
            {
                return true;
            }
            else
            {
                ValuePtr value = subsurface->getValue();
                if (value && !isZero(value))
                {
                    return true;
                }
            }
        }

        // Check for a transparent graph.
        InterfaceElementPtr impl = nodeDef->getImplementation(shadergen.getTarget(), shadergen.getLanguage());
        if (!impl)
        {
            throw ExceptionShaderGenError("Could not find a matching implementation for node '" + nodeDef->getNodeString() +
                "' matching language '" + shadergen.getLanguage() + "' and target '" + shadergen.getTarget() + "'");
        }
        if (impl->isA<NodeGraph>())
        {
            NodeGraphPtr graph = impl->asA<NodeGraph>();

            vector<OutputPtr> outputs = graph->getActiveOutputs();
            if (!outputs.empty())
            {
                const OutputPtr& output = outputs[0];
                if (TypeDesc::get(output->getType()) == Type::SURFACESHADER)
                {
                    StringSet opacityInterfaceNames;
                    StringSet transmissionInterfaceNames;
                    bool isTransparent = isTransparentShaderGraph(output, shadergen, opacityInterfaceNames, transmissionInterfaceNames);

                    if (!isTransparent)
                    {
                        for (const string& opacityInterfaceName : opacityInterfaceNames)
                        {
                            opacity = shaderNode->getActiveInput(opacityInterfaceName);
                            if (opacity)
                            {
                                if (!opacity->getOutputString().empty())
                                {
                                    return true;
                                }
                                else
                                {
                                    ValuePtr value = opacity->getValue();
                                    if (value && !isOne(value))
                                    {
                                        return true;
                                    }
                                }
                            }
                        }
                        for (const string& transmissionInterfaceName : transmissionInterfaceNames)
                        {
                            transmission = shaderNode->getActiveInput(transmissionInterfaceName);
                            if (transmission)
                            {
                                if (!transmission->getOutputString().empty())
                                {
                                    return true;
                                }
                                else
                                {
                                    ValuePtr value = transmission->getValue();
                                    if (value && !isZero(value))
                                    {
                                        return true;
                                    }
                                }
                            }
                        }
                    }

                    return false;
                }
            }
        }
    }

    // Handle shader refs
    else if (element->isA<ShaderRef>())
    {
        ShaderRefPtr shaderRef = element->asA<ShaderRef>();
        NodeDefPtr nodeDef = shaderRef->getNodeDef();
        if (!nodeDef)
        {
            throw ExceptionShaderGenError("Could not find a nodedef for shaderref '" + shaderRef->getName() + "' in material " + shaderRef->getParent()->getName());
        }
        if (TypeDesc::get(nodeDef->getType()) != Type::SURFACESHADER)
        {
            return false;
        }

        // Check opacity
        BindInputPtr opacity = shaderRef->getBindInput("opacity");
        if (opacity)
        {
            if (opacity->getConnectedOutput())
            {
                return true;
            }
            else
            {
                ValuePtr value = opacity->getValue();
                if (value && !isOne(value))
                {
                    return true;
                }
            }
        }

        // Check existence
        BindInputPtr existence = shaderRef->getBindInput("existence");
        if (existence)
        {
            if (existence->getConnectedOutput())
            {
                return true;
            }
            else
            {
                ValuePtr value = existence->getValue();
                if (value && !isOne(value))
                {
                    return true;
                }
            }
        }

        // Check transmission
        BindInputPtr transmission = shaderRef->getBindInput("transmission");
        if (transmission)
        {
            if (transmission->getConnectedOutput())
            {
                return true;
            }
            else
            {
                ValuePtr value = transmission->getValue();
                if (value && !isZero(value))
                {
                    return true;
                }
            }
        }

        // Check subsurface
        BindInputPtr subsurface = shaderRef->getBindInput("subsurface");
        if (subsurface)
        {
            if (subsurface->getConnectedOutput())
            {
                return true;
            }
            else
            {
                ValuePtr value = subsurface->getValue();
                if (value && !isZero(value))
                {
                    return true;
                }
            }
        }

        // Check for a transparent graph.
        InterfaceElementPtr impl = nodeDef->getImplementation(shadergen.getTarget(), shadergen.getLanguage());
        if (!impl)
        {
            throw ExceptionShaderGenError("Could not find a matching implementation for node '" + nodeDef->getNodeString() +
                "' matching language '" + shadergen.getLanguage() + "' and target '" + shadergen.getTarget() + "'");
        }
        if (impl->isA<NodeGraph>())
        {
            NodeGraphPtr graph = impl->asA<NodeGraph>();

            vector<OutputPtr> outputs = graph->getActiveOutputs();
            if (!outputs.empty())
            {
                const OutputPtr& output = outputs[0];
                if (TypeDesc::get(output->getType()) == Type::SURFACESHADER)
                {
                    StringSet opacityInterfaceNames;
                    StringSet transmissionInterfaceNames;
                    bool isTransparent = isTransparentShaderGraph(output, shadergen, opacityInterfaceNames, transmissionInterfaceNames);

                    if (!isTransparent)
                    {
                        for (const string& opacityInterfaceName : opacityInterfaceNames)
                        {
                            opacity = shaderRef->getBindInput(opacityInterfaceName);
                            if (opacity)
                            {
                                if (!opacity->getOutputString().empty())
                                {
                                    return true;
                                }
                                else
                                {
                                    ValuePtr value = opacity->getValue();
                                    if (value && !isOne(value))
                                    {
                                        return true;
                                    }
                                }
                            }
                        }
                        for (const string& transmissionInterfaceName : transmissionInterfaceNames)
                        {
                            transmission = shaderRef->getBindInput(transmissionInterfaceName);
                            if (transmission)
                            {
                                if (!transmission->getOutputString().empty())
                                {
                                    return true;
                                }
                                else
                                {
                                    ValuePtr value = transmission->getValue();
                                    if (value && !isZero(value))
                                    {
                                        return true;
                                    }
                                }
                            }
                        }
                    }

                    return false;
                }
            }
        }
    }

    // Handle output nodes
    else if (element->isA<Output>())
    {
        OutputPtr output = element->asA<Output>();
        StringSet opacityInterfaceNames;
        StringSet transmissionInterfaceNames;
        return isTransparentShaderGraph(output, shadergen, opacityInterfaceNames, transmissionInterfaceNames);
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
    else if (value->isA<Color2>())
    {
        Color2 v = value->asA<Color2>();
        color[0] = v[0];
        color[3] = v[1]; // Component 2 maps to alpha
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
    static string TYPE_NONE("none");
    const string& typeAttribute = nodeDef->getType();
    return !typeAttribute.empty() && typeAttribute != TYPE_NONE;
}

bool elementRequiresShading(ConstTypedElementPtr element)
{
    string elementType(element->getType());
    static StringSet colorClosures =
    {
        "surfaceshader", "volumeshader", "lightshader",
        "BSDF", "EDF", "VDF"
    };
    return (element->isA<ShaderRef>() ||
            colorClosures.count(elementType) > 0);
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
        std::unordered_set<NodePtr> shaderNodes = getShaderNodes(material);
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

void findRenderableShaderRefs(ConstDocumentPtr doc, 
                              vector<TypedElementPtr>& elements, 
                              bool includeReferencedGraphs,
                              std::unordered_set<ElementPtr>& processedSources)
{
    for (const auto& material : doc->getMaterials())
    {
        for (const auto& shaderRef : material->getShaderRefs())
        {
            if (!shaderRef->hasSourceUri())
            {
                // Add in all shader references which are not part of a node definition library
                ConstNodeDefPtr nodeDef = shaderRef->getNodeDef();
                if (!nodeDef)
                {
                    throw ExceptionShaderGenError("Could not find a nodedef for shaderref '" + shaderRef->getName() +
                                                  "' in material '" + shaderRef->getParent()->getName() + "'");
                }
                if (requiresImplementation(nodeDef))
                {
                    elements.push_back(shaderRef);
                }

                if (!includeReferencedGraphs)
                {
                    // Track outputs already used by the shaderref
                    for (const auto& bindInput : shaderRef->getBindInputs())
                    {
                        OutputPtr outputPtr = bindInput->getConnectedOutput();
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
    findRenderableShaderRefs(doc, elements, includeReferencedGraphs, processedSources);

    // Find node graph outputs. Skip any light shaders
    std::list<OutputPtr> testOutputs;
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

ValueElementPtr findNodeDefChild(const string& path, DocumentPtr doc, const string& target)
{
    if (path.empty() || !doc)
    {
        return nullptr;
    }
    ElementPtr pathElement = doc->getDescendant(path);
    if (!pathElement || pathElement == doc)
    {
        return nullptr;
    }
    ElementPtr parent = pathElement->getParent();
    if (!parent || parent == doc)
    {
        return nullptr;
    }

    // Note that we must cast to a specific type derived instance as getNodeDef() is not
    // a virtual method which is overridden in derived classes.
    NodeDefPtr nodeDef = nullptr;
    ShaderRefPtr shaderRef = parent->asA<ShaderRef>();
    if (shaderRef)
    {
        nodeDef = shaderRef->getNodeDef();
    }
    else
    {
        NodePtr node = parent->asA<Node>();
        if (node)
        {
            nodeDef = node->getNodeDef(target);
        }
    }
    if (!nodeDef)
    {
        return nullptr;
    }

    // Use the path element name to look up in the equivalent element
    // in the nodedef as only the nodedef elements contain the information.
    const string& valueElementName = pathElement->getName();
    ValueElementPtr valueElement = nodeDef->getActiveValueElement(valueElementName);

    return valueElement;
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

NodePtr connectsToNodeOfCategory(OutputPtr output, const StringSet& categories)
{
    ElementPtr connectedElement = output ? output->getConnectedNode() : nullptr;
    NodePtr connectedNode = connectedElement ? connectedElement->asA<Node>() : nullptr;
    if (!connectedNode)
    {
        return nullptr;
    }
    
    // Check the direct node type
    if (categories.count(connectedNode->getCategory()))
    {
        return connectedNode;
    }

    // Check if it's a definition which has a root which of the node type
    NodeDefPtr nodedef = connectedNode->getNodeDef();
    if (nodedef)
    {
        InterfaceElementPtr inter = nodedef->getImplementation();
        if (inter)
        {
            NodeGraphPtr graph = inter->asA<NodeGraph>();
            if (graph)
            {
                for (OutputPtr outputPtr : graph->getOutputs())
                {
                    NodePtr outputNode = outputPtr->getConnectedNode();
                    if (outputNode && categories.count(outputNode->getCategory()))
                    {
                        return outputNode;
                    }
                }
            }
        }
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

} // namespace MaterialX
