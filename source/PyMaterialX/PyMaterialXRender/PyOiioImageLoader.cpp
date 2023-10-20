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
        .def("saveImage", &mx::OiioImageLoader::saveImage)
        .def("loadImage", &mx::OiioImageLoader::loadImage)
        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class implementing an OpenImageIO image file loader.

    Supports the following file extensions:

    * `.bmp`
    * `.exr`
    * `.gif`
    * `.hdr`
    * `.jpg`
    * `.jpeg`
    * `.pic`
    * `.png`
    * `.psd`
    * `.tga`
    * `.tif`
    * `.tiff`
    * `.tx`
    * `.txt`
    * `.txr`

    :see: https://materialx.org/docs/api/class_oiio_image_loader.html
    :see: https://github.com/AcademySoftwareFoundation/OpenImageIO
)docstring");
}
