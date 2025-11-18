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
    py::class_<mx::ShaderRenderer, mx::ShaderRendererPtr>(mod, "ShaderRenderer", "Base class for renderers that generate shader code to produce images.")
        .def("initialize", &mx::ShaderRenderer::initialize, py::arg("renderContextHandle") = nullptr, "Initialize the renderer.")
        .def("setCamera", &mx::ShaderRenderer::setCamera, "Set the camera.")
        .def("getCamera", &mx::ShaderRenderer::getCamera, "Return the camera.")
        .def("setImageHandler", &mx::ShaderRenderer::setImageHandler, "Set the image handler used by this renderer for image I/O.")
        .def("getImageHandler", &mx::ShaderRenderer::getImageHandler, "Return the image handler.")
        .def("setLightHandler", &mx::ShaderRenderer::setLightHandler, "Set the light handler used by this renderer for light bindings.")
        .def("getLightHandler", &mx::ShaderRenderer::getLightHandler, "Return the light handler.")
        .def("setGeometryHandler", &mx::ShaderRenderer::setGeometryHandler, "Set the geometry handler.")
        .def("getGeometryHandler", &mx::ShaderRenderer::getGeometryHandler, "Return the geometry handler.")
        .def("createProgram", static_cast<void (mx::ShaderRenderer::*)(mx::ShaderPtr)>(&mx::ShaderRenderer::createProgram), "Create program based on shader stage source code.\n\nArgs:\n    stages: Map of name and source code for the shader stages.")
        .def("createProgram", static_cast<void (mx::ShaderRenderer::*)(const mx::ShaderRenderer::StageMap&)>(&mx::ShaderRenderer::createProgram), "Create program based on shader stage source code.\n\nArgs:\n    stages: Map of name and source code for the shader stages.")
        .def("validateInputs", &mx::ShaderRenderer::validateInputs, "Validate inputs for the program.")
        .def("updateUniform", &mx::ShaderRenderer::updateUniform, "Update the program with value of the uniform.")
        .def("setSize", &mx::ShaderRenderer::setSize, "Set the size of the rendered image.")
        .def("render", &mx::ShaderRenderer::render, "Render the current program to produce an image.");

    static py::exception<mx::ExceptionRenderError> pyExceptionRenderError(mod, "ExceptionRenderError");

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
