//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRender/ViewHandler.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyViewHandler(py::module& mod)
{
    py::class_<mx::ViewHandler, mx::ViewHandlerPtr>(mod, "ViewHandler")
        .def_static("create", &mx::ViewHandler::create)
        .def_static("createViewMatrix", &mx::ViewHandler::createViewMatrix)
        .def_static("createPerspectiveMatrix", &mx::ViewHandler::createPerspectiveMatrix)
        .def(py::init<>())
        .def("setWorldMatrix", &mx::ViewHandler::setWorldMatrix)
        .def("projectionMatrix", &mx::ViewHandler::projectionMatrix, py::return_value_policy::reference)
        .def("viewMatrix", &mx::ViewHandler::viewMatrix, py::return_value_policy::reference)
        .def("viewPosition", &mx::ViewHandler::viewPosition, py::return_value_policy::reference)
        .def("viewDirection", &mx::ViewHandler::viewDirection, py::return_value_policy::reference)
        .def("worldMatrix", &mx::ViewHandler::worldMatrix, py::return_value_policy::reference)
        .def("degreesToRadians", &mx::ViewHandler::degreesToRadians);
}
