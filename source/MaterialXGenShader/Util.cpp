//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/Util.h>

#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>

#include <MaterialXFormat/XmlIo.h>
#include <MaterialXFormat/PugiXML/pugixml.hpp>

#include <fstream>
#include <iostream>
#include <sstream>

namespace MaterialX
{

string removeExtension(const string& filename)
{
    size_t lastDot = filename.find_last_of('.');
    if (lastDot == string::npos) return filename;
    return filename.substr(0, lastDot);
}

bool readFile(const string& filename, string& contents)
{
    std::ifstream file(filename, std::ios::in);
    if (file)
    {
        StringStream stream;
        stream << file.rdbuf();
        file.close();
        if (stream)
        {
            contents = stream.str();
            return (contents.size() > 0);
        }
        return false;
    }
    return false;
}

void loadDocuments(const FilePath& rootPath, const FileSearchPath& searchPath, const StringSet& skipFiles,
                   const StringSet& includeFiles, vector<DocumentPtr>& documents, StringVec& documentsPaths,
                   const XmlReadOptions& readOptions, StringVec& errors)
{
    for (const FilePath& dir : rootPath.getSubDirectories())
    {
        for (const FilePath& file : dir.getFilesInDirectory(MTLX_EXTENSION))
        {
            if (!skipFiles.count(file) &&
               (includeFiles.empty() || includeFiles.count(file)))
            {
                DocumentPtr doc = createDocument();
                const FilePath filePath = dir / file;
                try
                {
                    FileSearchPath readSearchPath(searchPath);
                    readSearchPath.append(dir);
                    readFromXmlFile(doc, filePath, readSearchPath, &readOptions);
                    documents.push_back(doc);
                    documentsPaths.push_back(filePath.asString());
                }
                catch (Exception& e)
                {
                    errors.push_back("Failed to load: " + filePath.asString() + ". Error: " + e.what());
                }
            }
        }
    }
}

void loadLibrary(const FilePath& file, DocumentPtr doc, const FileSearchPath* searchPath)
{
    DocumentPtr libDoc = createDocument();
    XmlReadOptions readOptions;
    readOptions.skipConflictingElements = true;
    readFromXmlFile(libDoc, file, searchPath ? *searchPath : FileSearchPath(), &readOptions);
    CopyOptions copyOptions;
    copyOptions.skipConflictingElements = true;
    doc->importLibrary(libDoc, &copyOptions);
}

StringVec loadLibraries(const StringVec& libraryNames,
                        const FileSearchPath& searchPath,
                        DocumentPtr doc,
                        const StringSet* excludeFiles)
{
    StringVec loadedLibraries;
    for (const std::string& libraryName : libraryNames)
    {
        FilePath libraryPath = searchPath.find(libraryName);
        for (const FilePath& path : libraryPath.getSubDirectories())
        {
            for (const FilePath& filename : path.getFilesInDirectory(MTLX_EXTENSION))
            {
                if (!excludeFiles || !excludeFiles->count(filename))
                {
                    const FilePath& file = path / filename;
                    loadLibrary(file, doc, &searchPath);
                    loadedLibraries.push_back(file.asString());
                }
            }
        }
    }
    return loadedLibraries;
}

StringVec loadLibraries(const StringVec& libraryNames,
                        const FilePath& filePath,
                        DocumentPtr doc,
                        const StringSet* excludeFiles)
{
    FileSearchPath searchPath;
    searchPath.append(filePath);
    return loadLibraries(libraryNames, searchPath, doc, excludeFiles);
}

namespace
{
    const float EPS_ZERO = 0.00001f;
    const float EPS_ONE  = 1.0f - EPS_ZERO;

    inline bool isZero(float v)
    {
        return v < EPS_ZERO;
    }

    inline bool isOne(float v)
    {
        return v > EPS_ONE;
    }

    inline bool isBlack(const Color3& c)
    {
        return isZero(c[0]) && isZero(c[1]) && isZero(c[2]);
    }

