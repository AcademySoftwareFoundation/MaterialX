//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRender/ShaderRenderer.h>

namespace py = pybind11;
namespace mx = MaterialX;

class PyShaderRenderer : public mx::ShaderRenderer
{
  public:
    PyShaderRenderer() :
        mx::ShaderRenderer()
    {
    }

    void initialize() override
    {
        PYBIND11_OVERLOAD_PURE(
            void,
            mx::ShaderRenderer,
            initialize
        );
    }

    void createProgram(mx::ShaderPtr shader) override
    {
        PYBIND11_OVERLOAD_PURE(
            void,
            mx::ShaderRenderer,
            createProgram,
            shader
        );
    }

    void createProgram(const StageMap& stages) override
    {
        PYBIND11_OVERLOAD_PURE(
            void,
            mx::ShaderRenderer,
            createProgram,
            stages
        );
    }

    void validateInputs() override
    {
        PYBIND11_OVERLOAD_PURE(
            void,
            mx::ShaderRenderer,
            validateInputs
        );
    }

    void render() override
    {
        PYBIND11_OVERLOAD_PURE(
            void,
            mx::ShaderRenderer,
            render
        );
    }

    void saveImage(const mx::FilePath& filePath, mx::ConstImagePtr image, bool verticalFlip) override
    {
        PYBIND11_OVERLOAD_PURE(
            void,
            mx::ShaderRenderer,
            saveImage,
            filePath,
            image,
            verticalFlip
        );
    }
};

void bindPyShaderRenderer(py::module& mod)
{
    py::class_<mx::ShaderRenderer, PyShaderRenderer, mx::ShaderRendererPtr>(mod, "ShaderRenderer")
        .def("initialize", &mx::ShaderRenderer::initialize)
        .def("setSize", &mx::ShaderRenderer::setSize)
        .def("setImageHandler", &mx::ShaderRenderer::setImageHandler)
        .def("getImageHandler", &mx::ShaderRenderer::getImageHandler)
        .def("setLightHandler", &mx::ShaderRenderer::setLightHandler)
        .def("getLightHandler", &mx::ShaderRenderer::getLightHandler)
        .def("getGeometryHandler", &mx::ShaderRenderer::getGeometryHandler)
        .def("setViewHandler", &mx::ShaderRenderer::setViewHandler)
        .def("getViewHandler", &mx::ShaderRenderer::getViewHandler)
        .def("createProgram", static_cast<void (mx::ShaderRenderer::*)(mx::ShaderPtr)>(&mx::ShaderRenderer::createProgram))
        .def("createProgram", static_cast<void (mx::ShaderRenderer::*)(const mx::ShaderRenderer::StageMap&)>(&mx::ShaderRenderer::createProgram))
        .def("validateInputs", &mx::ShaderRenderer::validateInputs)
        .def("render", &mx::ShaderRenderer::render)
        .def("saveImage", &mx::ShaderRenderer::saveImage)
        .def("getReferencedImages", &mx::ShaderRenderer::getReferencedImages);

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
