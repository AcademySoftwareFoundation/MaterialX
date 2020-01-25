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
        .def_static("createOrthographicMatrix", &mx::ViewHandler::createOrthographicMatrix)
        .def(py::init<>())
        .def_readwrite("worldMatrix", &mx::ViewHandler::worldMatrix)
        .def_readwrite("viewMatrix", &mx::ViewHandler::viewMatrix)
        .def_readwrite("viewPosition", &mx::ViewHandler::viewPosition)
        .def_readwrite("viewDirection", &mx::ViewHandler::viewDirection)
        .def_readwrite("projectionMatrix", &mx::ViewHandler::projectionMatrix);
}
