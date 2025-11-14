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
        .def_static("create", &mx::OiioImageLoader::create)
        .def(py::init<>())
        .def("saveImage", &mx::OiioImageLoader::saveImage, "Save image to disk.\n\nArgs:\n    filePath: File path to be written\n    image: The image to be saved\n    verticalFlip: Whether the image should be flipped in Y during save\n\nReturns:\n    if save succeeded")
        .def("loadImage", &mx::OiioImageLoader::loadImage);
}
