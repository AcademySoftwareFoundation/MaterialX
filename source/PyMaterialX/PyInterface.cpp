//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Interface.h>

#include <MaterialXCore/Node.h>

namespace py = pybind11;
namespace mx = MaterialX;

#define BIND_INTERFACE_TYPE_INSTANCE(NAME, T)                                                                                                           \
.def("_setParameterValue" #NAME, &mx::InterfaceElement::setParameterValue<T>, py::arg("name"), py::arg("value"), py::arg("type") = mx::EMPTY_STRING)    \
.def("_setInputValue" #NAME, &mx::InterfaceElement::setInputValue<T>, py::arg("name"), py::arg("value"), py::arg("type") = mx::EMPTY_STRING)

void bindPyInterface(py::module& mod)
{
    py::class_<mx::Parameter, mx::ParameterPtr, mx::ValueElement>(mod, "Parameter")
        .def_readonly_static("CATEGORY", &mx::Parameter::CATEGORY);

    py::class_<mx::PortElement, mx::PortElementPtr, mx::ValueElement>(mod, "PortElement")
        .def("setNodeName", &mx::PortElement::setNodeName)
        .def("getNodeName", &mx::PortElement::getNodeName)
        .def("setConnectedNode", &mx::PortElement::setConnectedNode)
        .def("getConnectedNode", &mx::PortElement::getConnectedNode);

    py::class_<mx::Input, mx::InputPtr, mx::PortElement>(mod, "Input")
        .def_readonly_static("CATEGORY", &mx::Input::CATEGORY);

    py::class_<mx::Output, mx::OutputPtr, mx::PortElement>(mod, "Output")
        .def("hasUpstreamCycle", &mx::Output::hasUpstreamCycle)
        .def_readonly_static("CATEGORY", &mx::Output::CATEGORY);

    py::class_<mx::InterfaceElement, mx::InterfaceElementPtr, mx::TypedElement>(mod, "InterfaceElement")
        .def("setVersionString", &mx::InterfaceElement::setVersionString)
        .def("hasVersionString", &mx::InterfaceElement::hasVersionString)
        .def("getVersionString", &mx::InterfaceElement::getVersionString)
        .def("getVersionIntegers", &mx::InterfaceElement::getVersionIntegers)
        .def("setDefaultVersion", &mx::InterfaceElement::setDefaultVersion)
        .def("getDefaultVersion", &mx::InterfaceElement::getDefaultVersion)
        .def("addParameter", &mx::InterfaceElement::addParameter,
            py::arg("name") = mx::EMPTY_STRING, py::arg("type") = mx::DEFAULT_TYPE_STRING)
        .def("getParameter", &mx::InterfaceElement::getParameter)
        .def("getParameters", &mx::InterfaceElement::getParameters)
        .def("getParameterCount", &mx::InterfaceElement::getParameterCount)
        .def("removeParameter", &mx::InterfaceElement::removeParameter)
        .def("getActiveParameter", &mx::InterfaceElement::getActiveParameter)
        .def("getActiveParameters", &mx::InterfaceElement::getActiveParameters)
        .def("addInput", &mx::InterfaceElement::addInput,
            py::arg("name") = mx::EMPTY_STRING, py::arg("type") = mx::DEFAULT_TYPE_STRING)
        .def("getInput", &mx::InterfaceElement::getInput)
        .def("getInputs", &mx::InterfaceElement::getInputs)
        .def("getInputCount", &mx::InterfaceElement::getInputCount)
        .def("removeInput", &mx::InterfaceElement::removeInput)
        .def("getActiveInput", &mx::InterfaceElement::getActiveInput)
        .def("getActiveInputs", &mx::InterfaceElement::getActiveInputs)
        .def("addOutput", &mx::InterfaceElement::addOutput,
            py::arg("name") = mx::EMPTY_STRING, py::arg("type") = mx::DEFAULT_TYPE_STRING)
        .def("getOutput", &mx::InterfaceElement::getOutput)
        .def("getOutputs", &mx::InterfaceElement::getOutputs)
        .def("getOutputCount", &mx::InterfaceElement::getOutputCount)
        .def("removeOutput", &mx::InterfaceElement::removeOutput)
        .def("getActiveOutput", &mx::InterfaceElement::getActiveOutput)
        .def("getActiveOutputs", &mx::InterfaceElement::getActiveOutputs)
        .def("addToken", &mx::InterfaceElement::addToken,
            py::arg("name") = mx::DEFAULT_TYPE_STRING)
        .def("getToken", &mx::InterfaceElement::getToken)
        .def("getTokens", &mx::InterfaceElement::getTokens)
        .def("removeToken", &mx::InterfaceElement::removeToken)
        .def("getActiveToken", &mx::InterfaceElement::getActiveToken)
        .def("getActiveTokens", &mx::InterfaceElement::getActiveTokens)
        .def("getActiveValueElement", &mx::InterfaceElement::getActiveValueElement)
        .def("getActiveValueElements", &mx::InterfaceElement::getActiveValueElements)
        .def("_getParameterValue", &mx::InterfaceElement::getParameterValue)
        .def("_getInputValue", &mx::InterfaceElement::getInputValue)
        .def("setTokenValue", &mx::InterfaceElement::setTokenValue)
        .def("getTokenValue", &mx::InterfaceElement::getTokenValue)
        .def("getDeclaration", &mx::InterfaceElement::getDeclaration,
            py::arg("target") = mx::EMPTY_STRING)
        .def("isTypeCompatible", &mx::InterfaceElement::isTypeCompatible)
        .def("isVersionCompatible", &mx::InterfaceElement::isVersionCompatible)
        BIND_INTERFACE_TYPE_INSTANCE(integer, int)
        BIND_INTERFACE_TYPE_INSTANCE(boolean, bool)
        BIND_INTERFACE_TYPE_INSTANCE(float, float)
        BIND_INTERFACE_TYPE_INSTANCE(color2, mx::Color2)
        BIND_INTERFACE_TYPE_INSTANCE(color3, mx::Color3)
        BIND_INTERFACE_TYPE_INSTANCE(color4, mx::Color4)
        BIND_INTERFACE_TYPE_INSTANCE(vector2, mx::Vector2)
        BIND_INTERFACE_TYPE_INSTANCE(vector3, mx::Vector3)
        BIND_INTERFACE_TYPE_INSTANCE(vector4, mx::Vector4)
        BIND_INTERFACE_TYPE_INSTANCE(matrix33, mx::Matrix33)
        BIND_INTERFACE_TYPE_INSTANCE(matrix44, mx::Matrix44)
        BIND_INTERFACE_TYPE_INSTANCE(string, std::string);
}
