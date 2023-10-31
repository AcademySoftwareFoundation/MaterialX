//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Geom.h>
#include <MaterialXCore/Look.h>
#include <MaterialXCore/Material.h>
#include <MaterialXCore/Node.h>
#include <MaterialXCore/Traversal.h>

#define BIND_ELEMENT_FUNC_INSTANCE(T)                                                                           \
.def("_addChild" #T, &mx::Element::addChild<mx::T>)                                                             \
.def("_getChildOfType" #T, &mx::Element::getChildOfType<mx::T>)                                                 \
.def("_getChildrenOfType" #T, &mx::Element::getChildrenOfType<mx::T>, py::arg("category") = mx::EMPTY_STRING)   \
.def("_removeChildOfType" #T, &mx::Element::removeChildOfType<mx::T>)

#define BIND_VALUE_ELEMENT_FUNC_INSTANCE(NAME, T)                                                               \
.def("_setValue" #NAME, &mx::ValueElement::setValue<T>, py::arg("value"), py::arg("type") = mx::EMPTY_STRING)

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyElement(py::module& mod)
{
    py::class_<mx::Element, mx::ElementPtr>(mod, "Element")

        .def(py::self == py::self)

        .def(py::self != py::self)

        .def("setCategory", &mx::Element::setCategory,
             py::arg("category"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the element's category string.

    The category of a MaterialX element represents its role within a `Document`,
    with common examples being `"material"`, `"nodegraph"`, and `"image"`.
)docstring"))

        .def("getCategory", &mx::Element::getCategory,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the element's category string.

    The category of a MaterialX element represents its role within a `Document`,
    with common examples being `"material"`, `"nodegraph"`, and `"image"`.
)docstring"))

        .def("setName", &mx::Element::setName,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the element's name string.

    :attention: The name of a MaterialX element must be unique among all
        elements at the same scope.
    :type name: str
    :param name: The name to set for the element.
    :raises LookupError: If an element at the same scope already possesses the
       given name.
)docstring"))

        .def("getName", &mx::Element::getName,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the element's name string.

    :note: The name of a MaterialX element is unique among all elements at the
        same scope.
)docstring"))

        .def("getNamePath", &mx::Element::getNamePath,
             py::arg("relativeTo") = nullptr,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the element's hierarchical name path, relative to the root document.

    The name of each ancestor will be prepended in turn, separated by forward
    slashes.

    :type relativeTo: Element
    :param relativeTo: If a valid ancestor element is specified, then the
        returned path will be relative to this ancestor.
)docstring"))

        .def("getDescendant", &mx::Element::getDescendant,
             py::arg("namePath"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the element specified by the given hierarchical name path,
    relative to the current element.

    If the name path is empty, then the current element is returned.

    If no element is found at the given path, then `None` is returned.

    :type namePath: str
    :param namePath: The relative name path of the element to return.
)docstring"))

        .def("setFilePrefix", &mx::Element::setFilePrefix,
             py::arg("prefix"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the element's file prefix string.
)docstring"))

        .def("hasFilePrefix", &mx::Element::hasFilePrefix,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has a file prefix string.
)docstring"))

        .def("getFilePrefix", &mx::Element::getFilePrefix,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the element's file prefix string.
)docstring"))

        .def("getActiveFilePrefix", &mx::Element::getActiveFilePrefix,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the file prefix string that is active at the scope of this
    element, taking all ancestor elements into account.
)docstring"))

        .def("setGeomPrefix", &mx::Element::setGeomPrefix,
             py::arg("prefix"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the element's geom prefix string.
)docstring"))

        .def("hasGeomPrefix", &mx::Element::hasGeomPrefix,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has a geom prefix string.
)docstring"))

        .def("getGeomPrefix", &mx::Element::getGeomPrefix,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return this element's geom prefix string.
)docstring"))

        .def("getActiveGeomPrefix", &mx::Element::getActiveGeomPrefix,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the geom prefix string that is active at the scope of this
    element, taking all ancestor elements into account.
)docstring"))

        .def("setColorSpace", &mx::Element::setColorSpace,
             py::arg("colorSpace"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the element's color space string.
)docstring"))

        .def("hasColorSpace", &mx::Element::hasColorSpace,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has a color space string.
)docstring"))

        .def("getColorSpace", &mx::Element::getColorSpace,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the element's color space string.
)docstring"))

        .def("getActiveColorSpace", &mx::Element::getActiveColorSpace,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the color space string that is active at the scope of this
    element, taking all ancestor elements into account.
)docstring"))

        .def("setInheritString", &mx::Element::setInheritString,
             py::arg("inherit"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the inherit string of this element.
)docstring"))

        .def("hasInheritString", &mx::Element::hasInheritString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has an inherit string.
)docstring"))

        .def("getInheritString", &mx::Element::getInheritString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the inherit string of this element.
)docstring"))

        .def("setInheritsFrom", &mx::Element::setInheritsFrom,
             py::arg("ancestor"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the element that this one directly inherits from.

    Sets the inherit string of this element to the name of the given ancestor
    element.

    If no ancestor element is given, then the inherit attribute is removed from
    this element.
)docstring"))

        .def("getInheritsFrom", &mx::Element::getInheritsFrom,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the element, if any, that this one directly inherits from.
)docstring"))

        .def("hasInheritedBase", &mx::Element::hasInheritedBase,
             py::arg("base"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has the given `base` element as an inherited
    base, taking the full inheritance chain into account.
)docstring"))

        .def("hasInheritanceCycle", &mx::Element::hasInheritanceCycle,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if the inheritance chain for this element contains a cycle.
)docstring"))

        .def("setNamespace", &mx::Element::setNamespace,
             py::arg("namespace"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the namespace string of this element.
)docstring"))

        .def("hasNamespace", &mx::Element::hasNamespace,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has a namespace string.
)docstring"))

        .def("getNamespace", &mx::Element::getNamespace,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the namespace string of this element.
)docstring"))

        .def("getQualifiedName", &mx::Element::getQualifiedName,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a qualified version of the given `name`, taking the namespace at the
    scope of this element into account.
)docstring"))

        .def("setDocString", &mx::Element::setDocString,
             py::arg("doc"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the documentation string of this element.
)docstring"))

        .def("getDocString", &mx::Element::getDocString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the documentation string of this element
)docstring"))

        .def("addChildOfCategory", &mx::Element::addChildOfCategory,
             py::arg("category"),
             py::arg("name") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a child element of the given category and name.

    :type category: str
    :param category: The category string of the new child element.
        If the category string is recognized, then the correponding `Element`
        subclass is generated; otherwise, a `GenericElement` is generated.
    :type name: str
    :param name: The name of the new child element.
        If no name is specified, then a unique name will automatically be
        generated.
    :raises LookupError: If a child of this element already possesses the given
        `name`.
    :returns: The new child element.
)docstring"))

        .def("changeChildCategory", &mx::Element::changeChildCategory,
             py::arg("child"),
             py::arg("category"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Change the category of the given `child` element.

    :type child: Element
    :param child: The child element that will be modified.
    :type category: str
    :param category: The new category string for the child element.
    :returns: A new child element, containing the contents of the original
        child, but with a new category and subclass.
)docstring"))

        .def("getChild", &mx::Element::getChild,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the child element, if any, with the given `name`.

    :type name: str | None
    :param name: The name of the child element to return.
        If `name` is `None`, then `None` is returned.
)docstring"))

        .def("getChildren", &mx::Element::getChildren,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all child elements in the order in which they were added.
)docstring"))

        .def("setChildIndex", &mx::Element::setChildIndex,
             py::arg("name"),
             py::arg("index"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the index of the child, if any, with the given `name`.

    :raises LookupError: If the child is found and the given `index` is out of
        bounds.
)docstring"))

        .def("getChildIndex", &mx::Element::getChildIndex,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the index of the child, if any, with the given `name`.

    If no child with the given `name` is found, then `-1` is returned.
)docstring"))

        .def("removeChild", &mx::Element::removeChild,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the child element, if any, with the given `name`.
)docstring"))

        .def("setAttribute", &mx::Element::setAttribute,
             py::arg("attrib"),
             py::arg("value"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the value string of the given attribute.
)docstring"))

        .def("hasAttribute", &mx::Element::hasAttribute,
             py::arg("attrib"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if the specified attribute is present.
)docstring"))

        .def("getAttribute", &mx::Element::getAttribute,
             py::arg("attrib"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the value string of the specified attribute.

    If the specified attribute is not present, then an empty string is returned.
)docstring"))

        .def("getAttributeNames", &mx::Element::getAttributeNames,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of stored attribute names, in the order they were set.
)docstring"))

        .def("removeAttribute", &mx::Element::removeAttribute,
             py::arg("attrib"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the specified attribute, if present.
)docstring"))

        .def("getSelf", static_cast<mx::ElementPtr (mx::Element::*)()>(&mx::Element::getSelf),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return this element.

    >>> element.getSelf() is element
    True
)docstring"))

        .def("getParent", static_cast<mx::ElementPtr(mx::Element::*)()>(&mx::Element::getParent),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return our parent element.
)docstring"))

        .def("getRoot", static_cast<mx::ElementPtr(mx::Element::*)()>(&mx::Element::getRoot),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the root element of our tree.
)docstring"))

        .def("getDocument", static_cast<mx::DocumentPtr(mx::Element::*)()>(&mx::Element::getDocument),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the root document of our tree.
)docstring"))

        .def("traverseTree", &mx::Element::traverseTree,
             PYMATERIALX_DOCSTRING(R"docstring(
    Traverse the tree from this element to each of its descendants in
    depth-first order, using pre-order visitation.

    >>> import MaterialX as mx
    >>> document = mx.createDocument()
    >>> for i in range(3):
    ...     document.addBackdrop()
    ... 
    <MaterialX.PyMaterialXCore.Backdrop object at 0x100bf4970>
    <MaterialX.PyMaterialXCore.Backdrop object at 0x100bfdc70>
    <MaterialX.PyMaterialXCore.Backdrop object at 0x100bf4970>
    >>> for element in document.traverseTree():
    ...     print(element)
    ... 
    <materialx version="1.38">
    <backdrop name="backdrop1">
    <backdrop name="backdrop2">
    <backdrop name="backdrop3">

    :returns: A `TreeIterator` object.
)docstring"))

        .def("traverseGraph", &mx::Element::traverseGraph,
             PYMATERIALX_DOCSTRING(R"docstring(
    Traverse the dataflow graph from this element to each of its upstream
    sources in depth-first order, using pre-order visitation.

    Example usage:

    .. code:: python

        for edge in inputElem.traverseGraph():
            upElem = edge.getUpstreamElement()
            downElem = edge.getDownstreamElement()
            print("{} lies upstream from {}".format(upElem, downElem))

    :returns: A `GraphIterator` object.
    :raises ExceptionFoundCycle: If a cycle is encountered.
    :see: `Element.getUpstreamEdge()`
    :see: `Element.getUpstreamElement()`
)docstring"))

        .def("getUpstreamEdge", &mx::Element::getUpstreamEdge,
             py::arg("index") = 0,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `Edge` with the given `index` that lies directly upstream from
    this element in the dataflow graph.

    :type index: int
    :param index: An optional index of the edge to be returned, where the
        valid index range may be determined with `getUpstreamEdgeCount()`.
    :returns: The upstream `Edge`, if valid, or an empty `Edge` object.
)docstring"))

        .def("getUpstreamEdgeCount", &mx::Element::getUpstreamEdgeCount,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the number of queriable upstream edges for this element.
)docstring"))

        .def("getUpstreamElement", &mx::Element::getUpstreamElement,
             py::arg("index") = 0,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `Element` with the given `index` that lies directly upstream
    from this one in the dataflow graph.

    :type index: int
    :param index: An optional index of the element to be returned, where the
        valid index range may be determined with `getUpstreamEdgeCount()`.
    :returns: The upstream `Element`, if valid, or `None`.
)docstring"))

        .def("traverseInheritance", &mx::Element::traverseInheritance,
             PYMATERIALX_DOCSTRING(R"docstring(
    Traverse the inheritance chain from this element to each element from which
    it inherits.

    Example usage:

    .. code:: python

        derivedElem = None
        for elem in inputElem.traverseInheritance():
            if derivedElem:
                print("{} inherits from {}".format(derivedElem, elem))
            derivedElem = elem

    :returns: An `InheritanceIterator` object.
    :raises ExceptionFoundCycle: If a cycle is encountered.
)docstring"))

        .def("setSourceUri", &mx::Element::setSourceUri,
             py::arg("sourceUri"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the element's source URI.

    :param sourceUri: A URI string representing the resource from which this
        element originates. This string may be used by serialization and
        deserialization routines to maintain hierarchies of include references.
)docstring"))

        .def("hasSourceUri", &mx::Element::hasSourceUri,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has a source URI.
)docstring"))

        .def("getSourceUri", &mx::Element::getSourceUri,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the element's source URI.
)docstring"))

        .def("getActiveSourceUri", &mx::Element::getActiveSourceUri,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the source URI that is active at the scope of this element, taking
    all ancestor elements into account.
)docstring"))

        .def("validate",
             [](const mx::Element& elem)
             {
                 std::string message;
                 bool res = elem.validate(&message);
                 return std::pair<bool, std::string>(res, message);
             },
             PYMATERIALX_DOCSTRING(R"docstring(
    Validate that this element tree, including all descendants, is consistent
    with the MaterialX specification.

    :returns: A `tuple` with a boolean flag indicating whether the element tree
        is valid, and a message string detailing what might be wrong with it.
)docstring"))

        .def("copyContentFrom", &mx::Element::copyContentFrom,
             py::arg("source"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Copy all attributes and descendants from the given `source` element to this
    one.

    :param source: The element from which to copy content.
)docstring"))

        .def("clearContent", &mx::Element::clearContent,
             PYMATERIALX_DOCSTRING(R"docstring(
    Clear all attributes and descendants from this element.
)docstring"))

        .def("createValidChildName", &mx::Element::createValidChildName,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Using the input name as a starting point, modify it to create a valid,
    unique name for a child element.
)docstring"))

        .def("createStringResolver", &mx::Element::createStringResolver,
             py::arg("geom") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Construct a `StringResolver` at the scope of this element.

    The returned object may be used to apply substring modifiers to data values
    in the context of a specific element, geometry, and material.

    :type geom: str
    :param geom: An optional geometry name, which will be used to select the
        applicable set of geometry token substitutions.

        By default, no geometry token substitutions are applied.

        If the universal geometry name `"/"` is given, then all geometry token
        substitutions are applied,
    :returns: A `StringResolver`.
)docstring"))

        .def("asString", &mx::Element::asString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a single-line description of this element, including its category,
    name, and attributes.

    :note: More pythonic alternative: `str(element)` equals `element.toString()`
)docstring"))
        .def("__str__", &mx::Element::asString)

        BIND_ELEMENT_FUNC_INSTANCE(Collection)
        BIND_ELEMENT_FUNC_INSTANCE(Document)
        BIND_ELEMENT_FUNC_INSTANCE(GeomInfo)
        BIND_ELEMENT_FUNC_INSTANCE(GeomProp)
        BIND_ELEMENT_FUNC_INSTANCE(Implementation)
        BIND_ELEMENT_FUNC_INSTANCE(Look)
        BIND_ELEMENT_FUNC_INSTANCE(MaterialAssign)
        BIND_ELEMENT_FUNC_INSTANCE(Node)
        BIND_ELEMENT_FUNC_INSTANCE(NodeDef)
        BIND_ELEMENT_FUNC_INSTANCE(NodeGraph)
        BIND_ELEMENT_FUNC_INSTANCE(Property)
        BIND_ELEMENT_FUNC_INSTANCE(PropertySet)
        BIND_ELEMENT_FUNC_INSTANCE(PropertySetAssign)
        BIND_ELEMENT_FUNC_INSTANCE(Token)
        BIND_ELEMENT_FUNC_INSTANCE(TypeDef)
        BIND_ELEMENT_FUNC_INSTANCE(Visibility)

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Base class for MaterialX elements.

    An `Element` is a named object within a `Document`, which may possess any
    number of child elements and attributes.

    Inherited by: `TypedElement`, `GeomElement`, `Backdrop`, `Collection`,
    `CommentElement`, `GenericElement`, `Look`, `LookGroup`, `NewlineElement`,
    `PropertySet`, `TypeDef`, `Unit`, `UnitDef`, `UnitTypeDef`, `VariantSet`,
    and `VariantAssign`.

    :see: https://materialx.org/docs/api/class_element.html
)docstring");

    py::class_<mx::TypedElement, mx::TypedElementPtr, mx::Element>(mod, "TypedElement")

        .def("setType", &mx::TypedElement::setType,
             py::arg("elementType"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the element's type string.
)docstring"))

        .def("hasType", &mx::TypedElement::hasType,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has a type string.
)docstring"))

        .def("getType", &mx::TypedElement::getType,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return this element's type string.
)docstring"))

        .def("isMultiOutputType", &mx::TypedElement::isMultiOutputType,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element is of multi-output type.
)docstring"))

        .def("getTypeDef", &mx::TypedElement::getTypeDef,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `TypeDef` declaring the type string of this element. If no
    matching `TypeDef` is found, then `None` is returned.
)docstring"))

        .def_readonly_static("TYPE_ATTRIBUTE", &mx::TypedElement::TYPE_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's type string is stored as an attribute.

    :see: `setType()`
    :see: `hasType()`
    :see: `getType()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Base class for typed elements.

    Inherited by: `InterfaceElement`, `ValueElement`, `AttributeDef`,
    `GeomPropDef`, `Member`, and `TargetDef`.

    :see: https://materialx.org/docs/api/class_typed_element.html
)docstring");

    py::class_<mx::ValueElement, mx::ValueElementPtr, mx::TypedElement>(mod, "ValueElement")

        .def("setValueString", &mx::ValueElement::setValueString,
             py::arg("value"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the value string of an element.
)docstring"))

        .def("hasValueString", &mx::ValueElement::hasValueString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has a value string.
)docstring"))

        .def("getValueString", &mx::ValueElement::getValueString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Get the value string of this element.
)docstring"))

        .def("getResolvedValueString", &mx::ValueElement::getResolvedValueString,
             py::arg("resolver") = nullptr,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the resolved value string of this element, applying any string
    substitutions that are defined at this element's scope.

    :type resolver: StringResolver
    :param resolver: An optional string resolver, which will be used to
        apply string substitutions. By default, a new string resolver
        will be created at this scope and applied to the return value.
)docstring"))

        .def("setInterfaceName", &mx::ValueElement::setInterfaceName,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the interface name of this element.
)docstring"))

        .def("hasInterfaceName", &mx::ValueElement::hasInterfaceName,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has an interface name.
)docstring"))

        .def("getInterfaceName", &mx::ValueElement::getInterfaceName,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the interface name of this element.
)docstring"))

        .def("setImplementationName", &mx::ValueElement::setImplementationName,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the implementation name of this element.
)docstring"))

        .def("hasImplementationName", &mx::ValueElement::hasImplementationName,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has an implementation name.
)docstring"))

        .def("getImplementationName", &mx::ValueElement::getImplementationName,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the implementation name of this element.
)docstring"))

        .def("_getValue", &mx::ValueElement::getValue,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the typed value of this element as a generic `Value` object, which
    may be queried to access its data.

    :returns: The typed value of this element, or `None` if no value is present.
)docstring"))

        .def("_getDefaultValue", &mx::ValueElement::getDefaultValue,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the default value for this element as a generic `Value` object,
    which may be queried to access its data.

    :returns: A typed default value, or `None` if no default value was found.
)docstring"))

        .def("setUnit", &mx::ValueElement::setUnit,
             py::arg("unit"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the unit string of this element.
)docstring"))

        .def("hasUnit", &mx::ValueElement::hasUnit,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has a unit string.
)docstring"))

        .def("getUnit", &mx::ValueElement::getUnit,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the unit string of this element.
)docstring"))

        .def("getActiveUnit", &mx::ValueElement::getActiveUnit,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the unit defined by the associated `NodeDef` if this element
    is a child of a `Node`.
)docstring"))

        .def("setUnitType", &mx::ValueElement::setUnitType,
             py::arg("unitType"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the unit type of this element.
)docstring"))

        .def("hasUnitType", &mx::ValueElement::hasUnitType,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has a unit type.
)docstring"))

        .def("getUnitType", &mx::ValueElement::getUnitType,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the unit type of this element.
)docstring"))

        .def("getIsUniform", &mx::ValueElement::getIsUniform,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the uniform attribute flag for this element.
)docstring"))

        .def("setIsUniform", &mx::ValueElement::setIsUniform,
             py::arg("value"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the uniform attribute flag on this element.
)docstring"))

        .def_readonly_static("VALUE_ATTRIBUTE", &mx::ValueElement::VALUE_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's value string is stored as an attribute.

    :see: `setValueString()`
    :see: `hasValueString()`
    :see: `getValueString()`
)docstring"))

        .def_readonly_static("INTERFACE_NAME_ATTRIBUTE", &mx::ValueElement::INTERFACE_NAME_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's interface name is stored as an attribute.

    :see: `setInterfaceName()`
    :see: `hasInterfaceName()`
    :see: `getInterfaceName()`
)docstring"))

        .def_readonly_static("IMPLEMENTATION_NAME_ATTRIBUTE", &mx::ValueElement::IMPLEMENTATION_NAME_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's implementation name is stored as an
    attribute.

    :see: `setImplementationName()`
    :see: `hasImplementationName()`
    :see: `getImplementationName()`
)docstring"))

        .def_readonly_static("IMPLEMENTATION_TYPE_ATTRIBUTE", &mx::ValueElement::IMPLEMENTATION_TYPE_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's implementation type can be stored as an
    attribute.
)docstring"))

        .def_readonly_static("ENUM_ATTRIBUTE", &mx::ValueElement::ENUM_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which names of enums can be stored in an attribute for an
    element. Expected to correspond to values of enums as set in the
    `ENUM_VALUES_ATTRIBUTE` attribute.

    The enum list can be thought of as a list of commonly-used values or UI
    labels for an input.
)docstring"))

        .def_readonly_static("ENUM_VALUES_ATTRIBUTE", &mx::ValueElement::ENUM_VALUES_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which values of enums can be stored in an attribute for an
    element. Expected to correspond to names of enums as set in the
    `ENUM_ATTRIBUTE` attribute.

    The enum list can be thought of as a list of commonly-used values or UI
    labels for an input.
)docstring"))

        .def_readonly_static("UNIT_ATTRIBUTE", &mx::ValueElement::UNIT_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's unit string is stored as an attribute.

    :see: `setUnit()`
    :see: `hasUnit()`
    :see: `getUnit()`
)docstring"))

        .def_readonly_static("UNITTYPE_ATTRIBUTE", &mx::ValueElement::UNITTYPE_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's unit type is stored as an attribute.

    :see: `setUnitType()`
    :see: `hasUnitType()`
    :see: `getUnitType()`
)docstring"))

        .def_readonly_static("UNIFORM_ATTRIBUTE", &mx::ValueElement::UNIFORM_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's uniform flag is stored as an attribute.

    :see: `setIsUniform()`
    :see: `getIsUniform()`
)docstring"))

        .def_readonly_static("UI_NAME_ATTRIBUTE", &mx::ValueElement::UI_NAME_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's name can be stored as an attribute for
    use in UIs.
)docstring"))

        .def_readonly_static("UI_FOLDER_ATTRIBUTE", &mx::ValueElement::UI_FOLDER_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's folder can be stored as an attribute for
    use in UIs.
)docstring"))

        .def_readonly_static("UI_MIN_ATTRIBUTE", &mx::ValueElement::UI_MIN_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's minimum value can be stored as an
    attribute for use in UIs.
)docstring"))

        .def_readonly_static("UI_MAX_ATTRIBUTE", &mx::ValueElement::UI_MAX_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's maximum value can be stored as an
    attribute for use in UIs.
)docstring"))

        .def_readonly_static("UI_SOFT_MIN_ATTRIBUTE", &mx::ValueElement::UI_SOFT_MIN_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's soft minimum value can be stored as an
    attribute for use in UIs.
)docstring"))

        .def_readonly_static("UI_SOFT_MAX_ATTRIBUTE", &mx::ValueElement::UI_SOFT_MAX_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's soft maximum value can be stored as an
    attribute for use in UIs.
)docstring"))

        .def_readonly_static("UI_STEP_ATTRIBUTE", &mx::ValueElement::UI_STEP_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's incremental step value can be stored as
    an attribute for use in UIs.
)docstring"))

        .def_readonly_static("UI_ADVANCED_ATTRIBUTE", &mx::ValueElement::UI_ADVANCED_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's advanced flag can be stored as an
    attribute for use in UIs.
)docstring"))

        BIND_VALUE_ELEMENT_FUNC_INSTANCE(integer, int)
        BIND_VALUE_ELEMENT_FUNC_INSTANCE(boolean, bool)
        BIND_VALUE_ELEMENT_FUNC_INSTANCE(float, float)
        BIND_VALUE_ELEMENT_FUNC_INSTANCE(color3, mx::Color3)
        BIND_VALUE_ELEMENT_FUNC_INSTANCE(color4, mx::Color4)
        BIND_VALUE_ELEMENT_FUNC_INSTANCE(vector2, mx::Vector2)
        BIND_VALUE_ELEMENT_FUNC_INSTANCE(vector3, mx::Vector3)
        BIND_VALUE_ELEMENT_FUNC_INSTANCE(vector4, mx::Vector4)
        BIND_VALUE_ELEMENT_FUNC_INSTANCE(matrix33, mx::Matrix33)
        BIND_VALUE_ELEMENT_FUNC_INSTANCE(matrix44, mx::Matrix44)
        BIND_VALUE_ELEMENT_FUNC_INSTANCE(string, std::string)
        BIND_VALUE_ELEMENT_FUNC_INSTANCE(integerarray, mx::IntVec)
        BIND_VALUE_ELEMENT_FUNC_INSTANCE(booleanarray, mx::BoolVec)
        BIND_VALUE_ELEMENT_FUNC_INSTANCE(floatarray, mx::FloatVec)
        BIND_VALUE_ELEMENT_FUNC_INSTANCE(stringarray, mx::StringVec)

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Base class for elements that support typed values.

    Inherited by: `PortElement`, `GeomProp`, `Property`, `PropertyAssign`, and
    `Token`.

    :see: https://materialx.org/docs/api/class_value_element.html
)docstring");

    py::class_<mx::Token, mx::TokenPtr, mx::ValueElement>(mod, "Token")

        .def_readonly_static("CATEGORY", &mx::Token::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `Token` elements within a MaterialX
    document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    A token element representing a string value.

    Token elements are used to define input and output values for string
    substitutions in image filenames.

    :see: https://materialx.org/docs/api/class_token.html
)docstring");

    py::class_<mx::CommentElement, mx::CommentElementPtr, mx::Element>(mod, "CommentElement")

        .def_readonly_static("CATEGORY", &mx::CommentElement::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `CommentElement` elements within a
    MaterialX document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a block of descriptive text within a `Document`,
    which will be stored a comment when the document is written out.

    The comment text may be accessed with the methods `Element.getDocString()`
    and `Element.setDocString()`.

    :see: https://materialx.org/docs/api/class_comment_element.html
)docstring");

    py::class_<mx::NewlineElement, mx::NewlineElementPtr, mx::Element>(mod, "NewlineElement")

        .def_readonly_static("CATEGORY", &mx::NewlineElement::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `NewlineElement` elements within a
    MaterialX document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a newline within a `Document`.

    :see: https://materialx.org/docs/api/class_newline_element.html
)docstring");

    py::class_<mx::GenericElement, mx::GenericElementPtr, mx::Element>(mod, "GenericElement")

        .def_readonly_static("CATEGORY", &mx::GenericElement::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `GenericElement` elements within a
    MaterialX document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    A generic element subclass, for instantiating elements with unrecognized
    categories.

    :see: https://materialx.org/docs/api/class_generic_element.html
)docstring");

    py::class_<mx::StringResolver, mx::StringResolverPtr>(mod, "StringResolver")

        .def("setFilePrefix", &mx::StringResolver::setFilePrefix,
             py::arg("prefix"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set this element's file prefix string.
)docstring"))

        .def("getFilePrefix", &mx::StringResolver::getFilePrefix,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return this element's file prefix string.
)docstring"))

        .def("setGeomPrefix", &mx::StringResolver::setGeomPrefix,
             py::arg("prefix"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set this element's geom prefix string.
)docstring"))

        .def("getGeomPrefix", &mx::StringResolver::getGeomPrefix,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return this element's geom prefix string.
)docstring"))

        .def("setUdimString", &mx::StringResolver::setUdimString,
             py::arg("udim"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the UDIM substring substitution for filename data values.
    This string will be used to replace the standard `<UDIM>` token.
)docstring"))

        .def("setUvTileString", &mx::StringResolver::setUvTileString,
             py::arg("uvTile"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the UV-tile substring substitution for filename data values.
    This string will be used to replace the standard `<UVTILE>` token.
)docstring"))

        .def("setFilenameSubstitution", &mx::StringResolver::setFilenameSubstitution,
             py::arg("key"),
             py::arg("value"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set an arbitrary substring substitution for filename data values.
)docstring"))

        .def("getFilenameSubstitutions", &mx::StringResolver::getFilenameSubstitutions,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the map of filename substring substitutions.
)docstring"))

        .def("setGeomNameSubstitution", &mx::StringResolver::setGeomNameSubstitution,
             py::arg("key"),
             py::arg("value"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set an arbitrary substring substitution for geometry name data values.
)docstring"))

        .def("getGeomNameSubstitutions", &mx::StringResolver::getGeomNameSubstitutions,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the map of geometry name substring substitutions.
)docstring"))

        .def("resolve", &mx::StringResolver::resolve,
             py::arg("input"),
             py::arg("inputType"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Given an `input` string and type, apply all appropriate modifiers, and
    return the resulting string.

    The given `inputType` is expected to be either `FILENAME_TYPE_STRING` or
    `GEOMNAME_TYPE_STRING`. If it is neither, the given `input` string is
    returned as-is.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    A helper class for applying string modifiers to data values in the context
    of a specific element and geometry.

    A `StringResolver` may be constructed through the `Element.createStringResolver()`
    method, which initializes it in the context of a specific `Element`, geometry,
    and material.

    Calling the `StringResolver.resolve()` method applies all modifiers to a
    particular string value.

    Methods such as `StringResolver.setFilePrefix()` may be used to edit the
    stored string modifiers before calling `StringResolver.resolve()`.

    :see: https://materialx.org/docs/api/class_string_resolver.html
)docstring");

    py::class_<mx::ElementPredicate>(mod, "ElementPredicate")

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a function that takes an `Element` and returns a `bool`,
    to check whether some criteria has passed.
)docstring");

    py::register_exception<mx::ExceptionOrphanedElement>(mod, "ExceptionOrphanedElement")

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    A type of exception that is raised when an `Element` is used after its owning
    `Document` has gone out of scope.

    :see: https://materialx.org/docs/api/class_exception_orphaned_element.html
)docstring");

    mod.def("targetStringsMatch", &mx::targetStringsMatch,
            py::arg("target1"),
            py::arg("target2"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Given two target strings, each containing a string array of target names,
    return `True` if they have any targets in common.

    An empty target string matches all targets.
)docstring"));

    mod.def("prettyPrint", &mx::prettyPrint,
            py::arg("element"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Pretty-print the given `element` tree, calling `Element.asString()`
    recursively on each element in depth-first order.
)docstring"));
}
