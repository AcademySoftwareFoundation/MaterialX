//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRenderHlsl/HlslTextureHandler.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyHlslTextureHandler(py::module& mod)
{
    // HlslTextureHandler subclasses ImageHandler, so Python clients get
    // the full ImageHandler interface (bindImage, unbindImage,
    // releaseRenderResources, ...) plus the small HLSL-specific
    // factory. Underlying COM pointers (SRV / sampler) aren't exposed
    // to Python - the renderer uses them internally when binding the
    // material's t# / s# slots.
    py::class_<mx::HlslTextureHandler, mx::ImageHandler, mx::HlslTextureHandlerPtr>(mod, "HlslTextureHandler")
        .def_static("create", &mx::HlslTextureHandler::create);
}
