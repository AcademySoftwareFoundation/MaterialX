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
        .def("createColorImage", &mx::GLTextureHandler::createColorImage)
        .def("acquireImage", &mx::GLTextureHandler::acquireImage)
        .def("bindImage", &mx::GLTextureHandler::bindImage)
        .def("mapAddressModeToGL", &mx::GLTextureHandler::mapAddressModeToGL)
        .def("mapFilterTypeToGL", &mx::GLTextureHandler::mapFilterTypeToGL)
        .def("clearImageCache", &mx::GLTextureHandler::clearImageCache);
}
