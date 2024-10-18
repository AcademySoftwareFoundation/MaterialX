//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXCore/Document.h>

#include <mutex>

MATERIALX_NAMESPACE_BEGIN

const string Document::CMS_ATTRIBUTE = "cms";
const string Document::CMS_CONFIG_ATTRIBUTE = "cmsconfig";

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
                const string& nodeGraphName = elem->getAttribute(PortElement::NODE_GRAPH_ATTRIBUTE);
                const string& nodeString = elem->getAttribute(NodeDef::NODE_ATTRIBUTE);
                const string& nodeDefString = elem->getAttribute(InterfaceElement::NODE_DEF_ATTRIBUTE);

                if (!nodeName.empty())
                {
                    PortElementPtr portElem = elem->asA<PortElement>();
                    if (portElem)
                    {
                        portElementMap[portElem->getQualifiedName(nodeName)].push_back(portElem);
                    }
                }
                else
                {
                    if (!nodeGraphName.empty())
                    {
                        PortElementPtr portElem = elem->asA<PortElement>();
                        if (portElem)
                        {
                            portElementMap[portElem->getQualifiedName(nodeGraphName)].push_back(portElem);
                        }
                    }
                }
                if (!nodeString.empty())
                {
                    NodeDefPtr nodeDef = elem->asA<NodeDef>();
                    if (nodeDef)
                    {
                        nodeDefMap[nodeDef->getQualifiedName(nodeString)].push_back(nodeDef);
                    }
                }
                if (!nodeDefString.empty())
                {
                    InterfaceElementPtr interface = elem->asA<InterfaceElement>();
                    if (interface)
                    {
                        if (interface->isA<NodeGraph>())
                        {
                            implementationMap[interface->getQualifiedName(nodeDefString)].push_back(interface);
                        }
                        ImplementationPtr impl = interface->asA<Implementation>();
                        if (impl)
                        {
                            // Check for implementation which specifies a nodegraph as the implementation
                            const string& nodeGraphString = impl->getNodeGraph();
                            if (!nodeGraphString.empty())
                            {
                                NodeGraphPtr nodeGraph = impl->getDocument()->getNodeGraph(nodeGraphString);
                                if (nodeGraph)
                                    implementationMap[interface->getQualifiedName(nodeDefString)].push_back(nodeGraph);
                            }
                            else
                            {
                                implementationMap[interface->getQualifiedName(nodeDefString)].push_back(interface);
                            }
                        }
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
    std::unordered_map<string, std::vector<PortElementPtr>> portElementMap;
    std::unordered_map<string, std::vector<NodeDefPtr>> nodeDefMap;
    std::unordered_map<string, std::vector<InterfaceElementPtr>> implementationMap;
};

//
// Document methods
//

Document::Document(ElementPtr parent, const string& name) :
    GraphElement(parent, CATEGORY, name),
    _cache(std::make_unique<Cache>())
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

NodeDefPtr Document::addNodeDefFromGraph(NodeGraphPtr nodeGraph, const string& nodeDefName,
                                         const string& category, const string& newGraphName)
{
    if (category.empty())
    {
        throw Exception("Cannot create a nodedef without a category identifier");
    }

    if (getNodeDef(nodeDefName))
    {
        throw Exception("Cannot create duplicate nodedef: " + nodeDefName);
    }

    if (getNodeGraph(newGraphName))
    {
        throw Exception("Cannot create duplicate nodegraph: " + newGraphName);
    }

    // Create a new functional nodegraph, and copy over the
    // contents from the compound nodegraph
    NodeGraphPtr graph = addNodeGraph(newGraphName);
    graph->copyContentFrom(nodeGraph);

    for (auto graphChild : graph->getChildren())
    {
        graphChild->removeAttribute(Element::XPOS_ATTRIBUTE);
        graphChild->removeAttribute(Element::YPOS_ATTRIBUTE);
    }
    graph->setNodeDefString(nodeDefName);

    // Create a new nodedef and set its category
    NodeDefPtr nodeDef = addNodeDef(nodeDefName, EMPTY_STRING);
    nodeDef->setNodeString(category);

    // Expose any existing interfaces from the graph.
    // Any connection attributes ("nodegraph", "nodename", "interfacename") on the
    // existing interface should be removed from the definition as well as any source URI.

    // Attributes which should not be copied over
    StringSet filterAttributes = { PortElement::NODE_GRAPH_ATTRIBUTE, PortElement::NODE_NAME_ATTRIBUTE,
                                   PortElement::INTERFACE_NAME_ATTRIBUTE, Element::XPOS_ATTRIBUTE, Element::YPOS_ATTRIBUTE };

    // Transfer input interface from the graph to the nodedef
    for (InputPtr input : graph->getInputs())
    {
        InputPtr nodeDefInput = nodeDef->addInput(input->getName(), input->getType());
        if (nodeDefInput)
        {
            nodeDefInput->copyContentFrom(input);
            for (const string& filterAttribute : filterAttributes)
            {
                nodeDefInput->removeAttribute(filterAttribute);
            }
            nodeDefInput->setSourceUri(EMPTY_STRING);
            input->setInterfaceName(nodeDefInput->getName());
        }
    }
    // Remove interfaces from the nodegraph
    for (InputPtr input : graph->getInputs())
    {
        graph->removeInput(input->getName());
    }

    // Copy the output interface from the graph to the nodedef
    for (OutputPtr output : graph->getOutputs())
    {
        OutputPtr nodeDefOutput = nodeDef->addOutput(output->getName(), output->getType());
        if (nodeDefOutput)
        {
            nodeDefOutput->copyContentFrom(output);
            for (const string& filterAttribute : filterAttributes)
            {
                nodeDefOutput->removeAttribute(filterAttribute);
            }
            nodeDefOutput->setSourceUri(EMPTY_STRING);
        }
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
        return { MATERIALX_MAJOR_VERSION, MATERIALX_MINOR_VERSION };
    }
    return InterfaceElement::getVersionIntegers();
}

vector<PortElementPtr> Document::getMatchingPorts(const string& nodeName) const
{
    // Refresh the cache.
    _cache->refresh();

    // Return all port elements matching the given node name.
    if (_cache->portElementMap.count(nodeName))
    {
        return _cache->portElementMap.at(nodeName);
    }
    else
    {
        return vector<PortElementPtr>();
    }
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

vector<OutputPtr> Document::getMaterialOutputs() const
{
    vector<OutputPtr> materialOutputs;

    const string documentUri = getSourceUri();
    for (NodeGraphPtr docNodeGraph : getNodeGraphs())
    {
        // Skip nodegraphs which are either definitions or are from an included file.
        const string graphUri = docNodeGraph->getSourceUri();
        if (docNodeGraph->getNodeDef() || (!graphUri.empty() && documentUri != graphUri))
        {
            continue;
        }

        vector<OutputPtr> docNodeGraphOutputs = docNodeGraph->getMaterialOutputs();
        if (!docNodeGraphOutputs.empty())
        {
            materialOutputs.insert(materialOutputs.end(), docNodeGraphOutputs.begin(), docNodeGraphOutputs.end());
        }
    }
    return materialOutputs;
}

vector<NodeDefPtr> Document::getMatchingNodeDefs(const string& nodeName) const
{
    // Recurse to data library if present.
    vector<NodeDefPtr> matchingNodeDefs = hasDataLibrary() ? 
                                          getDataLibrary()->getMatchingNodeDefs(nodeName) :
                                          vector<NodeDefPtr>();

    // Refresh the cache.
    _cache->refresh();

    // Return all nodedefs matching the given node name.
    if (_cache->nodeDefMap.count(nodeName))
    {
        matchingNodeDefs.insert(matchingNodeDefs.end(), _cache->nodeDefMap.at(nodeName).begin(), _cache->nodeDefMap.at(nodeName).end());
    }
    
    return matchingNodeDefs;
}

vector<InterfaceElementPtr> Document::getMatchingImplementations(const string& nodeDef) const
{
    // Recurse to data library if present.
    vector<InterfaceElementPtr> matchingImplementations = hasDataLibrary() ?
                                                          getDataLibrary()->getMatchingImplementations(nodeDef) :
                                                          vector<InterfaceElementPtr>();
    
    // Refresh the cache.
    _cache->refresh();

    // Return all implementations matching the given nodedef string.
    if (_cache->implementationMap.count(nodeDef))
    {
        matchingImplementations.insert(matchingImplementations.end(), _cache->implementationMap.at(nodeDef).begin(), _cache->implementationMap.at(nodeDef).end());
    }

    return matchingImplementations;
}

bool Document::validate(string* message) const
{
    bool res = true;
    std::pair<int, int> expectedVersion(MATERIALX_MAJOR_VERSION, MATERIALX_MINOR_VERSION);
    validateRequire(getVersionIntegers() >= expectedVersion, res, message, "Unsupported document version");
    validateRequire(getVersionIntegers() <= expectedVersion, res, message, "Future document version");
    return GraphElement::validate(message) && res;
}

void Document::invalidateCache()
{
    _cache->valid = false;
}

//
// Deprecated methods
//

NodeDefPtr Document::addNodeDefFromGraph(NodeGraphPtr nodeGraph, const string& nodeDefName, const string& node,
                                         const string&, bool, const string&, const string& newGraphName)
{
    return addNodeDefFromGraph(nodeGraph, nodeDefName, node, newGraphName);
}

MATERIALX_NAMESPACE_END
