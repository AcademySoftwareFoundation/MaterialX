//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Interface.h>

#include <MaterialXCore/Node.h>

namespace py = pybind11;
namespace mx = MaterialX;

#define BIND_INTERFACE_TYPE_INSTANCE(NAME, T)                                       \
.def("_setInputValue" #NAME, &mx::InterfaceElement::setInputValue<T>,               \
     py::arg("name"),                                                               \
     py::arg("value"),                                                              \
     py::arg("inputType") = mx::EMPTY_STRING,                                       \
     "Set the typed `value` of an input by its `name`, creating a child element\n"  \
     "to hold the input if needed.")

void bindPyInterface(py::module& mod)
{
    py::class_<mx::PortElement, mx::PortElementPtr, mx::ValueElement>(mod, "PortElement")

        .def("setNodeName", &mx::PortElement::setNodeName,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the node name string of this element, creating a connection to
    the `Node` with the given `name` within the same `NodeGraph`.
)docstring"))

        .def("getNodeName", &mx::PortElement::getNodeName,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the node name string of this element.
)docstring"))

        .def("setNodeGraphString", &mx::PortElement::setNodeGraphString,
             py::arg("nodeGraph"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the node graph string of this element.
)docstring"))

        .def("hasNodeGraphString", &mx::PortElement::hasNodeGraphString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has a node graph string.
)docstring"))

        .def("getNodeGraphString", &mx::PortElement::getNodeGraphString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the node graph string of this element.
)docstring"))

        .def("setOutputString", &mx::PortElement::setOutputString,
             py::arg("output"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the output string of this element.
)docstring"))

        .def("hasOutputString", &mx::PortElement::hasOutputString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has an output string.
)docstring"))

        .def("getOutputString", &mx::PortElement::getOutputString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the output string of this element.
)docstring"))

        .def("setChannels", &mx::PortElement::setChannels,
             py::arg("channels"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the channels string of this element, defining a channel swizzle
    that will be applied to the upstream result if this port is connected.
)docstring"))

        .def("getChannels", &mx::PortElement::getChannels,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the channels string of this element.
)docstring"))

        .def("setConnectedNode", &mx::PortElement::setConnectedNode,
             py::arg("node"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the node to which this element is connected. The given `node` must
    belong to the same node graph. If the `node` argument is `None`, then
    any existing node connection will be cleared.
)docstring"))

        .def("getConnectedNode", &mx::PortElement::getConnectedNode,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the node, if any, to which this element is connected.
)docstring"))

        .def("setConnectedOutput", &mx::PortElement::setConnectedOutput,
             py::arg("output"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the `Output` to which this `Input` is connected. If the `output`
    argument is `None`, then any existing output connection will be cleared.
)docstring"))

        .def("getConnectedOutput", &mx::PortElement::getConnectedOutput,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `Output`, if any, to which this `Input` is connected.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Base class for port elements.

    Port elements support spatially-varying upstream connections to nodes.

    Inherited by: `Input` and `Output`.

    :see: https://materialx.org/docs/api/class_port_element.html
)docstring");

    py::class_<mx::Input, mx::InputPtr, mx::PortElement>(mod, "Input")

        .def("setDefaultGeomPropString", &mx::Input::setDefaultGeomPropString,
             py::arg("geomprop"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the defaultgeomprop string for this input.
)docstring"))

        .def("hasDefaultGeomPropString", &mx::Input::hasDefaultGeomPropString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this input has a defaultgeomprop string.
)docstring"))

        .def("getDefaultGeomPropString", &mx::Input::getDefaultGeomPropString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the defaultgeomprop string for this input.
)docstring"))

        .def("getDefaultGeomProp", &mx::Input::getDefaultGeomProp,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `GeomPropDef` element to use, if defined for this input.
)docstring"))

        .def("getConnectedNode", &mx::Input::getConnectedNode,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the node, if any, to which this element is connected.
)docstring"))

        .def("getInterfaceInput", &mx::Input::getInterfaceInput,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the input on the parent graph corresponding to the interface name
    for this input.
)docstring"))

        .def_readonly_static("CATEGORY", &mx::Input::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `Input` elements within a MaterialX
    document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing an input element within a `Node` or `NodeDef`.

    An `Input` holds either a uniform value or a connection to a spatially-varying
    `Output`, either of which may be modified within the scope of a `Material`.

    :see: https://materialx.org/docs/api/class_input.html
)docstring");

    py::class_<mx::Output, mx::OutputPtr, mx::PortElement>(mod, "Output")

        .def("hasUpstreamCycle", &mx::Output::hasUpstreamCycle,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if a cycle exists in any upstream path from this element.
)docstring"))

        .def_readonly_static("CATEGORY", &mx::Output::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `Output` elements within a MaterialX
    document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))
        .def_readonly_static("DEFAULT_INPUT_ATTRIBUTE", &mx::Output::DEFAULT_INPUT_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which the name of an output port's default input is stored
    as an attribute.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a spatially-varying output element within a `NodeGraph`
    or `NodeDef`.

    :see: https://materialx.org/docs/api/class_output.html
)docstring");

    py::class_<mx::InterfaceElement, mx::InterfaceElementPtr, mx::TypedElement>(mod, "InterfaceElement")

        .def("setNodeDefString", &mx::InterfaceElement::setNodeDefString,
             py::arg("nodeDef"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the NodeDef string for the interface.
)docstring"))

        .def("hasNodeDefString", &mx::InterfaceElement::hasNodeDefString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this interface has a NodeDef string.
)docstring"))

        .def("getNodeDefString", &mx::InterfaceElement::getNodeDefString,
             PYMATERIALX_DOCSTRING(R"docstring(
     Return the NodeDef string for the interface.
)docstring"))

        .def("addInput", &mx::InterfaceElement::addInput,
             py::arg("name") = mx::EMPTY_STRING,
             py::arg_v("inputType",
                       mx::DEFAULT_TYPE_STRING,
                       "mx.DEFAULT_TYPE_STRING"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Add an `Input` to this interface.

    :type name: str
    :param name: The name of the new `Input`.
        If no name is specified, then a unique name will automatically be
        generated.
    :type inputType: str
    :param inputType: An optional type string.
    :returns: The new `Input`.
    :see: `DEFAULT_TYPE_STRING`
)docstring"))

        .def("getInput", &mx::InterfaceElement::getInput,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `Input`, if any, with the given `name`.
)docstring"))

        .def("getInputs", &mx::InterfaceElement::getInputs,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `Input` elements.
)docstring"))

        .def("getInputCount", &mx::InterfaceElement::getInputCount,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the number of `Input` elements.
)docstring"))

        .def("removeInput", &mx::InterfaceElement::removeInput,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `Input`, if any, with the given `name`.
)docstring"))

        .def("getActiveInput", &mx::InterfaceElement::getActiveInput,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the first `Input` with the given `name` that belongs to this
    interface, taking interface inheritance into account.
)docstring"))

        .def("getActiveInputs", &mx::InterfaceElement::getActiveInputs,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `Input` elements that belong to this interface,
    taking inheritance into account.
)docstring"))

        .def("addOutput", &mx::InterfaceElement::addOutput,
             py::arg("name") = mx::EMPTY_STRING,
             py::arg_v("outputType",
                       mx::DEFAULT_TYPE_STRING,
                       "mx.DEFAULT_TYPE_STRING"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Add an `Output` to this interface.

    :type name: str
    :param name: The name of the new `Output`.
        If no name is specified, then a unique name will automatically be
        generated.
    :type outputType: str
    :param outputType: An optional type string.
    :returns: The new `Output`.
    :see: `DEFAULT_TYPE_STRING`
)docstring"))

        .def("getOutput", &mx::InterfaceElement::getOutput,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `Output`, if any, with the given `name`.
)docstring"))

        .def("getOutputs", &mx::InterfaceElement::getOutputs,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `Output` elements.
)docstring"))

        .def("getOutputCount", &mx::InterfaceElement::getOutputCount,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the number of `Output` elements.
)docstring"))

        .def("removeOutput", &mx::InterfaceElement::removeOutput,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `Output`, if any, with the given `name`.
)docstring"))

        .def("getActiveOutput", &mx::InterfaceElement::getActiveOutput,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the first `Output` with the given `name` that belongs to this
    interface, taking interface inheritance into account.
)docstring"))

        .def("getActiveOutputs", &mx::InterfaceElement::getActiveOutputs,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `Output` elements that belong to this interface,
    taking inheritance into account.
)docstring"))

        .def("setConnectedOutput", &mx::InterfaceElement::setConnectedOutput,
             py::arg("inputName"),
             py::arg("output"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the `Output` to which the specified `Input` is connected, creating a
    child `Input` if needed. If the `output` argument is `None`, then any
    existing output connection on the input will be cleared.
)docstring"))

        .def("getConnectedOutput", &mx::InterfaceElement::getConnectedOutput,
             py::arg("inputName"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `Output` connected to the specified input. If the specified
    input is not present, then `None` is returned.
)docstring"))

        .def("addToken", &mx::InterfaceElement::addToken,
             py::arg("name") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `Token` to this interface.

    :type name: str
    :param name: The name of the new `Token`.
        If no name is specified, then a unique name will automatically be
        generated.
    :returns: The new `Token`.
)docstring"))

        .def("getToken", &mx::InterfaceElement::getToken,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `Token`, if any, with the given `name`.
)docstring"))

        .def("getTokens", &mx::InterfaceElement::getTokens,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `Token` elements.
)docstring"))

        .def("removeToken", &mx::InterfaceElement::removeToken,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `Token`, if any, with the given `name`.
)docstring"))

        .def("getActiveToken", &mx::InterfaceElement::getActiveToken,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the first `Token` with the given `name` that belongs to this
    interface, taking interface inheritance into account.
)docstring"))

        .def("getActiveTokens", &mx::InterfaceElement::getActiveTokens,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `Token` elements that belong to this interface,
    taking inheritance into account.
)docstring"))

        .def("getActiveValueElement", &mx::InterfaceElement::getActiveValueElement,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the first value element with the given `name` that belongs to this
    interface, taking interface inheritance into account.

    Examples of value elements are `Input`, `Output`, and `Token`.
)docstring"))

        .def("getActiveValueElements", &mx::InterfaceElement::getActiveValueElements,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all value elements that belong to this interface,
    taking inheritance into account.

    Examples of value elements are `Input`, `Output`, and `Token`.
)docstring"))

        .def("_getInputValue", &mx::InterfaceElement::getInputValue,
             py::arg("name"),
             py::arg("target") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the typed value of an `Input` by its `name`, taking both the calling
    element and its declaration into account.

    :type name: str
    :param name: The name of the `Input` to be evaluated.
    :type target: str
    :param target: An optional target name, which will be used to filter
        the declarations that are considered.
    :returns: The value of the specified `Input`, if any, in this interface or
        its declaration, otherwise `None`.
)docstring"))

        .def("setTokenValue", &mx::InterfaceElement::setTokenValue,
             py::arg("name"),
             py::arg("value"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the string `value` of a `Token` by its `name`, creating a child element
    to hold the `Token` if needed.
)docstring"))

        .def("getTokenValue", &mx::InterfaceElement::getTokenValue,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the string value of a `Token` by its `name`, or an empty string if
    the specified `Token` is not present.
)docstring"))

        .def("setTarget", &mx::InterfaceElement::setTarget,
             py::arg("target"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the target string of this interface.
)docstring"))

        .def("hasTarget", &mx::InterfaceElement::hasTarget,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this interface has a target string.
)docstring"))

        .def("getTarget", &mx::InterfaceElement::getTarget,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the target string of this interface.
)docstring"))

        .def("setVersionString", &mx::InterfaceElement::setVersionString,
             py::arg("version"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the version string of this interface.
)docstring"))

        .def("hasVersionString", &mx::InterfaceElement::hasVersionString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this interface has a version string.
)docstring"))

        .def("getVersionString", &mx::InterfaceElement::getVersionString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the version string of this interface.
)docstring"))

        .def("setVersionIntegers", &mx::InterfaceElement::setVersionIntegers,
             py::arg("majorVersion"),
             py::arg("minorVersion"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the major and minor versions as an integer pair.
)docstring"))

        .def("getVersionIntegers", &mx::InterfaceElement::getVersionIntegers,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the major and minor versions as an integer pair.
)docstring"))

        .def("setDefaultVersion", &mx::InterfaceElement::setDefaultVersion,
             py::arg("defaultVersion"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the default version flag of this element.
)docstring"))

        .def("getDefaultVersion", &mx::InterfaceElement::getDefaultVersion,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the default version flag of this element.
)docstring"))

        .def("getDeclaration", &mx::InterfaceElement::getDeclaration,
             py::arg("target") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the first declaration of this interface, optionally filtered by the
    given `target` name.

    :type target: str
    :param target: An optional target name, which will be used to filter the
        declarations that are considered.
    :returns: The declaration, or `None` if no declaration was found.
)docstring"))

        .def("clearContent", &mx::InterfaceElement::clearContent,
             PYMATERIALX_DOCSTRING(R"docstring(
    Clear all attributes and descendants from this element.
)docstring"))

        .def("hasExactInputMatch", &mx::InterfaceElement::hasExactInputMatch,
             py::arg("declaration"),
             py::arg("message") = nullptr,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this instance has an exact input match with the given
    `declaration`, where each input of this the instance corresponds to a
    declaration input of the same name and type.

    If an exact input match is not found, and the optional `message` argument
    is provided, then an error message will be appended to the given string.
)docstring"))

        BIND_INTERFACE_TYPE_INSTANCE(integer, int)
        BIND_INTERFACE_TYPE_INSTANCE(boolean, bool)
        BIND_INTERFACE_TYPE_INSTANCE(float, float)
        BIND_INTERFACE_TYPE_INSTANCE(color3, mx::Color3)
        BIND_INTERFACE_TYPE_INSTANCE(color4, mx::Color4)
        BIND_INTERFACE_TYPE_INSTANCE(vector2, mx::Vector2)
        BIND_INTERFACE_TYPE_INSTANCE(vector3, mx::Vector3)
        BIND_INTERFACE_TYPE_INSTANCE(vector4, mx::Vector4)
        BIND_INTERFACE_TYPE_INSTANCE(matrix33, mx::Matrix33)
        BIND_INTERFACE_TYPE_INSTANCE(matrix44, mx::Matrix44)
        BIND_INTERFACE_TYPE_INSTANCE(string, std::string)
        BIND_INTERFACE_TYPE_INSTANCE(integerarray, mx::IntVec)
        BIND_INTERFACE_TYPE_INSTANCE(booleanarray, mx::BoolVec)
        BIND_INTERFACE_TYPE_INSTANCE(floatarray, mx::FloatVec)
        BIND_INTERFACE_TYPE_INSTANCE(stringarray, mx::StringVec)

        .def_readonly_static("NODE_DEF_ATTRIBUTE", &mx::InterfaceElement::NODE_DEF_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's NodeDef string is stored as an attribute.

    :see: `setNodeDefString()`
    :see: `hasNodeDefString()`
    :see: `getNodeDefString()`
)docstring"))

        .def_readonly_static("TARGET_ATTRIBUTE", &mx::InterfaceElement::TARGET_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's target string is stored as an attribute.

    :see: `setTarget()`
    :see: `hasTarget()`
    :see: `getTarget()`
)docstring"))

        .def_readonly_static("VERSION_ATTRIBUTE", &mx::InterfaceElement::VERSION_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's version string is stored as an attribute.

    :see: `setVersionString()`
    :see: `hasVersionString()`
    :see: `InterfaceElement.getVersionString()`
)docstring"))
        // We have to use `InterfaceElement.getVersionString()` above in order
        // to disambiguate from `PyMaterialXCore.getVersionString()`

        .def_readonly_static("DEFAULT_VERSION_ATTRIBUTE", &mx::InterfaceElement::DEFAULT_VERSION_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's default version flag is stored as an
    attribute.

    :see: `setDefaultVersion()`
    :see: `getDefaultVersion()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Base class for interface elements.

    An `InterfaceElement` supports a set of `Input` and `Output` elements, with
    an API for setting their values.

    Inherited by: `GraphElement`, `Implementation`, `Node`, `NodeDef`, and
    `Variant`.

    :see: https://materialx.org/docs/api/class_interface_element.html
)docstring");
}
