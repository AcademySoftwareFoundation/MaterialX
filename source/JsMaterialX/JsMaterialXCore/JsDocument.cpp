#include "../helpers.h"
#include <MaterialXCore/Document.h>

#include <emscripten.h>
#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

extern "C"
{
    EMSCRIPTEN_BINDINGS(document)
    {
        ems::function("createDocument", &mx::createDocument);
        ems::class_<mx::Document, ems::base<mx::GraphElement>>("Document")
            .smart_ptr_constructor("Document", &std::make_shared<mx::Document, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::Document>>("Document")
            .function("initialize", &mx::Document::initialize)
            .function("copy", &mx::Document::copy)
            .function("importLibrary", &mx::Document::importLibrary)
            .function("getReferencedSourceUris", ems::optional_override([](mx::Document &self) {
                          mx::StringSet referenced = self.mx::Document::getReferencedSourceUris();
                          int size = referenced.size();
                          return arrayToVec((std::string *)&referenced, size);
                      }))
            .function("addNodeGraph", &mx::Document::addNodeGraph)
            .function("getNodeGraph", &mx::Document::getNodeGraph)
            .function("getNodeGraphs", &mx::Document::getNodeGraphs)
            .function("removeNodeGraph", &mx::Document::removeNodeGraph)
            .function("getMatchingPorts", &mx::Document::getMatchingPorts)
            .function("addGeomInfo", &mx::Document::addGeomInfo)
            .function("getGeomInfo", &mx::Document::getGeomInfo)
            .function("getGeomInfos", &mx::Document::getGeomInfos)
            .function("removeGeomInfo", &mx::Document::removeGeomInfo)
            .function("getGeomPropValue", &mx::Document::getGeomPropValue)
            .function("addGeomPropDef", &mx::Document::addGeomPropDef)
            .function("getGeomPropDef", &mx::Document::getGeomPropDef)
            .function("getGeomPropDefs", &mx::Document::getGeomPropDefs)
            .function("removeGeomPropDef", &mx::Document::removeGeomPropDef)
            .function("addLook", &mx::Document::addLook)
            .function("getLook", &mx::Document::getLook)
            .function("getLooks", &mx::Document::getLooks)
            .function("removeLook", &mx::Document::removeLook)
            .function("addLookGroup", &mx::Document::addLookGroup)
            .function("getLookGroup", &mx::Document::getLookGroup)
            .function("getLookGroups", &mx::Document::getLookGroups)
            .function("removeLookGroup", &mx::Document::removeLookGroup)
            .function("addCollection", &mx::Document::addCollection)
            .function("getCollection", &mx::Document::getCollection)
            .function("getCollections", &mx::Document::getCollections)
            .function("removeCollection", &mx::Document::removeCollection)
            .function("addTypeDef", &mx::Document::addTypeDef)
            .function("getTypeDef", &mx::Document::getTypeDef)
            .function("getTypeDefs", &mx::Document::getTypeDefs)
            .function("removeTypeDef", &mx::Document::removeTypeDef)
            .function("addNodeDef", &mx::Document::addNodeDef)
            .function("addNodeDefFromGraph", ems::optional_override([](mx::Document &self, 
                                mx::NodeGraphPtr nodeGraph, std::string nodeDefName, std::string node,
                                std::string version, bool isDefaultVersion, std::string nodeGroup, std::string newGraphName)
                      {
                          const std::string &nodeDefName1 = nodeDefName;
                          const std::string &node1 = node;
                          const std::string& version1 = version;
                          const std::string &nodeGroup1 = nodeGroup;
                          std::string &newGraphName1 = newGraphName;
                          return self.addNodeDefFromGraph(nodeGraph, nodeDefName1, node1,
                                                version1, isDefaultVersion, nodeGroup1, newGraphName1);
                      }))
            .function("getNodeDef", &mx::Document::getNodeDef)
            .function("getNodeDefs", &mx::Document::getNodeDefs)
            .function("removeNodeDef", &mx::Document::removeNodeDef)
            .function("getMatchingNodeDefs", &mx::Document::getMatchingNodeDefs)
            .function("getMatchingImplementations", &mx::Document::getMatchingImplementations)
            .function("addPropertySet", &mx::Document::addPropertySet)
            .function("getPropertySet", &mx::Document::getPropertySet)
            .function("getPropertySets", &mx::Document::getPropertySets)
            .function("removePropertySet", &mx::Document::removePropertySet)
            .function("addVariantSet", &mx::Document::addVariantSet)
            .function("getVariantSet", &mx::Document::getVariantSet)
            .function("getVariantSets", &mx::Document::getVariantSets)
            .function("removeVariantSet", &mx::Document::removeVariantSet)
            .function("addImplementation", &mx::Document::addImplementation)
            .function("getImplementation", &mx::Document::getImplementation)
            .function("getImplementations", &mx::Document::getImplementations)
            .function("removeImplementation", &mx::Document::removeImplementation)
            .function("addUnitDef", &mx::Document::addUnitDef)
            .function("getUnitDef", &mx::Document::getUnitDef)
            .function("getUnitDefs", &mx::Document::getUnitDefs)
            .function("removeUnitDef", &mx::Document::removeUnitDef)
            .function("addUnitTypeDef", &mx::Document::addUnitTypeDef)
            .function("getUnitTypeDef", &mx::Document::getUnitTypeDef)
            .function("getUnitTypeDefs", &mx::Document::getUnitTypeDefs)
            .function("removeUnitTypeDef", &mx::Document::removeUnitTypeDef)
            .function("upgradeVersion", &mx::Document::upgradeVersion)
            .function("setColorManagementSystem", &mx::Document::setColorManagementSystem)
            .function("hasColorManagementSystem", &mx::Document::hasColorManagementSystem)
            .function("getColorManagementSystem", &mx::Document::getColorManagementSystem)
            .function("setColorManagementConfig", &mx::Document::setColorManagementConfig)
            .function("hasColorManagementConfig", &mx::Document::hasColorManagementConfig)
            .function("getColorManagementConfig", &mx::Document::getColorManagementConfig);
    }
}