    inline bool isWhite(const Color3& c)
    {
        return isOne(c[0]) && isOne(c[1]) && isOne(c[2]);
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
                        if (value && value->isA<float>() && isZero(value->asA<float>()))
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
                            if (!value || (value->isA<Color3>() && isBlack(value->asA<Color3>())))
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
                            if (!value || (value->asA<float>() && isZero(value->asA<float>())))
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
                                if (!value || (value->isA<Color3>() && isWhite(value->asA<Color3>())))
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

        const string& nodetype = nodeDef->getNodeString();
        if (nodetype == "standard_surface")
        {
            bool opaque = false;

            // First check the transmission weight
            InputPtr transmission = shaderNode->getActiveInput("transmission");
            if (!transmission)
            {
                opaque = true;
            }
            else if (transmission->getOutputString() == EMPTY_STRING)
            {
                // Unconnected, check the value
                ValuePtr value = transmission->getValue();
                if (!value || isZero(value->asA<float>()))
                {
                    opaque = true;
                }
            }

            // Second check the opacity
            if (opaque)
            {
                opaque = false;

                InputPtr opacity = shaderNode->getActiveInput("opacity");
                if (!opacity)
                {
                    opaque = true;
                }
                else if (opacity->getOutputString() == EMPTY_STRING)
                {
                    // Unconnected, check the value
                    ValuePtr value = opacity->getValue();
                    if (!value || (value->isA<Color3>() && isWhite(value->asA<Color3>())))
                    {
                        opaque = true;
                    }
                }
            }

            return !opaque;
        }
        else
        {
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
                            for (const string& transmissionInterfaceName : transmissionInterfaceNames)
                            {
                                // First check the transmission weight
                                InputPtr transmission = shaderNode->getActiveInput(transmissionInterfaceName);
                                if (transmission)
                                {
                                    if (!transmission->getOutputString().empty())
                                    {
                                        isTransparent = true;
                                    }
                                    else
                                    {
                                        ValuePtr value = transmission->getValue();
                                        if (value && !isZero(value->asA<float>()))
                                        {
                                            isTransparent = true;
                                        }
                                    }
                                }
                            }
                            for (const string& opacityInterfaceName : opacityInterfaceNames)
                            {
                                InputPtr opacity = shaderNode->getActiveInput(opacityInterfaceName);
                                if (opacity)
                                {
                                    if (!opacity->getOutputString().empty())
                                    {
                                        isTransparent = true;
                                    }
                                    else
                                    {
                                        ValuePtr value = opacity->getValue();
                                        if (value && value->isA<Color3>() && !isWhite(value->asA<Color3>()))
                                        {
                                            isTransparent = true;
                                        }
                                    }
                                }
                            }
                        }

                        return isTransparent;
                    }
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

        const string& nodetype = nodeDef->getNodeString();
        if (nodetype == "standard_surface")
        {
            bool opaque = false;

            // First check the transmission weight
            BindInputPtr transmission = shaderRef->getBindInput("transmission");
            if (!transmission)
            {
                opaque = true;
            }
            else if (transmission->getOutputString() == EMPTY_STRING)
            {
                // Unconnected, check the value
                ValuePtr value = transmission->getValue();
                if (!value || isZero(value->asA<float>()))
                {
                    opaque = true;
                }
            }

            // Second check the opacity
            if (opaque)
            {
                opaque = false;

                BindInputPtr opacity = shaderRef->getBindInput("opacity");
                if (!opacity)
                {
                    opaque = true;
                }
                else if (opacity->getOutputString() == EMPTY_STRING)
                {
                    // Unconnected, check the value
                    ValuePtr value = opacity->getValue();
                    if (!value || (value->isA<Color3>() && isWhite(value->asA<Color3>())))
                    {
                        opaque = true;
                    }
                }
            }

            return !opaque;
        }
        else
        {
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
                            for (const string& transmissionInterfaceName : transmissionInterfaceNames)
                            {
                                // First check the transmission weight
                                BindInputPtr transmission = shaderRef->getBindInput(transmissionInterfaceName);
                                if (transmission)
                                {
                                    if (!transmission->getOutputString().empty())
                                    {
                                        isTransparent = true;
                                    }
                                    else
                                    {
                                        ValuePtr value = transmission->getValue();
                                        if (value && !isZero(value->asA<float>()))
                                        {
                                            isTransparent = true;
                                        }
                                    }
                                }
                            }
                            for (const string& opacityInterfaceName : opacityInterfaceNames)
                            {
                                BindInputPtr opacity = shaderRef->getBindInput(opacityInterfaceName);
                                if (opacity)
                                {
                                    if (!opacity->getOutputString().empty())
                                    {
                                        isTransparent = true;
                                    }
                                    else
                                    {
                                        ValuePtr value = opacity->getValue();
                                        if (value && value->isA<Color3>() && !isWhite(value->asA<Color3>()))
                                        {
                                            isTransparent = true;
                                        }
                                    }
                                }
                            }
                        }

