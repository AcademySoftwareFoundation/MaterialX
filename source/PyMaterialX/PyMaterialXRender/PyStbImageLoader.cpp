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
    py::class_<mx::StbImageLoader, mx::ImageLoader, mx::StbImageLoaderPtr>(mod, "StbImageLoader")
        .def_static("create", &mx::StbImageLoader::create)
        .def("saveImage", &mx::StbImageLoader::saveImage)
        .def("loadImage", &mx::StbImageLoader::loadImage)
        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class implementing an image loader using the stb image library.

    Supports the following file extensions:

    * `.bmp`
    * `.gif`
    * `.hdr`
    * `.jpg`
    * `.jpeg`
    * `.pic`
    * `.png`
    * `.psd`
    * `.tga`

    :see: https://materialx.org/docs/api/class_stb_image_loader.html
    :see: https://github.com/nothings/stb
)docstring");
}
