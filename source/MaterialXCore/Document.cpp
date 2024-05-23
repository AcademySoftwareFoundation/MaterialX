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

NodeDefPtr Document::addNodeDefFromGraph(const NodeGraphPtr nodeGraph, const string& nodeDefName, const string& node,
                                         const string& version, bool isDefaultVersion, const string& group, const string& newGraphName)
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

void Document::importLibrary(const ConstDocumentPtr& library, bool errorOnDuplicates)
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
            if (errorOnDuplicates)
                throw Exception("Trying to import a child that already exists " + child->getName());
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
    // Refresh the cache.
    _cache->refresh();

    // Return all nodedefs matching the given node name.
    if (_cache->nodeDefMap.count(nodeName))
    {
        return _cache->nodeDefMap.at(nodeName);
    }
    else
    {
        return vector<NodeDefPtr>();
    }
}

vector<InterfaceElementPtr> Document::getMatchingImplementations(const string& nodeDef) const
{
    // Refresh the cache.
    _cache->refresh();

    // Return all implementations matching the given nodedef string.
    if (_cache->implementationMap.count(nodeDef))
    {
        return _cache->implementationMap.at(nodeDef);
    }
    else
    {
        return vector<InterfaceElementPtr>();
    }
}

bool Document::validate(string* message, const ValidationOptions* validationOptions) const
{
    bool res = true;
    std::pair<int, int> expectedVersion(MATERIALX_MAJOR_VERSION, MATERIALX_MINOR_VERSION);
    validateRequire(getVersionIntegers() >= expectedVersion, res, message, "Unsupported document version");
    validateRequire(getVersionIntegers() <= expectedVersion, res, message, "Future document version");
    return GraphElement::validate(message, validationOptions) && res;
}

void Document::invalidateCache()
{
    _cache->valid = false;
}

MATERIALX_NAMESPACE_END
