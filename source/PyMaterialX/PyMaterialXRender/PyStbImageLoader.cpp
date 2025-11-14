//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRender/StbImageLoader.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyStbImageLoader(py::module& mod)
{
    py::class_<mx::StbImageLoader, mx::ImageLoader, mx::StbImageLoaderPtr>(mod, "StbImageLoader", "Stb image file loader.")
        .def_static("create", &mx::StbImageLoader::create)
        .def("saveImage", &mx::StbImageLoader::saveImage, "Save image to disk.\n\nArgs:\n    filePath: File path to be written\n    image: The image to be saved\n    verticalFlip: Whether the image should be flipped in Y during save\n\nReturns:\n    if save succeeded")
        .def("loadImage", &mx::StbImageLoader::loadImage);
}
