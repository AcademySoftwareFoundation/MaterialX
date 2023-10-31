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

        .def_static("create", &mx::StbImageLoader::create,
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create a new stb image loader.
)docstring"))

        .def("saveImage", &mx::StbImageLoader::saveImage,
             py::arg("filePath"),
             py::arg("image"),
             py::arg("verticalFlip") = false,
             PYMATERIALX_DOCSTRING(R"docstring(
    Save image to disk.

    :param filePath: File path to be written.
    :type filePath: FilePath
    :param image: The image to be saved.
    :type image: Image
    :param verticalFlip: Whether the image should be flipped in Y during save.
    :type verticalFlip: bool
    :returns: `True` if save succeeded.
)docstring"))

        .def("loadImage", &mx::StbImageLoader::loadImage,
             py::arg("filePath"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Load an image from the file system.

    :param filePath: The requested image file path.
    :type filePath: FilePath
    :returns: On success, the loaded image; otherwise `None`.
)docstring"))

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
