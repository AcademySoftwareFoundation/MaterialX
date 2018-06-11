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

const string DOCUMENT_VERSION_STRING = std::to_string(MATERIALX_MAJOR_VERSION) + "." +
                                       std::to_string(MATERIALX_MINOR_VERSION);

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
            nodeDefMap.clear();
            implementationMap.clear();

            // Traverse the document to build a new cache.
            for (ElementPtr elem : doc.lock()->traverseTree())
            {
                PortElementPtr portElem = elem->asA<PortElement>();
                NodeDefPtr nodeDef = elem->asA<NodeDef>();
                NodeGraphPtr nodeGraph = elem->asA<NodeGraph>();
                ImplementationPtr implementation = elem->asA<Implementation>();

                if (portElem && portElem->hasNodeName())
                {
                    portElementMap.insert(std::pair<string, PortElementPtr>(
                        portElem->getQualifiedName(portElem->getNodeName()),
                        portElem));
                }
                if (nodeDef && nodeDef->hasNodeString())
                {
                    nodeDefMap.insert(std::pair<string, NodeDefPtr>(
                        nodeDef->getQualifiedName(nodeDef->getNodeString()),
                        nodeDef));
                }
                if (nodeGraph && nodeGraph->hasNodeDefString())
                {
                    implementationMap.insert(std::pair<string, InterfaceElementPtr>(
                        nodeGraph->getQualifiedName(nodeGraph->getNodeDefString()),
                        nodeGraph));
                }
                if (implementation && implementation->hasNodeDefString())
                {
                    implementationMap.insert(std::pair<string, InterfaceElementPtr>(
                        implementation->getQualifiedName(implementation->getNodeDefString()),
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

    DocumentPtr doc = getDocument();
    _cache->doc = doc;

    // Handle change notifications.
    ScopedUpdate update(doc);
    onInitialize();

    clearContent();
    setVersionString(DOCUMENT_VERSION_STRING);
}

void Document::importLibrary(ConstDocumentPtr library, const CopyOptions* copyOptions)
{
    bool skipDuplicateElements = copyOptions && copyOptions->skipDuplicateElements;
    bool copySourceUris = copyOptions && copyOptions->copySourceUris;
    for (ElementPtr child : library->getChildren())
    {
        string childName = child->getQualifiedName(child->getName());
        if (skipDuplicateElements && getChild(childName))
        {
            continue;
        }

        ElementPtr childCopy = addChildOfCategory(child->getCategory(), childName);
        childCopy->copyContentFrom(child, copyOptions);
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
        if (copySourceUris && !childCopy->hasSourceUri())
        {
            childCopy->setSourceUri(library->getSourceUri());
        }
    }
}

std::pair<int, int> Document::getVersionIntegers() const
{
    if (!hasVersionString())
    {
        return {MATERIALX_MAJOR_VERSION, MATERIALX_MINOR_VERSION};
    }
    return InterfaceElement::getVersionIntegers();
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
    return Element::validate(message) && res;
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
                        nodeDef->setType(nodeDef->getAttribute("shadertype") + "shader");
                        nodeDef->removeAttribute("shadertype");
                    }
                    if (nodeDef->hasAttribute("shaderprogram"))
                    {
                        nodeDef->setNodeString(nodeDef->getAttribute("shaderprogram"));
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
                if (!shaderRef->getNodeDef())
                {
                    NodeDefPtr nodeDef = getNodeDef(shaderRef->getName());
                    if (nodeDef)
                    {
                        shaderRef->setNodeDefString(nodeDef->getName());
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
                            if (nodeDef == shaderRef->getNodeDef())
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
                typedElem->setType(getTypeString<Matrix44>());
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

    // Upgrade from v1.35 to v1.36
    if (majorVersion == 1 && minorVersion == 35)
    {
        for (ElementPtr elem : traverseTree())
        {
            ValueElementPtr valueElem = elem->asA<ValueElement>();
            MaterialPtr material = elem->asA<Material>();
            LookPtr look = elem->asA<Look>();
            GeomInfoPtr geomInfo = elem->asA<GeomInfo>();

            if (valueElem)
            {
                if (valueElem->getType() == GEOMNAME_TYPE_STRING &&
                    valueElem->getValueString() == "*")
                {
                    valueElem->setValueString(UNIVERSAL_GEOM_NAME);
                }
            }

            vector<ElementPtr> origChildren = elem->getChildren();
            for (ElementPtr child : origChildren)
            {
                if (material && child->getCategory() == "override")
                {
                    for (ShaderRefPtr shaderRef : material->getShaderRefs())
                    {
                        NodeDefPtr nodeDef = shaderRef->getNodeDef();
                        for (ValueElementPtr activeValue : nodeDef->getActiveValueElements())
                        {
                            if (activeValue->getAttribute("publicname") == child->getName() &&
                                !shaderRef->getChild(child->getName()))
                            {
                                if (activeValue->isA<Parameter>())
                                {
                                    BindParamPtr bindParam = shaderRef->addBindParam(activeValue->getName(), activeValue->getType());
                                    bindParam->setValueString(child->getAttribute("value"));
                                }
                                else if (activeValue->isA<Input>())
                                {
                                    BindInputPtr bindInput = shaderRef->addBindInput(activeValue->getName(), activeValue->getType());
                                    bindInput->setValueString(child->getAttribute("value"));
                                }
                            }
                        }
                    }
                    elem->removeChild(child->getName());
                }
                else if (material && child->getCategory() == "materialinherit")
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

    if (majorVersion == MATERIALX_MAJOR_VERSION &&
        minorVersion == MATERIALX_MINOR_VERSION)
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
