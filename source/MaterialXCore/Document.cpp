//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Document.h>

#include <MaterialXCore/Util.h>

#include <iterator>
#include <mutex>
#include <sstream>

namespace MaterialX
{

const string DOCUMENT_VERSION_STRING = std::to_string(MAJOR_VERSION) + "." +
                                       std::to_string(MINOR_VERSION);

const string Document::VERSION_ATTRIBUTE = "version";
const string Document::REQUIRE_ATTRIBUTE = "require";
const string Document::CMS_ATTRIBUTE = "cms";
const string Document::CMS_CONFIG_ATTRIBUTE = "cmsconfig";
const string Document::REQUIRE_STRING_MATINHERIT = "matinherit";
const string Document::REQUIRE_STRING_MATNODEGRAPH = "matnodegraph";
const string Document::REQUIRE_STRING_OVERRIDE = "override";

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
            publicElementMap.clear();
            nodeDefMap.clear();
            implementationMap.clear();

            // Traverse the document to build a new cache.
            for (ElementPtr elem : doc.lock()->traverseTree())
            {
                PortElementPtr portElem = elem->asA<PortElement>();
                ValueElementPtr valueElem = elem->asA<ValueElement>();
                NodeDefPtr nodeDef = elem->asA<NodeDef>();
                NodeGraphPtr nodeGraph = elem->asA<NodeGraph>();
                ImplementationPtr implementation = elem->asA<Implementation>();

                if (portElem && portElem->hasNodeName())
                {
                    portElementMap.insert(std::pair<NodePtr, PortElementPtr>(
                        portElem->getConnectedNode(),
                        portElem));
                }
                if (valueElem && valueElem->hasPublicName())
                {
                    publicElementMap.insert(std::pair<string, ValueElementPtr>(
                        valueElem->getPublicName(),
                        valueElem));
                }
                if (nodeDef && nodeDef->hasNode())
                {
                    nodeDefMap.insert(std::pair<string, NodeDefPtr>(
                        nodeDef->getNode(),
                        nodeDef));
                }
                if (nodeGraph && nodeGraph->hasNodeDef())
                {
                    implementationMap.insert(std::pair<string, ElementPtr>(
                        nodeGraph->getNodeDef(),
                        nodeGraph));
                }
                if (implementation && implementation->hasNodeDef())
                {
                    implementationMap.insert(std::pair<string, ElementPtr>(
                        implementation->getNodeDef(),
                        implementation));
                }
            }

            valid = true;
        }
    }

  public:
    weak_ptr<Document> doc;
    std::mutex mutex;
    bool valid;
    std::unordered_multimap<ConstElementPtr, PortElementPtr> portElementMap;
    std::unordered_multimap<string, ValueElementPtr> publicElementMap;
    std::unordered_multimap<string, NodeDefPtr> nodeDefMap;
    std::unordered_multimap<string, ElementPtr> implementationMap;
};

//
// Document methods
//

Document::Document(ElementPtr parent, const string& name) :
    Element(parent, CATEGORY, name),
    _cache(std::unique_ptr<Cache>(new Cache))
{
}

Document::~Document()
{
}

void Document::initialize()
{
    _root = getSelf();

    DocumentPtr doc = getDocument();
    _cache->doc = doc;

    // Handle change notifications.
    ScopedUpdate update(doc);
    onInitialize();

    clearContent();
    setVersionString(DOCUMENT_VERSION_STRING);
}

void Document::importLibrary(ConstDocumentPtr library, bool skipDuplicates)
{
    for (ElementPtr child : library->getChildren())
    {
        std::string childName = child->getName();
        if (skipDuplicates && getChild(childName))
        {
            continue;
        }
        ElementPtr childCopy = addChildOfCategory(child->getCategory(), childName);
        childCopy->copyContentFrom(child, skipDuplicates);
        if (!childCopy->hasSourceUri())
        {
            childCopy->setSourceUri(library->getSourceUri());
        }
    }
}

std::pair<int, int> Document::getVersionIntegers()
{
    string versionString = getVersionString();
    if (versionString.empty())
    {
        return std::pair<int, int>(MAJOR_VERSION, MINOR_VERSION);
    }

    vector<string> splitVersion = splitString(versionString, ".");
    if (splitVersion.size() == 2)
    {
        return std::pair<int, int>(std::stoi(splitVersion[0]),
                                   std::stoi(splitVersion[1]));
    }

    return std::pair<int, int>(0, 0);
}

