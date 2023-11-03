//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRender/ImageHandler.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyImageHandler(py::module& mod)
{
    py::class_<mx::ImageSamplingProperties>(mod, "ImageSamplingProperties")

        .def_readwrite("uaddressMode", &mx::ImageSamplingProperties::uaddressMode,
                       PYMATERIALX_DOCSTRING(R"docstring(
    Address mode in U.
)docstring"))

        .def_readwrite("vaddressMode", &mx::ImageSamplingProperties::vaddressMode,
                       PYMATERIALX_DOCSTRING(R"docstring(
    Address mode in V.
)docstring"))

        .def_readwrite("filterType", &mx::ImageSamplingProperties::filterType,
                       PYMATERIALX_DOCSTRING(R"docstring(
    Filter type option.
)docstring"))

        .def_readwrite("defaultColor", &mx::ImageSamplingProperties::defaultColor,
                       PYMATERIALX_DOCSTRING(R"docstring(
    Default color. Corresponds to the `"default"` value on the image node
    definition.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing an interface to describe sampling properties for images.

    :see: https://materialx.org/docs/api/class_image_sampling_properties.html
)docstring");

    py::class_<mx::ImageLoader, mx::ImageLoaderPtr>(mod, "ImageLoader")

        .def_readonly_static("BMP_EXTENSION", &mx::ImageLoader::BMP_EXTENSION,
                             PYMATERIALX_DOCSTRING(R"docstring(
    Bitmap raster graphics image file format.
)docstring"))

        .def_readonly_static("EXR_EXTENSION", &mx::ImageLoader::EXR_EXTENSION,
                             PYMATERIALX_DOCSTRING(R"docstring(
    OpenEXR high-dynamic range, multi-channel raster graphics image file format.
)docstring"))

        .def_readonly_static("GIF_EXTENSION", &mx::ImageLoader::GIF_EXTENSION,
                             PYMATERIALX_DOCSTRING(R"docstring(
    Graphics Interchange Format.
)docstring"))

        .def_readonly_static("HDR_EXTENSION", &mx::ImageLoader::HDR_EXTENSION,
                             PYMATERIALX_DOCSTRING(R"docstring(
    High-dynamic range image file format.
)docstring"))

        .def_readonly_static("JPG_EXTENSION", &mx::ImageLoader::JPG_EXTENSION,
                             PYMATERIALX_DOCSTRING(R"docstring(
    Joint Photographic Experts Group image file format.
)docstring"))

        .def_readonly_static("JPEG_EXTENSION", &mx::ImageLoader::JPEG_EXTENSION,
                             PYMATERIALX_DOCSTRING(R"docstring(
    Joint Photographic Experts Group image file format.
)docstring"))

        .def_readonly_static("PIC_EXTENSION", &mx::ImageLoader::PIC_EXTENSION,
                             PYMATERIALX_DOCSTRING(R"docstring(
    PICtor image file format.
)docstring"))

        .def_readonly_static("PNG_EXTENSION", &mx::ImageLoader::PNG_EXTENSION,
                             PYMATERIALX_DOCSTRING(R"docstring(
    Portable Network Graphics raster graphics image file format.
)docstring"))

        .def_readonly_static("PSD_EXTENSION", &mx::ImageLoader::PSD_EXTENSION,
                             PYMATERIALX_DOCSTRING(R"docstring(
    Photoshop Document image file format.
)docstring"))

        .def_readonly_static("TGA_EXTENSION", &mx::ImageLoader::TGA_EXTENSION,
                             PYMATERIALX_DOCSTRING(R"docstring(
    Truevision TGA/TARGA raster graphics file format.
)docstring"))

        .def_readonly_static("TIF_EXTENSION", &mx::ImageLoader::TIF_EXTENSION,
                             PYMATERIALX_DOCSTRING(R"docstring(
    Tag Image File format.
)docstring"))

        .def_readonly_static("TIFF_EXTENSION", &mx::ImageLoader::TIFF_EXTENSION,
                             PYMATERIALX_DOCSTRING(R"docstring(
    Tag Image File Format.
)docstring"))

        .def_readonly_static("TXT_EXTENSION", &mx::ImageLoader::TXT_EXTENSION,
                             PYMATERIALX_DOCSTRING(R"docstring(
    Texture image file format.
)docstring"))

        .def("supportedExtensions", &mx::ImageLoader::supportedExtensions,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a set of extensions supported by the loader.
)docstring"))

        .def("saveImage", &mx::ImageLoader::saveImage,
             py::arg("filePath"),
             py::arg("image"),
             py::arg("verticalFlip") = false,
             PYMATERIALX_DOCSTRING(R"docstring(
    Save image to disk.

    This method must be implemented by derived classes.

    :param filePath: File path to be written.
    :type filePath: FilePath
    :param image: The image to be saved.
    :type image: Image
    :param verticalFlip: Whether the image should be flipped in Y during save.
    :type verticalFlip: bool
    :returns: `True` if save succeeded.
)docstring"))

        .def("loadImage", &mx::ImageLoader::loadImage,
             py::arg("filePath"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Load an image from the file system.

    This method must be implemented by derived classes.

    :param filePath: The requested image file path.
    :type filePath: FilePath
    :returns: On success, the loaded image; otherwise `None`.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Abstract base class for file-system image loaders.

    :see: https://materialx.org/docs/api/class_image_loader.html
)docstring");

    py::class_<mx::ImageHandler, mx::ImageHandlerPtr>(mod, "ImageHandler")

        .def_static("create", &mx::ImageHandler::create,
                    py::arg("imageLoader"),
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create an instance of this class, initialized using the given image loader.
)docstring"))

        .def("addLoader", &mx::ImageHandler::addLoader,
             py::arg("loader"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Add another image loader to the handler, which will be invoked if existing
    loaders cannot load a given image.
)docstring"))

        .def("saveImage", &mx::ImageHandler::saveImage,
             py::arg("filePath"),
             py::arg("image"),
             py::arg("verticalFlip") = false,
             PYMATERIALX_DOCSTRING(R"docstring(
    Save image to disk.

    This method must be implemented by derived classes.

    The first image loader which supports the file name extension will be used.

    :param filePath: File path to be written.
    :type filePath: FilePath
    :param image: The image to be saved.
    :type image: Image
    :param verticalFlip: Whether the image should be flipped in Y during save.
    :type verticalFlip: bool
    :returns: `True` if save succeeded.
)docstring"))

        .def("acquireImage", &mx::ImageHandler::acquireImage,
             py::arg("filePath"),
             py::arg_v("defaultColor",
                       mx::Color4(0.0f),
                       "mx.Color4(0.0)"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Acquire an image from the cache or file system. If the image is not
    found in the cache, then each image loader will be applied in turn.
    If the image cannot be found by any loader, then a uniform image of the
    given default color will be returned.

    :param filePath: File path of the image to acquire.
    :type filePath: FilePath
    :param defaultColor: Default color to use as a fallback for missing images.
    :type defaultColor: Color4
    :returns: On success, the acquired image; otherwise a uniform image of the
        given `defaultColor`.
)docstring"))

        .def("bindImage", &mx::ImageHandler::bindImage,
             py::arg("image"),
             py::arg("samplingProperties"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Bind an image for rendering.

    :param image: The image to bind.
    :type image: Image
    :param samplingProperties: Sampling properties for the image.
    :type samplingProperties: ImageSamplingProperties
)docstring"))

        .def("unbindImage", &mx::ImageHandler::unbindImage,
             py::arg("image"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Unbind an image, making it no longer active for rendering.

    :param image: The image to unbind.
    :type image: Image
)docstring"))

        .def("unbindImages", &mx::ImageHandler::unbindImages,
             PYMATERIALX_DOCSTRING(R"docstring(
    Unbind all images that are currently stored in the cache.
)docstring"))

        .def("setSearchPath", &mx::ImageHandler::setSearchPath,
             py::arg("searchPath"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the search path to be used for finding images on the file system.
)docstring"))

        .def("getSearchPath", &mx::ImageHandler::getSearchPath,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the image search path.
)docstring"))

        .def("setFilenameResolver", &mx::ImageHandler::setFilenameResolver,
             py::arg("resolver"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the filename resolver for images.
)docstring"))

        .def("getFilenameResolver", &mx::ImageHandler::getFilenameResolver,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the filename resolver for images.
)docstring"))

        .def("createRenderResources", &mx::ImageHandler::createRenderResources,
             py::arg("image"),
             py::arg("generateMipMaps"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Create rendering resources for the given image.
)docstring"))

        .def("releaseRenderResources", &mx::ImageHandler::releaseRenderResources,
             py::arg("image") = nullptr,
             PYMATERIALX_DOCSTRING(R"docstring(
    Release rendering resources for the given image, or for all cached images
    if `image` is `None`.
)docstring"))

        .def("clearImageCache", &mx::ImageHandler::clearImageCache,
             PYMATERIALX_DOCSTRING(R"docstring(
    Clear the contents of the image cache, first releasing any render resources
    associated with cached images.
)docstring"))

        .def("getZeroImage", &mx::ImageHandler::getZeroImage,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a fallback image with zeroes in all channels.
)docstring"))

        .def("getReferencedImages", &mx::ImageHandler::getReferencedImages,
             py::arg("doc"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Acquire all images referenced by the given document, and return the
    images as a list.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Base image handler class. Keeps track of images which are loaded from
    disk via supplied `ImageLoader`. Derived classes are responsible for
    determinining how to perform the logic for "binding" of these resources
    for a given target (such as a given shading language).

    :see: https://materialx.org/docs/api/class_image_handler.html
)docstring");
}
