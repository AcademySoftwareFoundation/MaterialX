//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Document.h>

#include <MaterialXCore/Util.h>

#include <mutex>

namespace MaterialX
{

const string Document::CMS_ATTRIBUTE = "cms";
const string Document::CMS_CONFIG_ATTRIBUTE = "cmsconfig";

namespace {

template<class T> shared_ptr<T> updateChildSubclass(ElementPtr parent, ElementPtr origChild)
{
    string childName = origChild->getName();
    int childIndex = parent->getChildIndex(childName);
    parent->removeChild(childName);
    shared_ptr<T> newChild = parent->addChild<T>(childName);
    parent->setChildIndex(childName, childIndex);
    newChild->copyContentFrom(origChild);
    return newChild;
}

NodeDefPtr getShaderNodeDef(ElementPtr shaderRef)
{
    if (shaderRef->hasAttribute(NodeDef::NODE_DEF_ATTRIBUTE))
    {
        string nodeDefString = shaderRef->getAttribute(NodeDef::NODE_DEF_ATTRIBUTE);
        return shaderRef->resolveRootNameReference<NodeDef>(nodeDefString);
    }
    if (shaderRef->hasAttribute(NodeDef::NODE_ATTRIBUTE))
    {
        string nodeString = shaderRef->getAttribute(NodeDef::NODE_ATTRIBUTE);
        string type = shaderRef->getAttribute(TypedElement::TYPE_ATTRIBUTE);
        vector<NodeDefPtr> nodeDefs = shaderRef->getDocument()->getMatchingNodeDefs(shaderRef->getQualifiedName(nodeString));
        vector<NodeDefPtr> secondary = shaderRef->getDocument()->getMatchingNodeDefs(nodeString);
        nodeDefs.insert(nodeDefs.end(), secondary.begin(), secondary.end());
        for (NodeDefPtr nodeDef : nodeDefs)
        {
            if (targetStringsMatch(nodeDef->getTarget(), shaderRef->getTarget()) &&
                nodeDef->isVersionCompatible(shaderRef) &&
                (type.empty() || nodeDef->getType() == type))
            {
                return nodeDef;
            }
        }
    }
    return NodeDefPtr();
}

} // anonymous namespace

//
// Document factory function
//

DocumentPtr createDocument()
{
    return Document::createDocument<Document>();
}

//
// Document cache
//

class Document::Cache
{
  public:
    Cache() :
        valid(false)
    {
    }
    ~Cache() { }

    void refresh()
    {
        // Thread synchronization for multiple concurrent readers of a single document.
        std::lock_guard<std::mutex> guard(mutex);

        if (!valid)
        {
            // Clear the existing cache.
            portElementMap.clear();
            nodeDefMap.clear();
            implementationMap.clear();

            // Traverse the document to build a new cache.
            for (ElementPtr elem : doc.lock()->traverseTree())
            {
                const string& nodeName = elem->getAttribute(PortElement::NODE_NAME_ATTRIBUTE);
                const string& nodeString = elem->getAttribute(NodeDef::NODE_ATTRIBUTE);
                const string& nodeDefString = elem->getAttribute(InterfaceElement::NODE_DEF_ATTRIBUTE);

                if (!nodeName.empty())
                {
                    PortElementPtr portElem = elem->asA<PortElement>();
                    if (portElem)
                    {
                        portElementMap.emplace(portElem->getQualifiedName(nodeName), portElem);
                    }
                }
                if (!nodeString.empty())
                {
                    NodeDefPtr nodeDef = elem->asA<NodeDef>();
                    if (nodeDef)
                    {
                        nodeDefMap.emplace(nodeDef->getQualifiedName(nodeString), nodeDef);
                    }
                }
                if (!nodeDefString.empty())
                {
                    InterfaceElementPtr interface = elem->asA<InterfaceElement>();
                    if (interface && (interface->isA<Implementation>() || interface->isA<NodeGraph>()))
                    {
                        implementationMap.emplace(interface->getQualifiedName(nodeDefString), interface);
                    }
                }
            }

            valid = true;
        }
    }

