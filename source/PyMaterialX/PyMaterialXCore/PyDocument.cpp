//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Document.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyDocument(py::module& mod)
{
    mod.def("createDocument", &mx::createDocument,
            PYMATERIALX_DOCSTRING(R"docstring(
    Create a MaterialX `Document` instance, which represents the top-level
    element in the MaterialX ownership hierarchy.
)docstring"));

    py::class_<mx::Document, mx::DocumentPtr, mx::GraphElement>(mod, "Document")

        .def("initialize", &mx::Document::initialize,
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize the document, removing any existing content.
)docstring"))

        .def("copy", &mx::Document::copy,
             PYMATERIALX_DOCSTRING(R"docstring(
    Create a deep copy of the document.
)docstring"))

        .def("importLibrary", &mx::Document::importLibrary,
             py::arg("library"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Import the given document as a library within this document.

    The contents of the library document are copied into this one, and
    are assigned the source URI of the library.

    :type library: `Document`
    :param library: The library document to be imported.
)docstring"))

        .def("getReferencedSourceUris", &mx::Document::getReferencedSourceUris,
             PYMATERIALX_DOCSTRING(R"docstring(
    Get a list of source URI's referenced by the document.
)docstring"))

        .def("addNodeGraph", &mx::Document::addNodeGraph,
             py::arg("name") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `NodeGraph` to the document.

    :type name: str
    :param name: The name of the new NodeGraph.
        If no name is specified, then a unique name will automatically be
        generated.
    :returns: The new `NodeGraph`.
)docstring"))

        .def("getNodeGraph", &mx::Document::getNodeGraph,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `NodeGraph`, if any, with the given `name`.
)docstring"))

        .def("getNodeGraphs", &mx::Document::getNodeGraphs,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `NodeGraph` elements in the document.
)docstring"))

        .def("removeNodeGraph", &mx::Document::removeNodeGraph,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `NodeGraph`, if any, with the given `name`.
)docstring"))

        .def("getMatchingPorts", &mx::Document::getMatchingPorts,
             py::arg("nodeName"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all port elements that match the given node name.
    Port elements support spatially-varying upstream connections to
    nodes, and include both `Input` and `Output` elements.
)docstring"))

        .def("addGeomInfo", &mx::Document::addGeomInfo,
             py::arg("name") = mx::EMPTY_STRING,
             py::arg_v("geom",
                       mx::UNIVERSAL_GEOM_NAME,
                       "mx.UNIVERSAL_GEOM_NAME"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `GeomInfo` to the document.

    :type name: str
    :param name: The name of the new `GeomInfo`.
        If no name is specified, then a unique name will automatically be
        generated.
    :type geom: str
    :param geom: An optional geometry string for the `GeomInfo`.
    :returns: The new `GeomInfo`.
)docstring"))

        .def("getGeomInfo", &mx::Document::getGeomInfo,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `GeomInfo`, if any, with the given `name`.
)docstring"))

        .def("getGeomInfos", &mx::Document::getGeomInfos,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `GeomInfo` elements in the document.
)docstring"))

        .def("removeGeomInfo", &mx::Document::removeGeomInfo,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `GeomInfo`, if any, with the given `name`.
)docstring"))

        .def("getGeomPropValue", &mx::Document::getGeomPropValue,
             py::arg("geomPropName"),
             py::arg_v("geom",
                       mx::UNIVERSAL_GEOM_NAME,
                       "mx.UNIVERSAL_GEOM_NAME"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the value of a geometric property for the given geometry string.
)docstring"))

        .def("addGeomPropDef", &mx::Document::addGeomPropDef,
             py::arg("name"),
             py::arg("geomprop"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `GeomPropDef` to the document.

    :type name: str
    :param name: The name of the new `GeomPropDef`.
    :type geomprop: str
    :param geomprop: The geometric property to use for the `GeomPropDef`.
    :returns: The new `GeomPropDef`.
)docstring"))

        .def("getGeomPropDef", &mx::Document::getGeomPropDef,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `GeomPropDef`, if any, with the given `name`.
)docstring"))

        .def("getGeomPropDefs", &mx::Document::getGeomPropDefs,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `GeomPropDef` elements in the document.
)docstring"))

        .def("removeGeomPropDef", &mx::Document::removeGeomPropDef,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `GeomPropDef`, if any, with the given `name`.
)docstring"))

        .def("getMaterialOutputs", &mx::Document::getMaterialOutputs,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return material-type outputs for all nodegraphs in the document.
)docstring"))

        .def("addLook", &mx::Document::addLook,
             py::arg("name") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `Look` to the document.

    :type name: str
    :param name: The name of the new `Look`.
        If no name is specified, then a unique name will automatically be
        generated.
    :returns: The new `Look`.
)docstring"))

        .def("getLook", &mx::Document::getLook,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `Look`, if any, with the given `name`.
)docstring"))

        .def("getLooks", &mx::Document::getLooks,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `Look` elements in the document.
)docstring"))

        .def("removeLook", &mx::Document::removeLook,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `Look`, if any, with the given `name`.
)docstring"))

        .def("addLookGroup", &mx::Document::addLookGroup,
             py::arg("name") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `LookGroup` to the document.

    :type name: str
    :param name: The name of the new `LookGroup`.
        If no name is specified, then a unique name will automatically be
        generated.
    :returns: The new `LookGroup`.
)docstring"))

        .def("getLookGroup", &mx::Document::getLookGroup,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `LookGroup`, if any, with the given `name`.
)docstring"))

        .def("getLookGroups", &mx::Document::getLookGroups,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `LookGroup` elements in the document.
)docstring"))

        .def("removeLookGroup", &mx::Document::removeLookGroup,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `LookGroup`, if any, with the given `name`.
)docstring"))

        .def("addCollection", &mx::Document::addCollection,
             py::arg("name") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `Collection` to the document.

    :type name: str
    :param name: The name of the new `Collection`.
        If no name is specified, then a unique name will automatically be
        generated.
    :returns: The new `Collection`.
)docstring"))

        .def("getCollection", &mx::Document::getCollection,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `Collection`, if any, with the given `name`.
)docstring"))

        .def("getCollections", &mx::Document::getCollections,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `Collection` elements in the document.
)docstring"))

        .def("removeCollection", &mx::Document::removeCollection,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `Collection`, if any, with the given `name`.
)docstring"))

        .def("addTypeDef", &mx::Document::addTypeDef,
             py::arg("name") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `TypeDef` to the document.

    :type name: str
    :param name: The name of the new `TypeDef`.
        If no name is specified, then a unique name will automatically be
        generated.
    :returns: The new `TypeDef`.
)docstring"))

        .def("getTypeDef", &mx::Document::getTypeDef,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `TypeDef`, if any, with the given `name`.
)docstring"))

        .def("getTypeDefs", &mx::Document::getTypeDefs,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `TypeDef` elements in the document.
)docstring"))

        .def("removeTypeDef", &mx::Document::removeTypeDef,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `TypeDef`, if any, with the given `name`.
)docstring"))

        .def("addNodeDef", &mx::Document::addNodeDef,
             py::arg("name") = mx::EMPTY_STRING,
             py::arg_v("nodeType",
                       mx::DEFAULT_TYPE_STRING,
                       "mx.DEFAULT_TYPE_STRING"),
             py::arg("node") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `NodeDef` to the document.

    :type name: str
    :param name: The name of the new `NodeDef`.
        If no name is specified, then a unique name will automatically be
        generated.
    :type outputType: str
    :param outputType: An optional output type string.
        If specified, then the new `NodeDef` will be assigned an `Output` of
        the given type.
    :type node: str
    :param node: An optional node string.
    :returns: The new `NodeDef`.
)docstring"))

        .def("addNodeDefFromGraph", &mx::Document::addNodeDefFromGraph,
             py::arg("nodeGraph"),
             py::arg("nodeDefName"),
             py::arg("node"),
             py::arg("version"),
             py::arg("isDefaultVersion"),
             py::arg("nodeGroup"),
             py::arg("newGraphName"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Create a `NodeDef` declaration which is based on a `NodeGraph`.

    :type nodeGraph: `NodeGraph`
    :param nodeGraph: `NodeGraph` used to create the `NodeDef`.
    :type nodeDefName: str
    :param nodeDefName: Declaration name.
    :type node: str
    :param node: Node type for the new declaration.
    :type version: str
    :param version: Version for the new declaration.
    :type isDefaultVersion: bool
    :param isDefaultVersion: Flag that specifies whether the given `version` is
        the default version.
    :type newGraphName: str
    :param newGraphName: Make a copy of this `NodeGraph` with the given `name` if
        a non-empty name is provided. Otherwise, modify the existing `NodeGraph`.
    :type nodeGroup: str
    :param nodeGroup: Optional node group for the new declaration.
    :returns: New declaration if successful.
)docstring"))

        .def("getNodeDef", &mx::Document::getNodeDef,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `NodeDef`, if any, with the given `name`.
)docstring"))

        .def("getNodeDefs", &mx::Document::getNodeDefs,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `NodeDef` elements in the document.
)docstring"))

        .def("removeNodeDef", &mx::Document::removeNodeDef,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `NodeDef`, if any, with the given `name`.
)docstring"))

        .def("getMatchingNodeDefs", &mx::Document::getMatchingNodeDefs,
             py::arg("nodeName"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `NodeDef` elements that match the given node name.
)docstring"))

        .def("addAttributeDef", &mx::Document::addAttributeDef,
             py::arg("name") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Add an `AttributeDef` to the document.

    :type name: str
    :param name: The name of the new `AttributeDef`.
        If no name is specified, then a unique name will automatically be
        generated.
    :returns: The new `AttributeDef`.
)docstring"))

        .def("getAttributeDef", &mx::Document::getAttributeDef,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `AttributeDef`, if any, with the given `name`.
)docstring"))

        .def("getAttributeDefs", &mx::Document::getAttributeDefs,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `AttributeDef` elements in the document.
)docstring"))

        .def("removeAttributeDef", &mx::Document::removeAttributeDef,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `AttributeDef`, if any, with the given `name`.
)docstring"))

        .def("addTargetDef", &mx::Document::addTargetDef,
             py::arg("name") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `TargetDef` to the document.

    :type name: str
    :param name: The name of the new `TargetDef`.
        If no name is specified, then a unique name will automatically be
        generated.
    :returns: The new `TargetDef`.
)docstring"))

        .def("getTargetDef", &mx::Document::getTargetDef,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `TargetDef`, if any, with the given `name`.
)docstring"))

        .def("getTargetDefs", &mx::Document::getTargetDefs,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `TargetDef` elements in the document.
)docstring"))

        .def("removeTargetDef", &mx::Document::removeTargetDef,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `TargetDef`, if any, with the given `name`.
)docstring"))

        .def("addPropertySet", &mx::Document::addPropertySet,
             py::arg("name") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `PropertySet` to the document.

    :type name: str
    :param name: The name of the new `PropertySet`.
        If no name is specified, then a unique name will automatically be
        generated.
    :returns: The new `PropertySet`.
)docstring"))

        .def("getPropertySet", &mx::Document::getPropertySet,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `PropertySet`, if any, with the given `name`.
)docstring"))

        .def("getPropertySets", &mx::Document::getPropertySets,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `PropertySet` elements in the document.
)docstring"))

        .def("removePropertySet", &mx::Document::removePropertySet,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `PropertySet`, if any, with the given `name`.
)docstring"))

        .def("addVariantSet", &mx::Document::addVariantSet,
             py::arg("name") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `VariantSet` to the document.

    :type name: str
    :param name: The name of the new `VariantSet`.
        If no name is specified, then a unique name will automatically be
        generated.
    :returns: The new `VariantSet`.
)docstring"))

        .def("getVariantSet", &mx::Document::getVariantSet,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `VariantSet`, if any, with the given `name`.
)docstring"))

        .def("getVariantSets", &mx::Document::getVariantSets,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `VariantSet` elements in the document.
)docstring"))

        .def("removeVariantSet", &mx::Document::removeVariantSet,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `VariantSet`, if any, with the given `name`.
)docstring"))

        .def("addImplementation", &mx::Document::addImplementation,
             py::arg("name") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Add an `Implementation` to the document.

    :type name: str
    :param name: The name of the new `Implementation`.
        If no name is specified, then a unique name will automatically be
        generated.
    :returns: The new `Implementation`.
)docstring"))

        .def("getImplementation", &mx::Document::getImplementation,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `Implementation`, if any, with the given `name`.
)docstring"))

        .def("getImplementations", &mx::Document::getImplementations,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `Implementation` elements in the document.
)docstring"))

        .def("removeImplementation", &mx::Document::removeImplementation,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `Implementation`, if any, with the given `name`.
)docstring"))

        .def("getMatchingImplementations", &mx::Document::getMatchingImplementations,
             py::arg("nodeDef"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all node implementations that match the given
    `nodeDef` string. Note that a node implementation may be either an
    `Implementation` element or `NodeGraph` element.
)docstring"))

        .def("addUnitDef", &mx::Document::addUnitDef,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `UnitDef` to the document.

    :type name: str
    :param name: The name of the new `UnitDef`.
    :returns: The new `UnitDef`.
    :raises LookupError: If an empty name is given.
)docstring"))

        .def("getUnitDef", &mx::Document::getUnitDef,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `UnitDef`, if any, with the given `name`.
)docstring"))

        .def("getUnitDefs", &mx::Document::getUnitDefs,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `UnitDef` elements in the document.
)docstring"))

        .def("removeUnitDef", &mx::Document::removeUnitDef,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `UnitDef`, if any, with the given `name`.
)docstring"))

        .def("addUnitTypeDef", &mx::Document::addUnitTypeDef,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `UnitTypeDef` to the document.

    :type name: str
    :param name: The name of the new `UnitTypeDef`.
    :returns: The new `UnitTypeDef`.
    :raises LookupError: If an empty name is given.
)docstring"))

        .def("getUnitTypeDef", &mx::Document::getUnitTypeDef,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `UnitTypeDef`, if any, with the given `name`.
)docstring"))

        .def("getUnitTypeDefs", &mx::Document::getUnitTypeDefs,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `UnitTypeDef` elements in the document.
)docstring"))

        .def("removeUnitTypeDef", &mx::Document::removeUnitTypeDef,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `UnitTypeDef`, if any, with the given `name`.
)docstring"))

        .def("upgradeVersion", &mx::Document::upgradeVersion,
             PYMATERIALX_DOCSTRING(R"docstring(
    Upgrade the content of this document from earlier supported versions to
    the library version.

    :see: `getVersionString()`
)docstring"))

        .def("setColorManagementSystem", &mx::Document::setColorManagementSystem,
             py::arg("cms"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the color management system string.
)docstring"))

        .def("hasColorManagementSystem", &mx::Document::hasColorManagementSystem,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if a color management system string has been set.
)docstring"))

        .def("getColorManagementSystem", &mx::Document::getColorManagementSystem,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the color management system string.
)docstring"))

        .def("setColorManagementConfig", &mx::Document::setColorManagementConfig,
             py::arg("cmsConfig"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the color management config string.
)docstring"))

        .def("hasColorManagementConfig", &mx::Document::hasColorManagementConfig,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if a color management config string has been set.
)docstring"))

        .def("getColorManagementConfig", &mx::Document::getColorManagementConfig,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the color management config string.
)docstring"))

        .def_readonly_static("CATEGORY",
                             &mx::Document::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `Document` elements within a MaterialX
    document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .def_readonly_static("CMS_ATTRIBUTE",
                             &mx::Document::CMS_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which a document's color management system string is stored
    as an attribute.

    :see: `setColorManagementSystem()`
    :see: `hasColorManagementSystem()`
    :see: `getColorManagementSystem()`
)docstring"))

        .def_readonly_static("CMS_CONFIG_ATTRIBUTE",
                             &mx::Document::CMS_CONFIG_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which a document's color management config string is stored
    as an attribute.

    :see: `setColorManagementConfig()`
    :see: `hasColorManagementConfig()`
    :see: `getColorManagementConfig()`
)docstring"))

        .doc() = R"docstring(
    Class representing the top-level element in the MaterialX ownership
    hierarchy.

    :see: https://materialx.org/docs/api/class_document.html
)docstring";
}
