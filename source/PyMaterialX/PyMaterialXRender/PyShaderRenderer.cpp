//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRender/ShaderRenderer.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyShaderRenderer(py::module& mod)
{
    py::class_<mx::ShaderRenderer, mx::ShaderRendererPtr>(mod, "ShaderRenderer")

        .def("initialize", &mx::ShaderRenderer::initialize,
             py::arg("renderContextHandle") = nullptr,
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize the renderer.
)docstring"))

        .def("setCamera", &mx::ShaderRenderer::setCamera,
             py::arg("camera"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the camera.
)docstring"))

        .def("getCamera", &mx::ShaderRenderer::getCamera,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the camera.
)docstring"))

        .def("setImageHandler", &mx::ShaderRenderer::setImageHandler,
             py::arg("imageHandler"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the image handler used by this renderer for image I/O.
)docstring"))

        .def("getImageHandler", &mx::ShaderRenderer::getImageHandler,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the image handler.
)docstring"))

        .def("setLightHandler", &mx::ShaderRenderer::setLightHandler,
             py::arg("lightHandler"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the light handler used by this renderer for light bindings.
)docstring"))

        .def("getLightHandler", &mx::ShaderRenderer::getLightHandler,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the light handler.
)docstring"))

        .def("setGeometryHandler", &mx::ShaderRenderer::setGeometryHandler,
             py::arg("geometryHandler"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the geometry handler.
)docstring"))

        .def("getGeometryHandler", &mx::ShaderRenderer::getGeometryHandler,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the geometry handler.
)docstring"))

        .def("createProgram",
             static_cast<void (mx::ShaderRenderer::*)(mx::ShaderPtr)>(&mx::ShaderRenderer::createProgram),
             py::arg("shader"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Create program based on an input shader.
)docstring"))

        .def("createProgram",
             static_cast<void (mx::ShaderRenderer::*)(const mx::ShaderRenderer::StageMap&)>(&mx::ShaderRenderer::createProgram),
             py::arg("stages"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Create program based on shader stage source code.

    :param stages: Map of name and source code for the shader stages.
    :type stages: Dict[str, str]
)docstring"))

        .def("validateInputs", &mx::ShaderRenderer::validateInputs,
             PYMATERIALX_DOCSTRING(R"docstring(
    Validate inputs for the program.
)docstring"))

        .def("updateUniform", &mx::ShaderRenderer::updateUniform,
             py::arg("name"),
             py::arg("value"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Update the program with value of the uniform.
)docstring"))

        .def("setSize", &mx::ShaderRenderer::setSize,
             py::arg("width"),
             py::arg("height"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the size of the rendered image.
)docstring"))

        .def("render", &mx::ShaderRenderer::render,
             PYMATERIALX_DOCSTRING(R"docstring(
    Render the current program to produce an image.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Base class for renderers that generate shader code to produce images.

    :see: https://materialx.org/docs/api/class_shader_renderer.html
)docstring");

    static py::exception<mx::ExceptionRenderError> pyExceptionRenderError(mod, "ExceptionRenderError");
    pyExceptionRenderError.doc() = PYMATERIALX_DOCSTRING(R"docstring(
    A type of exception that is raised when a rendering operation fails.

    Optionally stores an additional error log, which can be used to
    store and retrieve shader compilation errors.

    :see: https://materialx.org/docs/api/class_exception_render_error.html
)docstring");

    py::register_exception_translator(
        [](std::exception_ptr errPtr)
        {
            try
            {
                if (errPtr != NULL)
                    std::rethrow_exception(errPtr);
            }
            catch (const mx::ExceptionRenderError& err)
            {
                std::string errorMsg = err.what();
                for (std::string error : err.errorLog())
                {
                    errorMsg += "\n" + error;
                }
                PyErr_SetString(PyExc_LookupError, errorMsg.c_str());
            }
        }
    );
}
