//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRender/OiioImageLoader.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyOiioImageLoader(py::module& mod)
{
    py::class_<mx::OiioImageLoader, mx::ImageLoader, mx::OiioImageLoaderPtr>(mod, "OiioImageLoader")
        .def_static("create", &mx::OiioImageLoader::create, "Creator function.\n\nIf a TypeSystem is not provided it will be created internally. Optionally pass in an externally created TypeSystem here, if you want to keep type descriptions alive after the lifetime of the shader generator.")
        .def(py::init<>())
        .def("saveImage", &mx::OiioImageLoader::saveImage, "Save an image to the file system.\n\nArgs:\n    filePath: File path to be written\n    image: The image to be saved\n    verticalFlip: Whether the image should be flipped in Y during save\n\nReturns:\n    if save succeeded")
        .def("loadImage", &mx::OiioImageLoader::loadImage, "Load an image from the file system.\n\nArgs:\n    filePath: The requested image file path.\n\nReturns:\n    On success, a shared pointer to the loaded image; otherwise an empty shared pointer.");
}
