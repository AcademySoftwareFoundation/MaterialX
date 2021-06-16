//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRender/ShaderRenderer.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyShaderRenderer(py::module& mod)
{
    py::class_<mx::ShaderRenderer, mx::ShaderRendererPtr>(mod, "ShaderRenderer")
        .def("initialize", &mx::ShaderRenderer::initialize)
        .def("setImageHandler", &mx::ShaderRenderer::setImageHandler)
        .def("getImageHandler", &mx::ShaderRenderer::getImageHandler)
        .def("setLightHandler", &mx::ShaderRenderer::setLightHandler)
        .def("getLightHandler", &mx::ShaderRenderer::getLightHandler)
        .def("setGeometryHandler", &mx::ShaderRenderer::setGeometryHandler)
        .def("getGeometryHandler", &mx::ShaderRenderer::getGeometryHandler)
        .def("setViewHandler", &mx::ShaderRenderer::setViewHandler)
        .def("getViewHandler", &mx::ShaderRenderer::getViewHandler)
        .def("createProgram", static_cast<void (mx::ShaderRenderer::*)(mx::ShaderPtr)>(&mx::ShaderRenderer::createProgram))
        .def("createProgram", static_cast<void (mx::ShaderRenderer::*)(const mx::ShaderRenderer::StageMap&)>(&mx::ShaderRenderer::createProgram))
        .def("validateInputs", &mx::ShaderRenderer::validateInputs)
        .def("setSize", &mx::ShaderRenderer::setSize)
        .def("render", &mx::ShaderRenderer::render);

    static py::exception<mx::ExceptionShaderRenderError> pyExceptionShaderRenderError(mod, "ExceptionShaderRenderError");

    py::register_exception_translator(
        [](std::exception_ptr errPtr)
        {
            try
            {
                if (errPtr != NULL)
                    std::rethrow_exception(errPtr);
            }
            catch (const mx::ExceptionShaderRenderError& err)
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
