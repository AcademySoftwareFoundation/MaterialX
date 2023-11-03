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

        .def_static("create", &mx::OiioImageLoader::create,
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create a new OpenImageIO image loader.
)docstring"))

        .def(py::init<>(),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class.
)docstring"))

        .def("saveImage", &mx::OiioImageLoader::saveImage,
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

        .def("loadImage", &mx::OiioImageLoader::loadImage,
             py::arg("filePath"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Load an image from the file system.

    :param filePath: The requested image file path.
    :type filePath: FilePath
    :returns: On success, the loaded image; otherwise `None`.
)docstring"))

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
