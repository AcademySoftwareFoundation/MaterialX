//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRender/ImageHandler.h>

namespace py = pybind11;
namespace mx = MaterialX;

class PyImageLoader : public mx::ImageLoader
{
  public:
    PyImageLoader() :
        mx::ImageLoader()
    {
    }

    bool saveImage(const mx::FilePath& filePath, mx::ImagePtr image, bool verticalFlip) override
    {
        PYBIND11_OVERLOAD_PURE(
            bool,
            mx::ImageLoader,
            saveImage,
            filePath,
            image,
            verticalFlip
        );
    }

    mx::ImagePtr loadImage(const mx::FilePath& filePath) override
    {
        PYBIND11_OVERLOAD_PURE(
            mx::ImagePtr,
            mx::ImageLoader,
            loadImage,
            filePath
        );
    }

};

class PyImageHandler : public mx::ImageHandler
{
  public:
    explicit PyImageHandler(mx::ImageLoaderPtr imageLoader) :
        mx::ImageHandler(imageLoader)
    {
    }

    bool saveImage(const mx::FilePath& filePath, mx::ImagePtr image, bool verticalFlip = false) override
    {
        PYBIND11_OVERLOAD(
            bool,
            mx::ImageHandler,
            saveImage,
            filePath,
            image,
            verticalFlip
        );
    }

    mx::ImagePtr acquireImage(const mx::FilePath& filePath, bool generateMipMaps, const mx::Color4* fallbackColor, std::string* message) override
    {
        PYBIND11_OVERLOAD(
            mx::ImagePtr,
            mx::ImageHandler,
            acquireImage,
            filePath,
            generateMipMaps,
            fallbackColor,
            message
        );
    }

    bool bindImage(mx::ImagePtr image, const mx::ImageSamplingProperties& samplingProperties) override
    {
        PYBIND11_OVERLOAD(
            bool,
            mx::ImageHandler,
            bindImage,
            image,
            samplingProperties
        );
    }

  protected:
    void deleteImage(mx::ImagePtr image) override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ImageHandler,
            deleteImage,
            image
        );
    }
};

void bindPyImageHandler(py::module& mod)
{
    py::class_<mx::ImageBufferDeallocator>(mod, "ImageBufferDeallocator");

    py::class_<mx::Image>(mod, "Image")
        .def_static("create", &mx::Image::create)
        .def_static("createConstantColor", &mx::Image::createConstantColor)
        .def("getWidth", &mx::Image::getWidth)
        .def("getHeight", &mx::Image::getHeight)
        .def("getChannelCount", &mx::Image::getChannelCount)
        .def("getBaseType", &mx::Image::getBaseType)
        .def("getBaseStride", &mx::Image::getBaseStride)
        .def("getMaxMipCount", &mx::Image::getMaxMipCount)
        .def("setResourceBuffer", &mx::Image::setResourceBuffer)
        .def("getResourceBuffer", &mx::Image::getResourceBuffer)
        .def("createResourceBuffer", &mx::Image::createResourceBuffer)
        .def("releaseResourceBuffer", &mx::Image::releaseResourceBuffer)
        .def("setResourceBufferDeallocator", &mx::Image::setResourceBufferDeallocator)
        .def("getResourceBufferDeallocator", &mx::Image::getResourceBufferDeallocator)
        .def("getTexelColor", &mx::Image::getTexelColor);

    py::class_<mx::ImageSamplingProperties>(mod, "ImageSamplingProperties")
        .def_readwrite("uaddressMode", &mx::ImageSamplingProperties::uaddressMode)
        .def_readwrite("vaddressMode", &mx::ImageSamplingProperties::vaddressMode)
        .def_readwrite("filterType", &mx::ImageSamplingProperties::filterType)
        .def_readwrite("defaultColor", &mx::ImageSamplingProperties::defaultColor);

    py::class_<mx::ImageLoader, PyImageLoader, mx::ImageLoaderPtr>(mod, "ImageLoader")
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
        .def(py::init<>())
        .def("supportedExtensions", &mx::ImageLoader::supportedExtensions)
        .def("saveImage", &mx::ImageLoader::saveImage)
        .def("loadImage", &mx::ImageLoader::loadImage);

    py::class_<mx::ImageHandler, PyImageHandler, mx::ImageHandlerPtr>(mod, "ImageHandler")
        .def(py::init<mx::ImageLoaderPtr>())
        .def_static("create", &mx::ImageHandler::create)
        .def("addLoader", &mx::ImageHandler::addLoader)
        .def("saveImage", &mx::ImageHandler::saveImage)
        .def("acquireImage", &mx::ImageHandler::acquireImage)
        .def("bindImage", &mx::ImageHandler::bindImage)
        .def("clearImageCache", &mx::ImageHandler::clearImageCache)
        .def("setSearchPath", &mx::ImageHandler::setSearchPath)
        .def("getSearchPath", &mx::ImageHandler::getSearchPath);
}
