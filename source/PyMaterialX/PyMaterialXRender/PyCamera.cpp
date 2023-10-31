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
    py::class_<mx::Camera, mx::CameraPtr>(mod, "Camera")

        .def_static("create", &mx::Camera::create,
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create an instance of this class.
)docstring"))

        .def("setWorldMatrix", &mx::Camera::setWorldMatrix,
             py::arg("mat"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the world matrix.
)docstring"))

        .def("getWorldMatrix", &mx::Camera::getWorldMatrix,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the world matrix.
)docstring"))

        .def("setViewMatrix", &mx::Camera::setViewMatrix,
             py::arg("mat"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the view matrix.
)docstring"))

        .def("getViewMatrix", &mx::Camera::getViewMatrix,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the view matrix.
)docstring"))

        .def("setProjectionMatrix", &mx::Camera::setProjectionMatrix,
             py::arg("mat"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the projection matrix.
)docstring"))

        .def("getProjectionMatrix", &mx::Camera::getProjectionMatrix,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the projection matrix.
)docstring"))

        .def("getWorldViewProjMatrix", &mx::Camera::getWorldViewProjMatrix,
             PYMATERIALX_DOCSTRING(R"docstring(
    Compute our full model-view-projection matrix.
)docstring"))

        .def("getViewPosition", &mx::Camera::getViewPosition,
             PYMATERIALX_DOCSTRING(R"docstring(
    Derive viewer position from the view matrix.
)docstring"))

        .def("getViewDirection", &mx::Camera::getViewDirection,
             PYMATERIALX_DOCSTRING(R"docstring(
    Derive viewer direction from the view matrix.
)docstring"))

        .def("setViewportSize", &mx::Camera::setViewportSize,
             py::arg("size"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the size of the viewport window.
)docstring"))

        .def("getViewportSize", &mx::Camera::getViewportSize,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the size of the viewport window.
)docstring"))

        .def("projectToViewport", &mx::Camera::projectToViewport,
             py::arg("v"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Project a position from object to viewport space.
)docstring"))

        .def("unprojectFromViewport", &mx::Camera::unprojectFromViewport,
             py::arg("v"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Unproject a position from viewport to object space.
)docstring"))

        .def_static("createViewMatrix", &mx::Camera::createViewMatrix,
                    py::arg("eye"),
                    py::arg("target"),
                    py::arg("up"),
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create a view matrix given an `eye` position, a `target` position, and an
    `up` vector.
)docstring"))

        .def_static("createPerspectiveMatrix", &mx::Camera::createPerspectiveMatrix,
                    py::arg("left"),
                    py::arg("right"),
                    py::arg("bottom"),
                    py::arg("top"),
                    py::arg("nearP"),
                    py::arg("farP"),
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create a perpective projection matrix given a set of clip planes with
    `[-1, 1]` projected Z.
)docstring"))

        .def_static("createOrthographicMatrix", &mx::Camera::createOrthographicMatrix,
                    py::arg("left"),
                    py::arg("right"),
                    py::arg("bottom"),
                    py::arg("top"),
                    py::arg("nearP"),
                    py::arg("farP"),
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create an orthographic projection matrix given a set of clip planes with
    `[-1, 1]` projected Z.
)docstring"))

        .def_static("transformPointPerspective", &mx::Camera::transformPointPerspective,
                    py::arg("m"),
                    py::arg("v"),
                    PYMATERIALX_DOCSTRING(R"docstring(
    Apply a perspective transform to the given 3D point, performing a
    homogeneous divide on the transformed result.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    A simple camera class, supporting transform matrices and arcball
    functionality for object-viewing applications.

    :see: https://materialx.org/docs/api/class_camera.html
)docstring");
}
