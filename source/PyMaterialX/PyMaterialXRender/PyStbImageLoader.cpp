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
        .def_static("create", &mx::StbImageLoader::create, "Create a new stb image loader.")
        .def("saveImage", &mx::StbImageLoader::saveImage, "Save an image to the file system.")
        .def("loadImage", &mx::StbImageLoader::loadImage, "Load an image from the file system.");
}
