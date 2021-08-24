//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRenderGlsl/GLTextureHandler.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyGLTextureHandler(py::module& mod)
{
    py::class_<mx::GLTextureHandler, mx::ImageHandler, mx::GLTextureHandlerPtr>(mod, "GLTextureHandler")
        .def_static("create", &mx::GLTextureHandler::create)
        .def("bindImage", &mx::GLTextureHandler::bindImage)
        .def("unbindImage", &mx::GLTextureHandler::unbindImage)
        .def("createRenderResources", &mx::GLTextureHandler::createRenderResources)
        .def("releaseRenderResources", &mx::GLTextureHandler::releaseRenderResources,
            py::arg("image") = nullptr);
}
