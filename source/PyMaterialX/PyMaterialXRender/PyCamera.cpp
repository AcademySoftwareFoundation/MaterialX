//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRender/Camera.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyCamera(py::module& mod)
{
    py::class_<mx::Camera, mx::CameraPtr>(mod, "Camera", "A simple camera class, supporting transform matrices and arcball functionality for object-viewing applications.")
        .def_static("create", &mx::Camera::create)
        .def("setWorldMatrix", &mx::Camera::setWorldMatrix, "Set the world matrix.")
        .def("getWorldMatrix", &mx::Camera::getWorldMatrix, "Return the world matrix.")
        .def("setViewMatrix", &mx::Camera::setViewMatrix, "Set the view matrix.")
        .def("getViewMatrix", &mx::Camera::getViewMatrix, "Return the view matrix.")
        .def("setProjectionMatrix", &mx::Camera::setProjectionMatrix, "Set the projection matrix.")
        .def("getProjectionMatrix", &mx::Camera::getProjectionMatrix, "Return the projection matrix.")
        .def("getWorldViewProjMatrix", &mx::Camera::getWorldViewProjMatrix, "Compute our full model-view-projection matrix.")
        .def("getViewPosition", &mx::Camera::getViewPosition, "Derive viewer position from the view matrix.")
        .def("getViewDirection", &mx::Camera::getViewDirection, "Derive viewer direction from the view matrix.")
        .def("setViewportSize", &mx::Camera::setViewportSize, "Set the size of the viewport window.")
        .def("getViewportSize", &mx::Camera::getViewportSize, "Return the size of the viewport window.")
        .def("projectToViewport", &mx::Camera::projectToViewport, "Project a position from object to viewport space.")
        .def("unprojectFromViewport", &mx::Camera::unprojectFromViewport, "Unproject a position from viewport to object space.")
        .def_static("createViewMatrix", &mx::Camera::createViewMatrix)
        .def_static("createPerspectiveMatrix", &mx::Camera::createPerspectiveMatrix)
        .def_static("createOrthographicMatrix", &mx::Camera::createOrthographicMatrix)
        .def_static("transformPointPerspective", &mx::Camera::transformPointPerspective);
}
