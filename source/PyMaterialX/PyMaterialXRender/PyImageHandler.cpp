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
    py::class_<mx::ImageSamplingProperties>(mod, "ImageSamplingProperties", "Interface to describe sampling properties for images.")
        .def_readwrite("uaddressMode", &mx::ImageSamplingProperties::uaddressMode)
        .def_readwrite("vaddressMode", &mx::ImageSamplingProperties::vaddressMode)
        .def_readwrite("filterType", &mx::ImageSamplingProperties::filterType)
        .def_readwrite("defaultColor", &mx::ImageSamplingProperties::defaultColor);

    py::class_<mx::ImageLoader, mx::ImageLoaderPtr>(mod, "ImageLoader", "Abstract base class for file-system image loaders.")
        .def_readonly_static("BMP_EXTENSION", &mx::ImageLoader::BMP_EXTENSION)
        .def_readonly_static("EXR_EXTENSION", &mx::ImageLoader::EXR_EXTENSION)
        .def_readonly_static("GIF_EXTENSION", &mx::ImageLoader::GIF_EXTENSION)
        .def_readonly_static("HDR_EXTENSION", &mx::ImageLoader::HDR_EXTENSION)
        .def_readonly_static("JPG_EXTENSION", &mx::ImageLoader::JPG_EXTENSION)
        .def_readonly_static("JPEG_EXTENSION", &mx::ImageLoader::JPEG_EXTENSION)
        .def_readonly_static("PIC_EXTENSION", &mx::ImageLoader::PIC_EXTENSION)
        .def_readonly_static("PNG_EXTENSION", &mx::ImageLoader::PNG_EXTENSION)
        .def_readonly_static("PSD_EXTENSION", &mx::ImageLoader::PSD_EXTENSION)
        .def_readonly_static("TGA_EXTENSION", &mx::ImageLoader::TGA_EXTENSION)
        .def_readonly_static("TIF_EXTENSION", &mx::ImageLoader::TIF_EXTENSION)
        .def_readonly_static("TIFF_EXTENSION", &mx::ImageLoader::TIFF_EXTENSION)
        .def_readonly_static("TXT_EXTENSION", &mx::ImageLoader::TXT_EXTENSION)
        .def("supportedExtensions", &mx::ImageLoader::supportedExtensions, "Returns a list of supported extensions.\n\nReturns:\n    List of support extensions")
        .def("saveImage", &mx::ImageLoader::saveImage, "Save an image to the file system.\n\nArgs:\n    filePath: File path to be written\n    image: The image to be saved\n    verticalFlip: Whether the image should be flipped in Y during save\n\nReturns:\n    if save succeeded")
        .def("loadImage", &mx::ImageLoader::loadImage, "Load an image from the file system.\n\nArgs:\n    filePath: The requested image file path.\n\nReturns:\n    On success, a shared pointer to the loaded image; otherwise an empty shared pointer.");

    py::class_<mx::ImageHandler, mx::ImageHandlerPtr>(mod, "ImageHandler", "Base image handler class.\n\nKeeps track of images which are loaded from disk via supplied ImageLoader. Derived classes are responsible for determining how to perform the logic for \"binding\" of these resources for a given target (such as a given shading language).")
        .def_static("create", &mx::ImageHandler::create, "")
        .def("addLoader", &mx::ImageHandler::addLoader, "Add another image loader to the handler, which will be invoked if existing loaders cannot load a given image.")
        .def("saveImage", &mx::ImageHandler::saveImage, py::arg("filePath"), py::arg("image"), py::arg("verticalFlip") = false, "Save image to disk.\n\nArgs:\n    filePath: File path to be written\n    image: The image to be saved\n    verticalFlip: Whether the image should be flipped in Y during save\n\nReturns:\n    if save succeeded")
        .def("acquireImage", &mx::ImageHandler::acquireImage, py::arg("filePath"), py::arg("defaultColor") = mx::Color4(0.0f), "Acquire an image from the cache or file system.\n\nArgs:\n    filePath: File path of the image.\n    defaultColor: Default color to use as a fallback for missing images.\n\nReturns:\n    On success, a shared pointer to the acquired image.")
        .def("bindImage", &mx::ImageHandler::bindImage, "Bind an image for rendering.\n\nArgs:\n    image: The image to bind.\n    samplingProperties: Sampling properties for the image.")
        .def("unbindImage", &mx::ImageHandler::unbindImage, "Unbind an image, making it no longer active for rendering.\n\nArgs:\n    image: The image to unbind.")
        .def("unbindImages", &mx::ImageHandler::unbindImages, "Unbind all images that are currently stored in the cache.")
        .def("setSearchPath", &mx::ImageHandler::setSearchPath, "Set the search path to be used for finding images on the file system.")
        .def("getSearchPath", &mx::ImageHandler::getSearchPath, "Return the image search path.")
        .def("setFilenameResolver", &mx::ImageHandler::setFilenameResolver, "Set the filename resolver for images.")
        .def("getFilenameResolver", &mx::ImageHandler::getFilenameResolver, "Return the filename resolver for images.")
        .def("createRenderResources", &mx::ImageHandler::createRenderResources, "Create rendering resources for the given image.")
        .def("releaseRenderResources", &mx::ImageHandler::releaseRenderResources, py::arg("image") = nullptr, "Release rendering resources for the given image, or for all cached images if no image pointer is specified.")
        .def("clearImageCache", &mx::ImageHandler::clearImageCache, "Clear the contents of the image cache, first releasing any render resources associated with cached images.")
        .def("getZeroImage", &mx::ImageHandler::getZeroImage, "Return a fallback image with zeroes in all channels.")
        .def("getReferencedImages", &mx::ImageHandler::getReferencedImages, "Acquire all images referenced by the given document, and return the images in a vector.");
}