vector<PortElementPtr> Document::getMatchingPorts(const ConstElementPtr& node) const
{
    // Refresh the cache.
    _cache->refresh();

    // Find all port elements matching the given node.
    vector<PortElementPtr> ports;
    auto keyRange = _cache->portElementMap.equal_range(node);
    for (auto it = keyRange.first; it != keyRange.second; ++it)
    {
        ports.push_back(it->second);
    }

    // Return the matches.
    return ports;
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

vector<ElementPtr> Document::getMatchingImplementations(const string& nodeDef) const
{
    // Refresh the cache.
    _cache->refresh();

    // Find all implementations matching the given nodedef string.
    vector<ElementPtr> implementations;
    auto keyRange = _cache->implementationMap.equal_range(nodeDef);
    for (auto it = keyRange.first; it != keyRange.second; ++it)
    {
        implementations.push_back(it->second);
    }

    // Return the matches.
    return implementations;
}

ElementPtr Document::getPublicElement(const string& publicName) const
{
    // Refresh the cache.
    _cache->refresh();

    // Return any element with the given public name.
    auto it = _cache->publicElementMap.find(publicName);
    if (it != _cache->publicElementMap.end())
    {
        return it->second;
    }

    return ElementPtr();
}

vector<ElementPtr> Document::getPublicElements(const string& publicName) const
{
    // Refresh the cache.
    _cache->refresh();

    // Find all elements with the given public name.
    vector<ElementPtr> publicElements;
    auto keyRange = _cache->publicElementMap.equal_range(publicName);
    for (auto it = keyRange.first; it != keyRange.second; ++it)
    {
        publicElements.push_back(it->second);
    }

    // Return the matches.
    return publicElements;
}

StringMap Document::getFilenameStringMap(const string& geom) const
{
    StringMap map;
    for (GeomInfoPtr geomInfo : getGeomInfos())
    {
        if (!geomStringsMatch(geom, geomInfo->getGeom()))
            continue;
        for (GeomAttrPtr geomAttr : geomInfo->getGeomAttrs())
        {
            string key = "%" + geomAttr->getName();
            string value = geomAttr->getResolvedValueString();
            map[key] = value;
        }
    }
    return map;
}

string Document::applyStringSubstitutions(const string& filename, const string& geom) const
{
    return replaceSubstrings(filename, getFilenameStringMap(geom));
}

void Document::generateRequireString()
{
    std::set<string> requireSet;
    for (ElementPtr elem : traverseTree())
    {
        if (elem->isA<ShaderRef>())
        {
            ShaderRefPtr shaderRef = elem->asA<ShaderRef>();
            if (!shaderRef->getReferencedOutputs().empty())
            {
                requireSet.insert(REQUIRE_STRING_MATNODEGRAPH);
            }
        }
        else if (elem->isA<Override>())
        {
            requireSet.insert(REQUIRE_STRING_OVERRIDE);
        }
        else if (elem->isA<MaterialInherit>())
        {
            requireSet.insert(REQUIRE_STRING_MATINHERIT);
        }
    }

    string requireStr;
    for (string str : requireSet)
    {
        if (!requireStr.empty())
            requireStr += ",";
        requireStr += str;
    }
    setRequireString(requireStr);
}

bool Document::validate(string* message) const
{
    bool res = true;
    validateRequire(hasVersionString(), res, message, "Missing version string");
    return Element::validate(message) && res;
}

void Document::upgradeVersion()
{
    std::pair<int, int> versions = getVersionIntegers();
    int majorVersion = versions.first;
    int minorVersion = versions.second;
    if (majorVersion == MAJOR_VERSION && minorVersion == MINOR_VERSION)
        return;

    // Upgrade from v1.22 to v1.23
    if (majorVersion == 1 && minorVersion == 22)
    {
        for (ElementPtr elem : traverseTree())
        {
            if (elem->isA<TypedElement>())
            {
                TypedElementPtr typedElem = elem->asA<TypedElement>();
                if (typedElem->getType() == "vector")
                {
                    typedElem->setType(getTypeString<Vector3>());
                }
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
            vector<ElementPtr> origChildren = elem->getChildren();
            for (ElementPtr child : origChildren)
            {
                if (child->getCategory() == "assign")
                {
                    updateChildSubclass<MaterialAssign>(elem, child);
                }
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
            if (elem->isA<Node>("constant"))
            {
                NodePtr constant = elem->asA<Node>();
                ParameterPtr colorParam = constant->getChildOfType<Parameter>("color");
                if (colorParam)
                {
                    ParameterPtr valueParam = constant->addParameter("value");
                    valueParam->copyContentFrom(colorParam);
                    constant->removeParameter(colorParam->getName());
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
                        nodeDef->setType(nodeDef->getAttribute("shadertype") + "shader");
                        nodeDef->removeAttribute("shadertype");
                    }
                    if (nodeDef->hasAttribute("shaderprogram"))
                    {
                        nodeDef->setNode(nodeDef->getAttribute("shaderprogram"));
                        nodeDef->removeAttribute("shaderprogram");
                    }
                }
                else if (child->isA<Parameter>())
                {
                    ParameterPtr param = child->asA<Parameter>();
                    if (param->getType() == "opgraphnode")
                    {
                        if (elem->isA<Node>())
                        {
                            InputPtr input = updateChildSubclass<Input>(elem, param);
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
        for (MaterialPtr mat : getMaterials())
        {
            for (ShaderRefPtr shaderRef : mat->getShaderRefs())
            {
                if (!shaderRef->getReferencedShaderDef())
                {
                    NodeDefPtr nodeDef = getNodeDef(shaderRef->getName());
                    if (nodeDef)
                    {
                        shaderRef->setNodeDef(nodeDef->getName());
                    }
                }
            }
        }

        // Move connections from nodedef inputs to bindinputs.
        for (NodeDefPtr nodeDef : getNodeDefs())
        {
            for (InputPtr input : nodeDef->getInputs())
            {
                if (input->hasAttribute("opgraph") && input->hasAttribute("graphoutput"))
                {
                    for (MaterialPtr mat : getMaterials())
                    {
                        for (ShaderRefPtr shaderRef : mat->getShaderRefs())
                        {
                            if (nodeDef == shaderRef->getReferencedShaderDef())
                            {
                                if (shaderRef->getChild(input->getName()))
                                {
                                    shaderRef = shaderRef;
                                }
                                BindInputPtr bind = shaderRef->addBindInput(input->getName(), input->getType());
                                bind->setNodeGraphString(input->getAttribute("opgraph"));
                                bind->setOutputString(input->getAttribute("graphoutput"));
                            }
                        }
                    }
                    input->removeAttribute("opgraph");
                    input->removeAttribute("graphoutput");
                }
            }
        }

        minorVersion = 34;
    }

    // Upgrade from v1.34 to v1.35
    if (majorVersion == 1 && minorVersion == 34)
    {
        for (ElementPtr elem : traverseTree())
        {
            TypedElementPtr typedElem = elem->asA<TypedElement>();
            ValueElementPtr valueElem = elem->asA<ValueElement>();
            MaterialAssignPtr matAssign = elem->asA<MaterialAssign>();
            if (typedElem && typedElem->getType() == "matrix")
            {
                typedElem->setType(getTypeString<Matrix4x4>());
            }
            if (valueElem && valueElem->hasAttribute("default"))
            {
                valueElem->setValueString(elem->getAttribute("default"));
                valueElem->removeAttribute("default");
            }
            if (matAssign)
            {
                matAssign->setMaterial(matAssign->getName());
            }
        }
        minorVersion = 35;
    }

    if (majorVersion == MAJOR_VERSION && minorVersion == MINOR_VERSION)
    {
        setVersionString(DOCUMENT_VERSION_STRING);
    }
}

void Document::onAddElement(ElementPtr, ElementPtr)
{
    _cache->valid = false;
}

void Document::onRemoveElement(ElementPtr, ElementPtr)
{
    _cache->valid = false;
}

void Document::onSetAttribute(ElementPtr, const string&, const string&)
{
    _cache->valid = false;
}

void Document::onRemoveAttribute(ElementPtr, const string&)
{
    _cache->valid = false;
}

} // namespace MaterialX
