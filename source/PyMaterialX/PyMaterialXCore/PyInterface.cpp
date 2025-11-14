//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Interface.h>

#include <MaterialXCore/Node.h>

namespace py = pybind11;
namespace mx = MaterialX;

#define BIND_INTERFACE_TYPE_INSTANCE(NAME, T)                                                                                                           \
.def("_setInputValue" #NAME, &mx::InterfaceElement::setInputValue<T>, py::arg("name"), py::arg("value"), py::arg("type") = mx::EMPTY_STRING)

void bindPyInterface(py::module& mod)
{
    py::class_<mx::PortElement, mx::PortElementPtr, mx::ValueElement>(mod, "PortElement", "The base class for port elements such as Input and Output.\n\nPort elements support spatially-varying upstream connections to nodes.")
        .def("setNodeName", &mx::PortElement::setNodeName, "Set the node name string of this element, creating a connection to the Node with the given name within the same NodeGraph.")
        .def("getNodeName", &mx::PortElement::getNodeName, "Return the node name string of this element.")
        .def("setNodeGraphString", &mx::PortElement::setNodeGraphString, "Set the node graph string of this element.")
        .def("hasNodeGraphString", &mx::PortElement::hasNodeGraphString, "Return true if this element has a node graph string.")
        .def("getNodeGraphString", &mx::PortElement::getNodeGraphString, "Return the node graph string of this element.")
        .def("setOutputString", &mx::PortElement::setOutputString, "Set the output string of this element.")
        .def("hasOutputString", &mx::PortElement::hasOutputString, "Return true if this element has an output string.")
        .def("getOutputString", &mx::PortElement::getOutputString, "Return the output string of this element.")
        .def("setConnectedNode", &mx::PortElement::setConnectedNode, "Set the node to which the given input is connected, creating a child input if needed.\n\nIf the node argument is null, then any existing node connection on the input will be cleared.")
        .def("getConnectedNode", &mx::PortElement::getConnectedNode, "Return the node, if any, to which this input is connected.")
        .def("setConnectedOutput", &mx::PortElement::setConnectedOutput, "Set the output to which the given input is connected, creating a child input if needed.\n\nIf the node argument is null, then any existing output connection on the input will be cleared.")
        .def("getConnectedOutput", &mx::PortElement::getConnectedOutput, "Return the output connected to the given input.\n\nIf the given input is not present, then an empty OutputPtr is returned.");

    py::class_<mx::Input, mx::InputPtr, mx::PortElement>(mod, "Input", "An input element within a Node or NodeDef.\n\nAn Input holds either a uniform value or a connection to a spatially-varying Output, either of which may be modified within the scope of a Material.")
        .def("setDefaultGeomPropString", &mx::Input::setDefaultGeomPropString, "Set the defaultgeomprop string for the input.")
        .def("hasDefaultGeomPropString", &mx::Input::hasDefaultGeomPropString, "Return true if the given input has a defaultgeomprop string.")
        .def("getDefaultGeomPropString", &mx::Input::getDefaultGeomPropString, "Return the defaultgeomprop string for the input.")
        .def("getDefaultGeomProp", &mx::Input::getDefaultGeomProp, "Return the GeomPropDef element to use, if defined for this input.")
        .def("getConnectedNode", &mx::Input::getConnectedNode, "Return the node, if any, to which this input is connected.")
        .def("setConnectedInterfaceName", &mx::Input::setConnectedInterfaceName, "Connects this input to a corresponding interface with the given name.\n\nIf the interface name specified is an empty string then any existing connection is removed.")
        .def("getInterfaceInput", &mx::Input::getInterfaceInput, "Return the input on the parent graph corresponding to the interface name for this input.")
        .def_readonly_static("CATEGORY", &mx::Input::CATEGORY);

    py::class_<mx::Output, mx::OutputPtr, mx::PortElement>(mod, "Output", "A spatially-varying output element within a NodeGraph or NodeDef.")
        .def("hasUpstreamCycle", &mx::Output::hasUpstreamCycle, "Return true if a cycle exists in any upstream path from this element.")
        .def_readonly_static("CATEGORY", &mx::Output::CATEGORY)
        .def_readonly_static("DEFAULT_INPUT_ATTRIBUTE", &mx::Output::DEFAULT_INPUT_ATTRIBUTE);

    py::class_<mx::InterfaceElement, mx::InterfaceElementPtr, mx::TypedElement>(mod, "InterfaceElement", "The base class for interface elements such as Node, NodeDef, and NodeGraph.\n\nAn InterfaceElement supports a set of Input and Output elements, with an API for setting their values.")
        .def("setNodeDefString", &mx::InterfaceElement::setNodeDefString, "Set the NodeDef string for the interface.")
        .def("hasNodeDefString", &mx::InterfaceElement::hasNodeDefString, "Return true if the given interface has a NodeDef string.")
        .def("getNodeDefString", &mx::InterfaceElement::getNodeDefString, "Return the NodeDef string for the interface.")
        .def("addInput", &mx::InterfaceElement::addInput,
            py::arg("name") = mx::EMPTY_STRING, py::arg("type") = mx::DEFAULT_TYPE_STRING, "Add an Input to this interface.\n\nArgs:\n    name: The name of the new Input. If no name is specified, then a unique name will automatically be generated.\n    type: An optional type string.\n\nReturns:\n    A shared pointer to the new Input.")
        .def("getInput", &mx::InterfaceElement::getInput, "Return the Input, if any, with the given name.")
        .def("getInputs", &mx::InterfaceElement::getInputs, "Return a vector of all Input elements.")
        .def("getInputCount", &mx::InterfaceElement::getInputCount, "Return the number of Input elements.")
        .def("removeInput", &mx::InterfaceElement::removeInput, "Remove the Input, if any, with the given name.")
        .def("getActiveInput", &mx::InterfaceElement::getActiveInput, "Return the first Input with the given name that belongs to this interface, taking interface inheritance into account.")
        .def("getActiveInputs", &mx::InterfaceElement::getActiveInputs, "Return a vector of all Input elements that belong to this interface, taking inheritance into account.")
        .def("addOutput", &mx::InterfaceElement::addOutput,
            py::arg("name") = mx::EMPTY_STRING, py::arg("type") = mx::DEFAULT_TYPE_STRING, "Add an Output to this interface.\n\nArgs:\n    name: The name of the new Output. If no name is specified, then a unique name will automatically be generated.\n    type: An optional type string.\n\nReturns:\n    A shared pointer to the new Output.")
        .def("getOutput", &mx::InterfaceElement::getOutput, "Return the Output, if any, with the given name.")
        .def("getOutputs", &mx::InterfaceElement::getOutputs, "Return a vector of all Output elements.")
        .def("getOutputCount", &mx::InterfaceElement::getOutputCount, "Return the number of Output elements.")
        .def("removeOutput", &mx::InterfaceElement::removeOutput, "Remove the Output, if any, with the given name.")
        .def("getActiveOutput", &mx::InterfaceElement::getActiveOutput, "Return the first Output with the given name that belongs to this interface, taking interface inheritance into account.")
        .def("getActiveOutputs", &mx::InterfaceElement::getActiveOutputs, "Return a vector of all Output elements that belong to this interface, taking inheritance into account.")
        .def("setConnectedOutput", &mx::InterfaceElement::setConnectedOutput, "Set the output to which the given input is connected, creating a child input if needed.\n\nIf the node argument is null, then any existing output connection on the input will be cleared.")
        .def("getConnectedOutput", &mx::InterfaceElement::getConnectedOutput, "Return the output connected to the given input.\n\nIf the given input is not present, then an empty OutputPtr is returned.")
        .def("addToken", &mx::InterfaceElement::addToken,
            py::arg("name") = mx::DEFAULT_TYPE_STRING, "Add a Token to this element.\n\nArgs:\n    name: The name of the new Token. If no name is specified, then a unique name will automatically be generated.\n\nReturns:\n    A shared pointer to the new Token.")
        .def("getToken", &mx::InterfaceElement::getToken, "Return the Token, if any, with the given name.")
        .def("getTokens", &mx::InterfaceElement::getTokens, "Return a vector of all Token elements.")
        .def("removeToken", &mx::InterfaceElement::removeToken, "Remove the Token, if any, with the given name.")
        .def("getActiveToken", &mx::InterfaceElement::getActiveToken, "Return the first Token with the given name that belongs to this interface, taking interface inheritance into account.")
        .def("getActiveTokens", &mx::InterfaceElement::getActiveTokens, "Return a vector of all Token elements that belong to this interface, taking inheritance into account.")
        .def("getActiveValueElement", &mx::InterfaceElement::getActiveValueElement, "Return the first value element with the given name that belongs to this interface, taking interface inheritance into account.\n\nExamples of value elements are Input, Output, and Token.")
        .def("getActiveValueElements", &mx::InterfaceElement::getActiveValueElements, "Return a vector of all value elements that belong to this interface, taking inheritance into account.\n\nExamples of value elements are Input, Output, and Token.")
        .def("_getInputValue", &mx::InterfaceElement::getInputValue, "Return the typed value of an input by its name, taking both the calling element and its declaration into account.\n\nArgs:\n    name: The name of the input to be evaluated.\n    target: An optional target name, which will be used to filter the declarations that are considered.\n\nReturns:\n    If the given input is found in this interface or its declaration, then a shared pointer to its value is returned; otherwise, an empty shared pointer is returned.")
        .def("setTokenValue", &mx::InterfaceElement::setTokenValue, "Set the string value of a Token by its name, creating a child element to hold the Token if needed.")
        .def("getTokenValue", &mx::InterfaceElement::getTokenValue, "Return the string value of a Token by its name, or an empty string if the given Token is not present.")
        .def("setTarget", &mx::InterfaceElement::setTarget, "Set the target string of this interface.")
        .def("hasTarget", &mx::InterfaceElement::hasTarget, "Return true if the given interface has a target string.")
        .def("getTarget", &mx::InterfaceElement::getTarget, "Return a unique identifier for the target this generator is for.")
        .def("setVersionString", &mx::InterfaceElement::setVersionString, "Set the version string of this interface.")
        .def("hasVersionString", &mx::InterfaceElement::hasVersionString, "Return true if this interface has a version string.")
        .def("getVersionString", &mx::InterfaceElement::getVersionString, "Return the version string of this interface.")
        .def("setVersionIntegers", &mx::InterfaceElement::setVersionIntegers, "Set the major and minor versions as an integer pair.")
        .def("getVersionIntegers", &mx::InterfaceElement::getVersionIntegers, "Return the major and minor versions as an integer pair.")
        .def("setDefaultVersion", &mx::InterfaceElement::setDefaultVersion, "Set the default version flag of this element.")
        .def("getDefaultVersion", &mx::InterfaceElement::getDefaultVersion, "Return the default version flag of this element.")
        .def("getDeclaration", &mx::InterfaceElement::getDeclaration,
            py::arg("target") = mx::EMPTY_STRING, "Return the first declaration of this interface, optionally filtered by the given target name.")
        .def("clearContent", &mx::InterfaceElement::clearContent, "Clear all attributes and descendants from this element.")
        .def("hasExactInputMatch", &mx::InterfaceElement::hasExactInputMatch,
            py::arg("declaration"), py::arg("message") = nullptr, "Return true if this instance has an exact input match with the given declaration, where each input of this the instance corresponds to a declaration input of the same name and type.\n\nIf an exact input match is not found, and the optional message argument is provided, then an error message will be appended to the given string.")
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
        .def_readonly_static("NODE_DEF_ATTRIBUTE", &mx::InterfaceElement::NODE_DEF_ATTRIBUTE);
}