                        return isTransparent;
                    }
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


vector<NodePtr> getShaderNodes(const NodePtr materialNode, const string& shaderType, const string& target)
{
    DocumentPtr doc = materialNode->getDocument();
    vector<NodePtr> shaderNodes;
    for (const InputPtr& input : materialNode->getActiveInputs())
    {
        const string& inputShader = input->getNodeName();
        if (!inputShader.empty())
        {
            NodePtr shaderNode = doc->getNode(inputShader);
            if (shaderNode)
            {
                if (!target.empty())
                {
                    NodeDefPtr nodeDef = shaderNode->getNodeDef();
                    if (!nodeDef || !targetStringsMatch(nodeDef->getTarget(), target))
                    {
                        continue;
                    }
                }
                if (shaderType.empty() || shaderNode->getType() == shaderType)
                {
                    shaderNodes.push_back(shaderNode);
                }
            }
        }
    }
    return shaderNodes;
}

vector<MaterialAssignPtr> getGeometryBindings(NodePtr materialNode, const string& geom)
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

void findRenderableMaterialNodes(ConstDocumentPtr doc, 
                                 vector<TypedElementPtr>& elements, 
                                 bool includeReferencedGraphs,
                                 std::unordered_set<OutputPtr> &processedOutputs)
{
    for (const NodePtr& material : doc->getMaterialNodes())
    {
        // Push the material node only once.
        elements.push_back(material);

        // Scan for any upstream shader outpus and put them on the "processed" list
        // if we don't want to consider them for rendering.
        vector<NodePtr> shaderNodes = getShaderNodes(material);
        for (NodePtr shaderNode : shaderNodes)
        {
            if (!includeReferencedGraphs)
            {
                for (InputPtr input : shaderNode->getActiveInputs())
                {
                    OutputPtr outputPtr = input->getConnectedOutput();
                    if (outputPtr && !outputPtr->hasSourceUri() && !processedOutputs.count(outputPtr))
                    {
                        processedOutputs.insert(outputPtr);
                    }
                }
            }
        }
    }
}

void findRenderableShaderRefs(ConstDocumentPtr doc, 
                              vector<TypedElementPtr>& elements, 
                              bool includeReferencedGraphs,
                              std::unordered_set<OutputPtr> &processedOutputs)
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
                        if (outputPtr && !outputPtr->hasSourceUri() && !processedOutputs.count(outputPtr))
                        {
                            processedOutputs.insert(outputPtr);
                        }
                    }
                }
            }
        }
    }
}

void findRenderableElements(ConstDocumentPtr doc, vector<TypedElementPtr>& elements, bool includeReferencedGraphs)
{
    std::unordered_set<OutputPtr> processedOutputs;

    findRenderableMaterialNodes(doc, elements, includeReferencedGraphs, processedOutputs);
    findRenderableShaderRefs(doc, elements, includeReferencedGraphs, processedOutputs);

    // Find node graph outputs. Skip any light shaders
    for (NodeGraphPtr nodeGraph : doc->getNodeGraphs())
    {
        // Skip anything from an include file including libraries.
        // Skip any nodegraph which is a definition
        if (!nodeGraph->hasSourceUri() && !nodeGraph->hasAttribute(InterfaceElement::NODE_DEF_ATTRIBUTE))
        {
            for (OutputPtr output : nodeGraph->getOutputs())
            {
                if (processedOutputs.count(output))
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
                        elements.push_back(output);
                    }
                }
                processedOutputs.insert(output);
            }
        }
    }

    // Add in all top-level outputs not already processed.
    for (OutputPtr output : doc->getOutputs())
    {
        if (!output->hasSourceUri() && !processedOutputs.count(output))
        {
            elements.push_back(output);
        }
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
        udimCoordinates.push_back(Vector2(static_cast<float>(uVal), static_cast<float>(vVal)));
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

} // namespace MaterialX