  public:
    weak_ptr<Document> doc;
    std::mutex mutex;
    bool valid;
    std::unordered_multimap<string, PortElementPtr> portElementMap;
    std::unordered_multimap<string, NodeDefPtr> nodeDefMap;
    std::unordered_multimap<string, InterfaceElementPtr> implementationMap;
};

//
// Document methods
//

Document::Document(ElementPtr parent, const string& name) :
    GraphElement(parent, CATEGORY, name),
    _cache(std::unique_ptr<Cache>(new Cache))
{
}

Document::~Document()
{
}

void Document::initialize()
{
    _root = getSelf();
    _cache->doc = getDocument();

    clearContent();
    setVersionIntegers(MATERIALX_MAJOR_VERSION, MATERIALX_MINOR_VERSION);
}

NodeDefPtr Document::addNodeDefFromGraph(const NodeGraphPtr nodeGraph, const string& nodeDefName, const string& node,
                                         const string& version, bool isDefaultVersion, const string& group, string& newGraphName)
{
    if (getNodeDef(nodeDefName))
    {
        throw Exception("Cannot create duplicate nodedef: " + nodeDefName);
    }

    NodeGraphPtr graph = nodeGraph;
    if (!newGraphName.empty())
    {
        if (getNodeGraph(newGraphName))
        {
            throw Exception("Cannot create duplicate nodegraph: " + newGraphName);
        }
        graph = addNodeGraph(newGraphName);
        graph->copyContentFrom(nodeGraph);
    }
    graph->setNodeDefString(nodeDefName);

    NodeDefPtr nodeDef = addChild<NodeDef>(nodeDefName);
    nodeDef->setNodeString(node);
    if (!group.empty())
    {
        nodeDef->setNodeGroup(group);
    }

    if (!version.empty())
    {
        nodeDef->setVersionString(version);

        // Can only be a default version if there is a version string
        if (isDefaultVersion)
        {
            nodeDef->setDefaultVersion(true);
        }
    }

    for (auto output : graph->getOutputs())
    {
        nodeDef->addOutput(output->getName(), output->getType());
    }

    return nodeDef;
}

void Document::importLibrary(const ConstDocumentPtr& library)
{
    if (!library)
    {
        return;
    }

    for (auto child : library->getChildren())
    {
        if (child->getCategory().empty())
        {
            throw Exception("Trying to import child without a category: " + child->getName());
        }

        const string childName = child->getQualifiedName(child->getName());

        // Check for duplicate elements.
        ConstElementPtr previous = getChild(childName);
        if (previous)
        {
            continue;
        }

        // Create the imported element.
        ElementPtr childCopy = addChildOfCategory(child->getCategory(), childName);
        childCopy->copyContentFrom(child);
        if (!childCopy->hasFilePrefix() && library->hasFilePrefix())
        {
            childCopy->setFilePrefix(library->getFilePrefix());
        }
        if (!childCopy->hasGeomPrefix() && library->hasGeomPrefix())
        {
            childCopy->setGeomPrefix(library->getGeomPrefix());
        }
        if (!childCopy->hasColorSpace() && library->hasColorSpace())
        {
            childCopy->setColorSpace(library->getColorSpace());
        }
        if (!childCopy->hasNamespace() && library->hasNamespace())
        {
            childCopy->setNamespace(library->getNamespace());
        }
        if (!childCopy->hasSourceUri() && library->hasSourceUri())
        {
            childCopy->setSourceUri(library->getSourceUri());
        }
    }
}

StringSet Document::getReferencedSourceUris() const
{
    StringSet sourceUris;
    for (ElementPtr elem : traverseTree())
    {
        if (elem->hasSourceUri())
        {
            sourceUris.insert(elem->getSourceUri());
        }
    }
    return sourceUris;
}

std::pair<int, int> Document::getVersionIntegers() const
{
    if (!hasVersionString())
    {
        return {MATERIALX_MAJOR_VERSION, MATERIALX_MINOR_VERSION};
    }
    return Element::getVersionIntegers();
}

vector<PortElementPtr> Document::getMatchingPorts(const string& nodeName) const
{
    // Refresh the cache.
    _cache->refresh();

    // Find all port elements matching the given node name.
    vector<PortElementPtr> ports;
    auto keyRange = _cache->portElementMap.equal_range(nodeName);
    for (auto it = keyRange.first; it != keyRange.second; ++it)
    {
        ports.push_back(it->second);
    }

    // Return the matches.
    return ports;
}

ValuePtr Document::getGeomPropValue(const string& geomPropName, const string& geom) const
{
    ValuePtr value;
    for (GeomInfoPtr geomInfo : getGeomInfos())
    {
        if (!geomStringsMatch(geom, geomInfo->getActiveGeom()))
        {
            continue;
        }
        GeomPropPtr geomProp = geomInfo->getGeomProp(geomPropName);
        if (geomProp)
        {
            value = geomProp->getValue();
        }
    }
    return value;
}

vector<NodeDefPtr> Document::getMatchingNodeDefs(const string& nodeName) const
{
    // Refresh the cache.
    _cache->refresh();

    // Find all nodedefs matching the given node name.
    vector<NodeDefPtr> nodeDefs;
    auto keyRange = _cache->nodeDefMap.equal_range(nodeName);
    for (auto it = keyRange.first; it != keyRange.second; ++it)
    {
        nodeDefs.push_back(it->second);
    }

    // Return the matches.
    return nodeDefs;
}

vector<InterfaceElementPtr> Document::getMatchingImplementations(const string& nodeDef) const
{
    // Refresh the cache.
    _cache->refresh();

    // Find all implementations matching the given nodedef string.
    vector<InterfaceElementPtr> implementations;
    auto keyRange = _cache->implementationMap.equal_range(nodeDef);
    for (auto it = keyRange.first; it != keyRange.second; ++it)
    {
        implementations.push_back(it->second);
    }

    // Return the matches.
    return implementations;
}

bool Document::validate(string* message) const
{
    bool res = true;
    validateRequire(hasVersionString(), res, message, "Missing version string");
    return GraphElement::validate(message) && res;
}

void Document::upgradeVersion()
{
    std::pair<int, int> versions = getVersionIntegers();
    int majorVersion = versions.first;
    int minorVersion = versions.second;
    if (majorVersion == MATERIALX_MAJOR_VERSION &&
        minorVersion == MATERIALX_MINOR_VERSION)
    {
        return;
    }

    // Upgrade from v1.22 to v1.23
    if (majorVersion == 1 && minorVersion == 22)
    {
        for (ElementPtr elem : traverseTree())
        {
            if (elem->getAttribute(TypedElement::TYPE_ATTRIBUTE) == "vector")
            {
                elem->setAttribute(TypedElement::TYPE_ATTRIBUTE, getTypeString<Vector3>());
            }
        }
        minorVersion = 23;
    }

    // Upgrade from v1.23 to v1.24
    if (majorVersion == 1 && minorVersion == 23)
    {
        for (ElementPtr elem : traverseTree())
        {
            if (elem->getCategory() == "shader" && elem->hasAttribute("shadername"))
            {
                elem->setAttribute(NodeDef::NODE_ATTRIBUTE, elem->getAttribute("shadername"));
                elem->removeAttribute("shadername");
            }
            for (ElementPtr child : getChildrenOfType<Element>("assign"))
            {
                updateChildSubclass<MaterialAssign>(elem, child);
            }
        }
        minorVersion = 24;
    }

    // Upgrade from v1.24 to v1.25
    if (majorVersion == 1 && minorVersion == 24)
    {
        for (ElementPtr elem : traverseTree())
        {
            if (elem->isA<Input>() && elem->hasAttribute("graphname"))
            {
                elem->setAttribute("opgraph", elem->getAttribute("graphname"));
                elem->removeAttribute("graphname");
            }
        }
        minorVersion = 25;
    }

    // Upgrade from v1.25 to v1.26
    if (majorVersion == 1 && minorVersion == 25)
    {
        for (ElementPtr elem : traverseTree())
        {
            if (elem->getCategory() == "constant")
            {
                ElementPtr param = elem->getChild("color");
                if (param)
                {
                    param->setName("value");
                }
            }
        }
        minorVersion = 26;
    }

    // Upgrade from v1.26 to v1.34
    if (majorVersion == 1 && minorVersion == 26)
    {
        // Upgrade elements in place.
        for (ElementPtr elem : traverseTree())
        {
            vector<ElementPtr> origChildren = elem->getChildren();
            for (ElementPtr child : origChildren)
            {
                if (child->getCategory() == "opgraph")
                {
                    updateChildSubclass<NodeGraph>(elem, child);
                }
                else if (child->getCategory() == "shader")
                {
                    NodeDefPtr nodeDef = updateChildSubclass<NodeDef>(elem, child);
                    if (nodeDef->hasAttribute("shadertype"))
                    {
                        nodeDef->setType(SURFACE_SHADER_TYPE_STRING);
                        nodeDef->removeAttribute("shadertype");
                    }
                    if (nodeDef->hasAttribute("shaderprogram"))
                    {
                        nodeDef->setNodeString(nodeDef->getAttribute("shaderprogram"));
                        nodeDef->removeAttribute("shaderprogram");
                    }
                }
                else if (child->getCategory() == "shaderref")
                {
                    if (child->hasAttribute("shadertype"))
                    {
                        child->setAttribute(TypedElement::TYPE_ATTRIBUTE, SURFACE_SHADER_TYPE_STRING);
                        child->removeAttribute("shadertype");
                    }
                }
                else if (child->getCategory() == "parameter")
                {
                    if (child->getAttribute(TypedElement::TYPE_ATTRIBUTE) == "opgraphnode")
                    {
                        if (elem->isA<Node>())
                        {
                            InputPtr input = updateChildSubclass<Input>(elem, child);
                            input->setNodeName(input->getAttribute("value"));
                            input->removeAttribute("value");
                            if (input->getConnectedNode())
                            {
                                input->setType(input->getConnectedNode()->getType());
                            }
                            else
                            {
                                input->setType(getTypeString<Color3>());
                            }
                        }
                        else if (elem->isA<Output>())
                        {
                            if (child->getName() == "in")
                            {
                                elem->setAttribute("nodename", child->getAttribute("value"));
                            }
                            elem->removeChild(child->getName());
                        }
                    }
               }
            }
        }

        // Assign nodedef names to shaderrefs.
        for (ElementPtr mat : getChildrenOfType<Element>("material"))
        {
            for (ElementPtr shaderRef : mat->getChildrenOfType<Element>("shaderref"))
            {
                if (!getShaderNodeDef(shaderRef))
                {
                    NodeDefPtr nodeDef = getNodeDef(shaderRef->getName());
                    if (nodeDef)
                    {
                        shaderRef->setAttribute(NodeDef::NODE_DEF_ATTRIBUTE, nodeDef->getName());
                        shaderRef->setAttribute(NodeDef::NODE_ATTRIBUTE, nodeDef->getNodeString());
                    }
                }
            }
        }

        // Move connections from nodedef inputs to bindinputs.
        vector<ElementPtr> materials = getChildrenOfType<Element>("material");
        for (NodeDefPtr nodeDef : getNodeDefs())
        {
            for (InputPtr input : nodeDef->getActiveInputs())
            {
                if (input->hasAttribute("opgraph") && input->hasAttribute("graphoutput"))
                {
                    for (ElementPtr mat : materials)
                    {
                        for (ElementPtr shaderRef : mat->getChildrenOfType<Element>("shaderref"))
                        {
                            if (getShaderNodeDef(shaderRef) == nodeDef && !shaderRef->getChild(input->getName()))
                            {
                                ElementPtr bindInput = shaderRef->addChildOfCategory("bindinput", input->getName());
                                bindInput->setAttribute(TypedElement::TYPE_ATTRIBUTE, input->getType());
                                bindInput->setAttribute("nodegraph", input->getAttribute("opgraph"));
                                bindInput->setAttribute("output", input->getAttribute("graphoutput"));
                            }
                        }
                    }
                    input->removeAttribute("opgraph");
                    input->removeAttribute("graphoutput");
                }
            }
        }

        // Combine udim assignments into udim sets.
        for (GeomInfoPtr geomInfo : getGeomInfos())
        {
            for (ElementPtr child : geomInfo->getChildrenOfType<Element>("geomattr"))
            {
                updateChildSubclass<GeomProp>(geomInfo, child);
            }
        }
        if (getGeomPropValue("udim") && !getGeomPropValue("udimset"))
        {
            StringSet udimSet;
            for (GeomInfoPtr geomInfo : getGeomInfos())
            {
                for (GeomPropPtr geomProp : geomInfo->getGeomProps())
                {
                    if (geomProp->getName() == "udim")
                    {
                        udimSet.insert(geomProp->getValueString());
                    }
                }
            }

            std::string udimSetString;
            for (const std::string& udim : udimSet)
            {
                if (udimSetString.empty())
                {
                    udimSetString = udim;
                }
                else
                {
                    udimSetString += ", " + udim;
                }
            }

            GeomInfoPtr udimSetInfo = addGeomInfo();
            udimSetInfo->setGeomPropValue("udimset", udimSetString, getTypeString<StringVec>());
        }

        minorVersion = 34;
    }

    // Upgrade from v1.34 to v1.35
    if (majorVersion == 1 && minorVersion == 34)
    {
        for (ElementPtr elem : traverseTree())
        {
            if (elem->getAttribute(TypedElement::TYPE_ATTRIBUTE) == "matrix")
            {
                elem->setAttribute(TypedElement::TYPE_ATTRIBUTE, getTypeString<Matrix44>());
            }
            if (elem->hasAttribute("default") && !elem->hasAttribute(ValueElement::VALUE_ATTRIBUTE))
            {
                elem->setAttribute(ValueElement::VALUE_ATTRIBUTE, elem->getAttribute("default"));
                elem->removeAttribute("default");
            }

            MaterialAssignPtr matAssign = elem->asA<MaterialAssign>();
            if (matAssign)
            {
                matAssign->setMaterial(matAssign->getName());
            }
        }
        minorVersion = 35;
    }

    // Upgrade from v1.35 to v1.36
    if (majorVersion == 1 && minorVersion == 35)
    {
        for (ElementPtr elem : traverseTree())
        {
            LookPtr look = elem->asA<Look>();
            GeomInfoPtr geomInfo = elem->asA<GeomInfo>();

            if (elem->getAttribute(TypedElement::TYPE_ATTRIBUTE) == GEOMNAME_TYPE_STRING &&
                elem->getAttribute(ValueElement::VALUE_ATTRIBUTE) == "*")
            {
                elem->setAttribute(ValueElement::VALUE_ATTRIBUTE, UNIVERSAL_GEOM_NAME);
            }
            if (elem->getAttribute(TypedElement::TYPE_ATTRIBUTE) == FILENAME_TYPE_STRING)
            {
                StringMap stringMap;
                stringMap["%UDIM"] = UDIM_TOKEN;
                stringMap["%UVTILE"] = UV_TILE_TOKEN;
                elem->setAttribute(ValueElement::VALUE_ATTRIBUTE, replaceSubstrings(elem->getAttribute(ValueElement::VALUE_ATTRIBUTE), stringMap));
            }

            vector<ElementPtr> origChildren = elem->getChildren();
            for (ElementPtr child : origChildren)
            {
                if (elem->getCategory() == "material" && child->getCategory() == "override")
                {
                    for (ElementPtr shaderRef : elem->getChildrenOfType<Element>("shaderref"))
                    {
                        NodeDefPtr nodeDef = getShaderNodeDef(shaderRef);
                        if (nodeDef)
                        {
                            for (ValueElementPtr activeValue : nodeDef->getActiveValueElements())
                            {
                                if (activeValue->getAttribute("publicname") == child->getName() &&
                                    !shaderRef->getChild(child->getName()))
                                {
                                    if (activeValue->getCategory() == "parameter")
                                    {
                                        ElementPtr bindParam = shaderRef->addChildOfCategory("bindparam", activeValue->getName());
                                        bindParam->setAttribute(TypedElement::TYPE_ATTRIBUTE, activeValue->getType());
                                        bindParam->setAttribute(ValueElement::VALUE_ATTRIBUTE, child->getAttribute("value"));
                                    }
                                    else if (activeValue->isA<Input>())
                                    {
                                        ElementPtr bindInput = shaderRef->addChildOfCategory("bindinput", activeValue->getName());
                                        bindInput->setAttribute(TypedElement::TYPE_ATTRIBUTE, activeValue->getType());
                                        bindInput->setAttribute(ValueElement::VALUE_ATTRIBUTE, child->getAttribute("value"));
                                    }
                                }
                            }
                        }
                    }
                    elem->removeChild(child->getName());
                }
                else if (elem->getCategory() == "material" && child->getCategory() == "materialinherit")
                {
                    elem->setInheritString(child->getAttribute("material"));
                    elem->removeChild(child->getName());
                }
                else if (look && child->getCategory() == "lookinherit")
                {
                    elem->setInheritString(child->getAttribute("look"));
                    elem->removeChild(child->getName());
                }
            }
        }
        minorVersion = 36;
    }

    // Upgrade from 1.36 to 1.37
    if (majorVersion == 1 && minorVersion == 36)
    {
        // Convert type attributes to child outputs.
        for (NodeDefPtr nodeDef : getNodeDefs())
        {
            InterfaceElementPtr interfaceElem = std::static_pointer_cast<InterfaceElement>(nodeDef);
            if (interfaceElem && interfaceElem->hasType())
            {
                string type = interfaceElem->getAttribute(TypedElement::TYPE_ATTRIBUTE);
                OutputPtr outputPtr;
                if (!type.empty() && type != MULTI_OUTPUT_TYPE_STRING)
                {
                    outputPtr = interfaceElem->getOutput("out");
                    if (!outputPtr)
                    {
                        outputPtr = interfaceElem->addOutput("out", type);
                    }
                }
                interfaceElem->removeAttribute(TypedElement::TYPE_ATTRIBUTE);

                const string& defaultInput = interfaceElem->getAttribute(Output::DEFAULT_INPUT_ATTRIBUTE);
                if (outputPtr && !defaultInput.empty())
                {
                    outputPtr->setAttribute(Output::DEFAULT_INPUT_ATTRIBUTE, defaultInput);
                }
                interfaceElem->removeAttribute(Output::DEFAULT_INPUT_ATTRIBUTE);
            }
        }

        // Convert geometric attributes to geometric properties.
        for (GeomInfoPtr geomInfo : getGeomInfos())
        {
            for (ElementPtr child : geomInfo->getChildrenOfType<Element>("geomattr"))
            {
                updateChildSubclass<GeomProp>(geomInfo, child);
            }
        }
        for (ElementPtr elem : traverseTree())
        {
            NodePtr node = elem->asA<Node>();
            if (!node)
            {
                continue;
            }

            if (node->getCategory() == "geomattrvalue")
            {
                node->setCategory("geompropvalue");
                if (node->hasAttribute("attrname"))
                {
                    node->setAttribute("geomprop", node->getAttribute("attrname"));
                    node->removeAttribute("attrname");
                }
            }
        }

        for (ElementPtr elem : traverseTree())
        {
            NodePtr node = elem->asA<Node>();
            if (!node)
            {
                continue;
            }
            const string& nodeCategory = node->getCategory();

            // Change category from "invert to "invertmatrix" for matrix invert nodes
            if (nodeCategory == "invert" &&
                (node->getType() == getTypeString<Matrix33>() || node->getType() == getTypeString<Matrix44>()))
            {
                node->setCategory("invertmatrix");
            }

            // Change category from "rotate" to "rotate2d" or "rotate3d" nodes
            else if (nodeCategory == "rotate")
            {
                node->setCategory((node->getType() == getTypeString<Vector2>()) ? "rotate2d" : "rotate3d");
            }

            // Convert "compare" node to "ifgreatereq".
            else if (nodeCategory == "compare")
            {
                node->setCategory("ifgreatereq");
                InputPtr intest = node->getInput("intest");
                if (intest)
                {
                    intest->setName("value1");
                }
                ElementPtr cutoff = node->getChild("cutoff");
                if (cutoff)
                {
                    cutoff = updateChildSubclass<Input>(node, cutoff);
                    cutoff->setName("value2");
                }
                InputPtr in1 = node->getInput("in1");
                InputPtr in2 = node->getInput("in2");
                if (in1 && in2)
                {
                    in1->setName(createValidChildName("temp"));
                    in2->setName("in1");
                    in1->setName("in2");
                }
            }

            // Change nodes with category "tranform[vector|point|normal]",
            // which are not fromspace/tospace variants, to "transformmatrix"
            else if (nodeCategory == "transformpoint" ||
                     nodeCategory == "transformvector" ||
                     nodeCategory == "transformnormal")
            {
                if (!node->getChild("fromspace") && !node->getChild("tospace"))
                {
                    node->setCategory("transformmatrix");
                }
            }

            // Convert "combine" to "combine2", "combine3" or "combine4"
            else if (nodeCategory == "combine")
            {
                if (node->getChild("in4"))
                {
                    node->setCategory("combine4");
                }
                else if (node->getChild("in3"))
                {
                    node->setCategory("combine3");
                }
                else
                {
                    node->setCategory("combine2");
                }
            }

            // Convert "separate" to "separate2", "separate3" or "separate4"
            else if (nodeCategory == "separate")
            {
                InputPtr in = node->getInput("in");
                if (in)
                {
                    const string& inType = in->getType();
                    if (inType == getTypeString<Vector4>() || inType == getTypeString<Color4>())
                    {
                        node->setCategory("separate4");
                    }
                    else if (inType == getTypeString<Vector3>() || inType == getTypeString<Color3>())
                    {
                        node->setCategory("separate3");
                    }
                    else
                    {
                        node->setCategory("separate2");
                    }
                }
            }

            // Convert backdrop nodes to backdrop elements
            else if (nodeCategory == "backdrop")
            {
                BackdropPtr backdrop = addBackdrop(node->getName());
                for (ElementPtr child : node->getChildrenOfType<Element>("parameter"))
                {
                    if (child->hasAttribute(ValueElement::VALUE_ATTRIBUTE))
                    {
                        backdrop->setAttribute(child->getName(), child->getAttribute(ValueElement::VALUE_ATTRIBUTE));
                    }
                }
                removeNode(node->getName());
            }
        }

        // Remove deprecated nodedefs
        removeNodeDef("ND_backdrop");
        removeNodeDef("ND_invert_matrix33");
        removeNodeDef("ND_invert_matrix44");
        removeNodeDef("ND_rotate_vector2");
        removeNodeDef("ND_rotate_vector3");

        minorVersion = 37;
    }

    // Upgrade from 1.37 to 1.38
    if (majorVersion == 1 && minorVersion >= 37)
    {
        // Convert color2 types to vector2
        const StringMap COLOR2_CHANNEL_MAP = { { "r", "x" }, { "a", "y" } };
        for (ElementPtr elem : traverseTree())
        {
            if (elem->getAttribute(TypedElement::TYPE_ATTRIBUTE) == "color2")
            {
                elem->setAttribute(TypedElement::TYPE_ATTRIBUTE, getTypeString<Vector2>());
                NodePtr parentNode = elem->getParent()->asA<Node>();
                if (!parentNode)
                {
                    continue;
                }

                for (PortElementPtr port : parentNode->getDownstreamPorts())
                {
                    if (port->hasChannels())
                    {
                        string channels = port->getChannels();
                        channels = replaceSubstrings(channels, COLOR2_CHANNEL_MAP);
                        port->setChannels(channels);
                    }
                    if (port->hasOutputString())
                    {
                        string output = port->getOutputString();
                        output = replaceSubstrings(output, COLOR2_CHANNEL_MAP);
                        port->setOutputString(output);
                    }
                }

                ElementPtr channels = parentNode->getChild("channels");
                if (channels && channels->hasAttribute(ValueElement::VALUE_ATTRIBUTE))
                {
                    string value = channels->getAttribute(ValueElement::VALUE_ATTRIBUTE);
                    value = replaceSubstrings(value, COLOR2_CHANNEL_MAP);
                    channels->setAttribute(ValueElement::VALUE_ATTRIBUTE, value);
                }
            }
        }

        // Convert material elements to nodes
        for (ElementPtr mat : getChildrenOfType<Element>("material"))
        {
            string materialName = mat->getName();
            NodePtr materialNode = nullptr;

            for (ElementPtr shaderRef : mat->getChildrenOfType<Element>("shaderref"))
            {
                // Find the shader type if defined
                string shaderNodeType = SURFACE_SHADER_TYPE_STRING;
                NodeDefPtr nodeDef = getShaderNodeDef(shaderRef);
                if (nodeDef)
                {
                    shaderNodeType = nodeDef->getType();
                }

                // Add the shader node.
                string shaderNodeName = createValidChildName(shaderRef->getName());
                string shaderNodeCategory = shaderRef->getAttribute("node");
                NodePtr shaderNode = addNode(shaderNodeCategory, shaderNodeName, shaderNodeType);
                shaderNode->setSourceUri(shaderRef->getSourceUri());

                for (ElementPtr child : shaderRef->getChildren())
                {
                    ElementPtr port = nullptr;

                    // Copy over bindinputs as inputs, and bindparams as params
                    if (child->getCategory() == "bindinput")
                    {
                        port = shaderNode->addInput(child->getName(), child->getAttribute(TypedElement::TYPE_ATTRIBUTE));
                    }
                    else if (child->getCategory() == "bindparam")
                    {
                        port = shaderNode->addChildOfCategory("parameter", child->getName());
                        port->setAttribute(TypedElement::TYPE_ATTRIBUTE, child->getAttribute(TypedElement::TYPE_ATTRIBUTE));
                    }
                    else if (child->getCategory() == "bindtoken")
                    {
                        TokenPtr token = shaderNode->addToken(child->getName());
                        token->copyContentFrom(child);
                    }
                    if (port)
                    {
                        // Copy over attributes.
                        // Note: We preserve inputs which have nodegraph connections,
                        // as well as top level output connections.
                        port->copyContentFrom(child);
                    }
                }

                // Create a new material node if not already created and
                // add a reference from the material node to the new shader node
                if (!materialNode)
                {
                    materialNode = addMaterialNode(createValidName("temp"), shaderNode);
                    materialNode->setSourceUri(mat->getSourceUri());
                }

                // Create input to replace each shaderref. Use shaderref name as unique
                // input name.
                InputPtr shaderInput = materialNode->getInput(shaderNodeType);
                if (!shaderInput)
                {
                    shaderInput = materialNode->addInput(shaderNodeType, shaderNodeType);
                    shaderInput->setNodeName(shaderNode->getName());
                }
                if (!shaderRef->getTarget().empty())
                {
                    shaderInput->setTarget(shaderRef->getTarget());
                }
                if (!shaderRef->getVersionString().empty())
                {
                    shaderInput->setVersionString(shaderRef->getVersionString());
                }
            }

            // Remove the material element and transfer its name to the material node.
            removeChild(materialName);
            if (materialNode)
            {
                materialNode->setName(materialName);
            }
        }

        // Update atan2 interface and rotate3d interface
        const string ATAN2 = "atan2";
        const string IN1 = "in1";
        const string IN2 = "in2";
        const string ROTATE3D = "rotate3d";
        const string AXIS = "axis";
        const string INPUT_ONE = "1.0";

        // Update nodedefs
        bool upgradeAtan2Instances = false;
        for (auto nodedef : getMatchingNodeDefs(ATAN2))
        {
            InputPtr input = nodedef->getInput(IN1);
            InputPtr input2 = nodedef->getInput(IN2);
            string inputValue = input->getValueString();
            // Only flip value if nodedef value is the previous versions.
            if (inputValue == INPUT_ONE)
            {
                input->setValueString(input2->getValueString());
                input2->setValueString(inputValue);
                upgradeAtan2Instances = true;
            }
        }
        for (auto nodedef : getMatchingNodeDefs(ROTATE3D))
        {
            ElementPtr axis = nodedef->getChild(AXIS);
            if (axis)
            {
                updateChildSubclass<Input>(nodedef, axis);
            }
        }

        // Update nodes
        for (ElementPtr elem : traverseTree())
        {
            NodePtr node = elem->asA<Node>();
            if (!node)
            {
                continue;
            }
            const string& nodeCategory = node->getCategory();
            if (upgradeAtan2Instances && nodeCategory == ATAN2)
            {
                InputPtr input = node->getInput(IN1);
                InputPtr input2 = node->getInput(IN2);
                if (input && input2)
                {
                    input->setName(EMPTY_STRING);
                    input2->setName(IN1);
                    input->setName(IN2);
                }
                else
                {
                    if (input)
                    {
                        input->setName(IN2);
                    }
                    if (input2)
                    {
                        input2->setName(IN1);
                    }
                }
            }
            else if (nodeCategory == ROTATE3D)
            {
                ElementPtr axis = node->getChild(AXIS);
                if (axis)
                {
                    updateChildSubclass<Input>(node, axis);
                }
            }
        }

        // Make it so that interface names and nodes in a nodegraph are not duplicates
        // If they are, rename the nodes.
        for (NodeGraphPtr nodegraph : getNodeGraphs())
        {
            // Clear out any erroneously set version 
            nodegraph->removeAttribute(Element::VERSION_ATTRIBUTE);

            StringSet interfaceNames;            
            for (auto child : nodegraph->getChildren())
            {
                NodePtr node = child->asA<Node>();
                if (node)
                {
                    for (ValueElementPtr elem : node->getChildrenOfType<ValueElement>())
                    {
                        const string& interfaceName = elem->getInterfaceName();
                        if (!interfaceName.empty())
                        {
                            interfaceNames.insert(interfaceName);
                        }
                    }
                }
            }
            for (string interfaceName : interfaceNames)
            {
                NodePtr node = nodegraph->getNode(interfaceName);
                if (node)
                {
                    string newNodeName = nodegraph->createValidChildName(interfaceName);
                    vector<MaterialX::PortElementPtr> downstreamPorts = node->getDownstreamPorts();
                    for (MaterialX::PortElementPtr downstreamPort : downstreamPorts)
                    {
                        if (downstreamPort->getNodeName() == interfaceName)
                        {
                            downstreamPort->setNodeName(newNodeName);
                        }
                    }
                    node->setName(newNodeName);
                }
            }
        }   

        // Convert parameters to inputs, applying uniform markings as needed.
        for (ElementPtr interface : traverseTree())
        {
            if (interface->isA<InterfaceElement>())
            {
                for (ElementPtr param : interface->getChildrenOfType<Element>("parameter"))
                {
                    InputPtr input = updateChildSubclass<Input>(interface, param);
                    if (interface->isA<NodeDef>())
                    {
                        // Only strings and filename types should be set as uniforms.
                        const string& inputType = input->getType();
                        if (inputType == FILENAME_TYPE_STRING || inputType == STRING_TYPE_STRING)
                        {
                            input->setIsUniform(true);
                        }
                    }
                }
            }
        }
    }

    setVersionIntegers(majorVersion, minorVersion);
}

void Document::invalidateCache()
{
    _cache->valid = false;
}

} // namespace MaterialX
