//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Document.h>

namespace py = pybind11;
namespace mx = MaterialX;

class PyBindDocument : public mx::Document
{
  public:
    mx::NodeDefPtr old_addNodeDefFromGraph(mx::NodeGraphPtr nodeGraph, const std::string& nodeDefName, const std::string& node, 
        const std::string&, bool, const std::string&, const std::string& newGraphName)
    {
        PyErr_WarnEx(PyExc_DeprecationWarning,
            "This method is deprecated, use addNodeDefFromGraph(nodeGraph, nodeDefName, category, newGraphName) instead.", 1);
        return addNodeDefFromGraph(nodeGraph, nodeDefName, node, newGraphName);
    }
};

void bindPyDocument(py::module& mod)
{
    mod.def("createDocument", &mx::createDocument, "Create a new document of the given subclass.\n\nCreate a new Document.");

    py::class_<mx::Document, mx::DocumentPtr, mx::GraphElement>(mod, "Document", "A MaterialX document, which represents the top-level element in the MaterialX ownership hierarchy.\n\nUse the factory function createDocument() to create a Document instance.")
        .def("initialize", &mx::Document::initialize, "Initialize with the given implementation element.\n\nInitialization must set the name and hash for the implementation, as well as any other data needed to emit code for the node.")
        .def("copy", &mx::Document::copy, "Create a deep copy of the value.")
        .def("setDataLibrary", &mx::Document::setDataLibrary, "Store a reference to a data library in this document.")
        .def("getDataLibrary", &mx::Document::getDataLibrary, "Return the data library, if any, referenced by this document.")
        .def("hasDataLibrary", &mx::Document::hasDataLibrary, "Return true if this document has a data library.")
        .def("importLibrary", &mx::Document::importLibrary, "Import the given data library into this document.\n\nArgs:\n    library: The data library to be imported.")
        .def("getReferencedSourceUris", &mx::Document::getReferencedSourceUris, "Get a list of source URIs referenced by the document.")
        .def("addNodeGraph", &mx::Document::addNodeGraph,
            py::arg("name") = mx::EMPTY_STRING, "Add a NodeGraph to the document.\n\nArgs:\n    name: The name of the new NodeGraph. If no name is specified, then a unique name will automatically be generated.\n\nReturns:\n    A shared pointer to the new NodeGraph.")
        .def("getNodeGraph", &mx::Document::getNodeGraph, "Return the NodeGraph, if any, with the given name.")
        .def("getNodeGraphs", &mx::Document::getNodeGraphs, "Return a vector of all NodeGraph elements in the document.")
        .def("removeNodeGraph", &mx::Document::removeNodeGraph, "Remove the NodeGraph, if any, with the given name.")
        .def("getMatchingPorts", &mx::Document::getMatchingPorts, "Return a vector of all port elements that match the given node name.\n\nPort elements support spatially-varying upstream connections to nodes, and include both Input and Output elements.")
        .def("addGeomInfo", &mx::Document::addGeomInfo,
            py::arg("name") = mx::EMPTY_STRING, py::arg("geom") = mx::UNIVERSAL_GEOM_NAME, "Add a GeomInfo to the document.\n\nArgs:\n    name: The name of the new GeomInfo. If no name is specified, then a unique name will automatically be generated.\n    geom: An optional geometry string for the GeomInfo.\n\nReturns:\n    A shared pointer to the new GeomInfo.")
        .def("getGeomInfo", &mx::Document::getGeomInfo, "Return the GeomInfo, if any, with the given name.")
        .def("getGeomInfos", &mx::Document::getGeomInfos, "Return a vector of all GeomInfo elements in the document.")
        .def("removeGeomInfo", &mx::Document::removeGeomInfo, "Remove the GeomInfo, if any, with the given name.")
        .def("getGeomPropValue", &mx::Document::getGeomPropValue,
            py::arg("geomPropName"), py::arg("geom") = mx::UNIVERSAL_GEOM_NAME, "Return the value of a geometric property for the given geometry string.")
        .def("addGeomPropDef", &mx::Document::addGeomPropDef, "Add a GeomPropDef to the document.\n\nArgs:\n    name: The name of the new GeomPropDef.\n    geomprop: The geometric property to use for the GeomPropDef.\n\nReturns:\n    A shared pointer to the new GeomPropDef.")
        .def("getGeomPropDef", &mx::Document::getGeomPropDef, "Return the GeomPropDef, if any, with the given name.")
        .def("getGeomPropDefs", &mx::Document::getGeomPropDefs, "Return a vector of all GeomPropDef elements in the document.")
        .def("removeGeomPropDef", &mx::Document::removeGeomPropDef, "Remove the GeomPropDef, if any, with the given name.")
        .def("getMaterialOutputs", &mx::Document::getMaterialOutputs, "Return material-type outputs for all nodegraphs in the document.")        
        .def("addLook", &mx::Document::addLook,
            py::arg("name") = mx::EMPTY_STRING, "Add a Look to the document.\n\nArgs:\n    name: The name of the new Look. If no name is specified, then a unique name will automatically be generated.\n\nReturns:\n    A shared pointer to the new Look.")
        .def("getLook", &mx::Document::getLook, "Return the Look, if any, with the given name.")
        .def("getLooks", &mx::Document::getLooks, "Return a vector of all Look elements in the document.")
        .def("removeLook", &mx::Document::removeLook, "Remove the Look, if any, with the given name.")
        .def("addLookGroup", &mx::Document::addLookGroup,
            py::arg("name") = mx::EMPTY_STRING, "Add a LookGroup to the document.\n\nArgs:\n    name: The name of the new LookGroup. If no name is specified, then a unique name will automatically be generated.\n\nReturns:\n    A shared pointer to the new LookGroup.")
        .def("getLookGroup", &mx::Document::getLookGroup, "Return the LookGroup, if any, with the given name.")
        .def("getLookGroups", &mx::Document::getLookGroups, "Return a vector of all LookGroup elements in the document.")
        .def("removeLookGroup", &mx::Document::removeLookGroup, "Remove the LookGroup, if any, with the given name.")
        .def("addCollection", &mx::Document::addCollection,
            py::arg("name") = mx::EMPTY_STRING, "Add a Collection to the document.\n\nArgs:\n    name: The name of the new Collection. If no name is specified, then a unique name will automatically be generated.\n\nReturns:\n    A shared pointer to the new Collection.")
        .def("getCollection", &mx::Document::getCollection, "Return the Collection, if any, with the given name.")
        .def("getCollections", &mx::Document::getCollections, "Return a vector of all Collection elements in the document.")
        .def("removeCollection", &mx::Document::removeCollection, "Remove the Collection, if any, with the given name.")
        .def("addTypeDef", &mx::Document::addTypeDef,
            py::arg("name") = mx::EMPTY_STRING, "Add a TypeDef to the document.\n\nArgs:\n    name: The name of the new TypeDef. If no name is specified, then a unique name will automatically be generated.\n\nReturns:\n    A shared pointer to the new TypeDef.")
        .def("getTypeDef", &mx::Document::getTypeDef, "Return the TypeDef, if any, with the given name.")
        .def("getTypeDefs", &mx::Document::getTypeDefs, "Return a vector of all TypeDef elements in the document.")
        .def("removeTypeDef", &mx::Document::removeTypeDef, "Remove the TypeDef, if any, with the given name.")
        .def("addNodeDef", &mx::Document::addNodeDef,
            py::arg("name") = mx::EMPTY_STRING, py::arg("type") = mx::DEFAULT_TYPE_STRING, py::arg("node") = mx::EMPTY_STRING, "Add a NodeDef to the document.\n\nArgs:\n    name: The name of the new NodeDef. If no name is specified, then a unique name will automatically be generated.\n    type: An optional type string. If specified, then the new NodeDef will be assigned an Output of the given type.\n    node: An optional node string.\n\nReturns:\n    A shared pointer to the new NodeDef.")
        .def("addNodeDefFromGraph", (mx::NodeDefPtr (mx::Document::*)(mx::NodeGraphPtr, const std::string&, const std::string&, const std::string&)) & mx::Document::addNodeDefFromGraph)
        .def("addNodeDefFromGraph", (mx::NodeDefPtr(mx::Document::*)(mx::NodeGraphPtr, const std::string&, const std::string&, const std::string&,
            bool, const std::string&, const std::string& )) & PyBindDocument::old_addNodeDefFromGraph)
        .def("getNodeDef", &mx::Document::getNodeDef, "Returns a nodedef for a given transform.")
        .def("getNodeDefs", &mx::Document::getNodeDefs, "Return a vector of all NodeDef elements in the document.")
        .def("removeNodeDef", &mx::Document::removeNodeDef, "Remove the NodeDef, if any, with the given name.")
        .def("getMatchingNodeDefs", &mx::Document::getMatchingNodeDefs, "Return a vector of all NodeDef elements that match the given node name.")
        .def("addAttributeDef", &mx::Document::addAttributeDef, "Add an AttributeDef to the document.\n\nArgs:\n    name: The name of the new AttributeDef. If no name is specified, then a unique name will automatically be generated.\n\nReturns:\n    A shared pointer to the new AttributeDef.")
        .def("getAttributeDef", &mx::Document::getAttributeDef, "Return the AttributeDef, if any, with the given name.")
        .def("getAttributeDefs", &mx::Document::getAttributeDefs, "Return a vector of all AttributeDef elements in the document.")
        .def("removeAttributeDef", &mx::Document::removeAttributeDef, "Remove the AttributeDef, if any, with the given name.")
        .def("addTargetDef", &mx::Document::addTargetDef, "Add an TargetDef to the document.\n\nArgs:\n    name: The name of the new TargetDef. If no name is specified, then a unique name will automatically be generated.\n\nReturns:\n    A shared pointer to the new TargetDef.")
        .def("getTargetDef", &mx::Document::getTargetDef, "Return the AttributeDef, if any, with the given name.")
        .def("getTargetDefs", &mx::Document::getTargetDefs, "Return a vector of all TargetDef elements in the document.")
        .def("removeTargetDef", &mx::Document::removeTargetDef, "Remove the TargetDef, if any, with the given name.")
        .def("addPropertySet", &mx::Document::addPropertySet,
            py::arg("name") = mx::EMPTY_STRING, "Add a PropertySet to the document.\n\nArgs:\n    name: The name of the new PropertySet. If no name is specified, then a unique name will automatically be generated.\n\nReturns:\n    A shared pointer to the new PropertySet.")
        .def("getPropertySet", &mx::Document::getPropertySet, "Return the PropertySet, if any, with the given name.")
        .def("getPropertySets", &mx::Document::getPropertySets, "Return a vector of all PropertySet elements in the document.")
        .def("removePropertySet", &mx::Document::removePropertySet, "Remove the PropertySet, if any, with the given name.")
        .def("addVariantSet", &mx::Document::addVariantSet,
            py::arg("name") = mx::EMPTY_STRING, "Add a VariantSet to the document.\n\nArgs:\n    name: The name of the new VariantSet. If no name is specified, then a unique name will automatically be generated.\n\nReturns:\n    A shared pointer to the new VariantSet.")
        .def("getVariantSet", &mx::Document::getVariantSet, "Return the VariantSet, if any, with the given name.")
        .def("getVariantSets", &mx::Document::getVariantSets, "Return a vector of all VariantSet elements in the document.")
        .def("removeVariantSet", &mx::Document::removeVariantSet, "Remove the VariantSet, if any, with the given name.")
        .def("addImplementation", &mx::Document::addImplementation,
            py::arg("name") = mx::EMPTY_STRING, "Add an Implementation to the document.\n\nArgs:\n    name: The name of the new Implementation. If no name is specified, then a unique name will automatically be generated.\n\nReturns:\n    A shared pointer to the new Implementation.")
        .def("getImplementation", &mx::Document::getImplementation, "Return the Implementation, if any, with the given name.")
        .def("getImplementations", &mx::Document::getImplementations, "Return a vector of all Implementation elements in the document.")
        .def("removeImplementation", &mx::Document::removeImplementation, "Remove the Implementation, if any, with the given name.")
        .def("getMatchingImplementations", &mx::Document::getMatchingImplementations, "Return a vector of all node implementations that match the given NodeDef string.\n\nNote that a node implementation may be either an Implementation element or NodeGraph element.")
        .def("addUnitDef", &mx::Document::addUnitDef)
        .def("getUnitDef", &mx::Document::getUnitDef, "Return the UnitDef, if any, with the given name.")
        .def("getUnitDefs", &mx::Document::getUnitDefs, "Return a vector of all Member elements in the TypeDef.")
        .def("removeUnitDef", &mx::Document::removeUnitDef, "Remove the UnitDef, if any, with the given name.")
        .def("addUnitTypeDef", &mx::Document::addUnitTypeDef)
        .def("getUnitTypeDef", &mx::Document::getUnitTypeDef, "Return the UnitTypeDef, if any, with the given name.")
        .def("getUnitTypeDefs", &mx::Document::getUnitTypeDefs, "Return a vector of all UnitTypeDef elements in the document.")
        .def("removeUnitTypeDef", &mx::Document::removeUnitTypeDef, "Remove the UnitTypeDef, if any, with the given name.")
        .def("upgradeVersion", &mx::Document::upgradeVersion, "Upgrade the content of this document from earlier supported versions to the library version.")
        .def("setColorManagementSystem", &mx::Document::setColorManagementSystem, "Set the color management system string.")
        .def("hasColorManagementSystem", &mx::Document::hasColorManagementSystem, "Return true if a color management system string has been set.")
        .def("getColorManagementSystem", &mx::Document::getColorManagementSystem, "Return the color management system string.")
        .def("setColorManagementConfig", &mx::Document::setColorManagementConfig, "Set the color management config string.")
        .def("hasColorManagementConfig", &mx::Document::hasColorManagementConfig, "Return true if a color management config string has been set.")
        .def("getColorManagementConfig", &mx::Document::getColorManagementConfig, "Return the color management config string.");
}
