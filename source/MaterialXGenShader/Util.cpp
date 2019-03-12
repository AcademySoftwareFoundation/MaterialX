//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Util.h>
#include <MaterialXFormat/XmlIo.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_set>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

namespace MaterialX
{

void makeDirectory(const string& directoryPath)
{
#if defined(_WIN32)
    _mkdir(directoryPath.c_str());
#else
    mkdir(directoryPath.c_str(), 0777);
#endif
}

string removeExtension(const string& filename)
{
    size_t lastDot = filename.find_last_of(".");
    if (lastDot == string::npos) return filename;
    return filename.substr(0, lastDot);
}

void getSubDirectories(const string& baseDirectory, StringVec& relativePaths)
{
    relativePaths.push_back(baseDirectory);

#if defined(_WIN32)
    WIN32_FIND_DATA fd;
    HANDLE hFind = ::FindFirstFile((baseDirectory + "\\*").c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            string filename = fd.cFileName;
            if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (filename != "." && filename != ".."))
            {
                string newBaseDirectory = baseDirectory + "\\" + filename;
                getSubDirectories(newBaseDirectory, relativePaths);
            }
        } while (::FindNextFile(hFind, &fd));
        ::FindClose(hFind);
    }
#else
    struct dirent *entry = nullptr;
    DIR* dir = opendir(baseDirectory.c_str());
    if (dir)
    {
        while ((entry = readdir(dir)))
        {
            string filename = entry->d_name;
            if (entry->d_type == DT_DIR && (filename != "." && filename != ".."))
            {
                string newBaseDirectory = baseDirectory + "/" + filename;
                getSubDirectories(newBaseDirectory, relativePaths);
            }
        }
        closedir(dir);
    }
#endif
}

void getFilesInDirectory(const string& directory, StringVec& files, const string& extension)
{
#if defined(_WIN32)
    WIN32_FIND_DATA fd;
    HANDLE hFind = ::FindFirstFile((directory + "/*." + extension).c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                files.push_back(fd.cFileName);
            }
        } while (::FindNextFile(hFind, &fd));
        ::FindClose(hFind);
    }
#else
    struct dirent *entry = nullptr;
    DIR* dir = opendir(directory.c_str());
    if (dir)
    {
        while ((entry = readdir(dir)))
        {
            if (entry->d_type != DT_DIR && getFileExtension(entry->d_name) == "mtlx")
            {
                files.push_back(entry->d_name);
            }
        }
        closedir(dir);
    }
#endif
}

bool readFile(const string& filename, string& contents)
{
    std::ifstream file(filename, std::ios::in);
    if (file)
    {
        std::stringstream stream;
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

string getFileExtension(const string& filename)
{
    size_t i = filename.rfind('.');
    return i != string::npos ? filename.substr(i + 1) : EMPTY_STRING;
}

void loadDocuments(const FilePath& rootPath, const StringSet& skipFiles, 
                   vector<DocumentPtr>& documents, StringVec& documentsPaths, std::ostream* validityLog)
{
    const string MTLX_EXTENSION("mtlx");

    StringVec dirs;
    string baseDirectory = rootPath;
    getSubDirectories(baseDirectory, dirs);

    bool testSkipFiles = !skipFiles.empty();

    for (auto dir : dirs)
    {
        StringVec files;
        getFilesInDirectory(dir, files, MTLX_EXTENSION);

        for (const string& file : files)
        {
            if (testSkipFiles && skipFiles.count(file) != 0)
            {
                continue;
            }

            const FilePath filePath = FilePath(dir) / FilePath(file);
            const string filename = filePath;

            DocumentPtr doc = createDocument();
            try
            {
                readFromXmlFile(doc, filename, dir);

                if (validityLog)
                {
                    string docErrors;
                    bool valid = doc->validate(&docErrors);
                    if (!valid)
                    {
                        *validityLog << filename << std::endl;
                        *validityLog << docErrors << std::endl;
                        throw ExceptionShaderGenError("");
                    }
                }

                documents.push_back(doc);
                documentsPaths.push_back(filePath.asString());
            }
            catch (Exception& /*e*/)
            {
                continue;
            }
        }
    }
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

    bool isTransparentShaderGraph(OutputPtr output, const ShaderGenerator& shadergen)
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
                    else if (opacity->getNodeName() == EMPTY_STRING && opacity->getInterfaceName() == EMPTY_STRING)
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
                    else if (transmission->getNodeName() == EMPTY_STRING && transmission->getInterfaceName() == EMPTY_STRING)
                    {
                        // Unconnected, check the value
                        ValuePtr value = transmission->getValue();
                        if (!value || (value->asA<float>() && isZero(value->asA<float>())))
                        {
                            opaque = true;
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
                        else if (opacity->getNodeName() == EMPTY_STRING && opacity->getInterfaceName() == EMPTY_STRING)
                        {
                            // Unconnected, check the value
                            ValuePtr value = opacity->getValue();
                            if (!value || (value->isA<Color3>() && isWhite(value->asA<Color3>())))
                            {
                                opaque = true;
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
                                    bool isTransparent = isTransparentShaderGraph(graphOutput, shadergen);
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
    if (element->isA<ShaderRef>())
    {
        ShaderRefPtr shaderRef = element->asA<ShaderRef>();
        NodeDefPtr nodeDef = shaderRef->getNodeDef();
        if (!nodeDef)
        {
            throw ExceptionShaderGenError("Could not find a nodedef for shaderref '" + shaderRef->getName() + "'");
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
                if (outputs.size() > 0)
                {
                    const OutputPtr& output = outputs[0];
                    if (TypeDesc::get(output->getType()) == Type::SURFACESHADER)
                    {
                        return isTransparentShaderGraph(output, shadergen);
                    }
                }
            }
        }
    }
    else if (element->isA<Output>())
    {
        OutputPtr output = element->asA<Output>();
        return isTransparentShaderGraph(output, shadergen);
    }

    return false;
}

void mapValueToColor(const ValuePtr value, Color4& color)
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

bool requiresImplementation(const NodeDefPtr nodeDef)
{
    if (!nodeDef)
    {
        return false;
    }
    static string TYPE_NONE("none");
    const string& typeAttribute = nodeDef->getType();
    return !typeAttribute.empty() && typeAttribute != TYPE_NONE;
}

bool elementRequiresShading(const TypedElementPtr element)
{
    string elementType(element->getType());
    static std::set<string> colorClosures =
    {
        "surfaceshader", "volumeshader", "lightshader",
        "BSDF", "EDF", "VDF"
    };
    return (element->isA<ShaderRef>() ||
            colorClosures.count(elementType) > 0);
}

void findRenderableElements(const DocumentPtr& doc, std::vector<TypedElementPtr>& elements, bool includeReferencedGraphs, 
                            std::ostream* errorLog)
{
    try
    {
        std::vector<NodeGraphPtr> nodeGraphs = doc->getNodeGraphs();
        std::vector<OutputPtr> outputList = doc->getOutputs();
        std::unordered_set<OutputPtr> outputSet(outputList.begin(), outputList.end());
        std::vector<MaterialPtr> materials = doc->getMaterials();

        if (!materials.empty() || !nodeGraphs.empty() || !outputList.empty())
        {
            std::unordered_set<OutputPtr> shaderrefOutputs;
            for (auto material : materials)
            {
                for (auto shaderRef : material->getShaderRefs())
                {
                    if (!shaderRef->hasSourceUri())
                    {
                        // Add in all shader references which are not part of a node definition library
                        NodeDefPtr nodeDef = shaderRef->getNodeDef();
                        if (!nodeDef)
                        {
                            throw ExceptionShaderGenError("Could not find a nodedef for shaderref '" + shaderRef->getName() + "'");
                        }
                        if (requiresImplementation(nodeDef))
                        {
                            elements.push_back(shaderRef);
                        }

                        // Find all bindinputs which reference outputs and outputgraphs
                        if (!includeReferencedGraphs)
                        {
                            for (auto bindInput : shaderRef->getBindInputs())
                            {
                                OutputPtr outputPtr = bindInput->getConnectedOutput();
                                if (outputPtr)
                                {
                                    shaderrefOutputs.insert(outputPtr);
                                }
                            }
                        }
                    }
                }
            }

            // Find node graph outputs. Skip any light shaders
            const string LIGHT_SHADER("lightshader");
            for (NodeGraphPtr nodeGraph : nodeGraphs)
            {
                // Skip anything from an include file including libraries.
                if (!nodeGraph->hasSourceUri())
                {
                    std::vector<OutputPtr> nodeGraphOutputs = nodeGraph->getOutputs();
                    for (OutputPtr output : nodeGraphOutputs)
                    {
                        NodePtr outputNode = output->getConnectedNode();

                        // For now we skip any outputs which are referenced elsewhere.
                        if (outputNode &&
                            outputNode->getType() != LIGHT_SHADER &&
                            (!includeReferencedGraphs && shaderrefOutputs.count(output) == 0))
                        {
                            NodeDefPtr nodeDef = outputNode->getNodeDef();
                            if (!nodeDef)
                            {
                                throw ExceptionShaderGenError("Could not find a nodedef for output '" + outputNode->getName() + "'");
                            }
                            if (requiresImplementation(nodeDef))
                            {
                                outputSet.insert(output);
                            }
                        }
                    }
                }
            }

            // Add in all outputs which are not part of a library
            for (OutputPtr output : outputSet)
            {
                if (!output->hasSourceUri())
                {
                    elements.push_back(output);
                }
            }
        }
    }
    catch (ExceptionShaderGenError& e)
    {
        if (errorLog)
        {
            *errorLog << e.what() << std::endl;
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
    ValueElementPtr valueElement = nodeDef->getChildOfType<ValueElement>(valueElementName);

    return valueElement;
}

unsigned int getUIProperties(const ValueElementPtr nodeDefElement, UIProperties& uiProperties)
{
    if (!nodeDefElement)
    {
        return 0;
    }

    unsigned int propertyCount = 0;
    uiProperties.uiName = nodeDefElement->getAttribute(ValueElement::UI_NAME_ATTRIBUTE);
    if (!uiProperties.uiName.empty())
        propertyCount++;

    uiProperties.uiFolder = nodeDefElement->getAttribute(ValueElement::UI_FOLDER_ATTRIBUTE);
    if (!uiProperties.uiFolder.empty())
        propertyCount++;

    if (nodeDefElement->isA<Parameter>())
    {
        string enumString = nodeDefElement->getAttribute(ValueElement::ENUM_ATTRIBUTE);
        if (!enumString.empty())
        {
            uiProperties.enumeration = splitString(enumString, ",");
            if (uiProperties.enumeration.size())
                propertyCount++;
        }

        const string& enumerationValues = nodeDefElement->getAttribute(ValueElement::ENUM_VALUES_ATTRIBUTE);
        if (!enumerationValues.empty())
        {
            const string& elemType = nodeDefElement->getType();
            const TypeDesc* typeDesc = TypeDesc::get(elemType);
            if (typeDesc->isScalar() || typeDesc->isFloat2() || typeDesc->isFloat3() || 
                typeDesc->isFloat4())
            {
                StringVec stringValues = splitString(enumerationValues, ",");
                string valueString;
                size_t elementCount = typeDesc->getSize();
                elementCount--;
                size_t count = 0;
                for (size_t i = 0; i < stringValues.size(); i++)
                {
                    if (count == elementCount)
                    { 
                        valueString += stringValues[i];
                        uiProperties.enumerationValues.push_back(Value::createValueFromStrings(valueString, elemType));
                        valueString.clear();
                        count = 0;
                    }
                    else
                    {
                        valueString += stringValues[i] + ",";
                        count++;
                    }
                }
            }
            else
            {
                uiProperties.enumerationValues.push_back(Value::createValue(enumerationValues));
            }
            propertyCount++;
        }
    }

    const string& uiMinString = nodeDefElement->getAttribute(ValueElement::UI_MIN_ATTRIBUTE);
    if (!uiMinString.empty())
    {
        ValuePtr value = Value::createValueFromStrings(uiMinString, nodeDefElement->getType());
        if (value)
        {
            uiProperties.uiMin = value;
            propertyCount++;
        }
    }

    const string& uiMaxString = nodeDefElement->getAttribute(ValueElement::UI_MAX_ATTRIBUTE);
    if (!uiMaxString.empty())
    {
        ValuePtr value = Value::createValueFromStrings(uiMaxString, nodeDefElement->getType());
        if (value)
        {
            uiProperties.uiMax = value;
            propertyCount++;
        }
    }
    return propertyCount;
}

unsigned int getUIProperties(const string& path, DocumentPtr doc, const string& target, UIProperties& uiProperties)
{
    ValueElementPtr valueElement = findNodeDefChild(path, doc, target);
    if (valueElement)
    {
        return getUIProperties(valueElement, uiProperties);
    }
    return 0;
}

} // namespace MaterialX
