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

    bool saveImage(const mx::FilePath& filePath, const mx::ImageDesc &imageDesc, bool verticalFlip) override
    {
        PYBIND11_OVERLOAD_PURE(
            bool,
            mx::ImageLoader,
            saveImage,
            filePath,
            imageDesc,
            verticalFlip
        );
    }

    bool loadImage(const mx::FilePath& filePath, mx::ImageDesc &imageDesc, const mx::ImageDescRestrictions* restrictions = nullptr) override
    {
        PYBIND11_OVERLOAD_PURE(
            bool,
            mx::ImageLoader,
            acquireImage,
            filePath,
            imageDesc,
            restrictions
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

    bool saveImage(const mx::FilePath& filePath, const mx::ImageDesc &imageDesc, bool verticalFlip = false) override
    {
        PYBIND11_OVERLOAD(
            bool,
            mx::ImageHandler,
            saveImage,
            filePath,
            imageDesc,
            verticalFlip
        );
    }

    bool acquireImage(const mx::FilePath& filePath, mx::ImageDesc& desc, bool generateMipMaps, const mx::Color4* fallbackColor) override
    {
        PYBIND11_OVERLOAD(
            bool,
            mx::ImageHandler,
            acquireImage,
            filePath,
            desc,
            generateMipMaps,
            fallbackColor
        );
    }

    bool createColorImage(const mx::Color4& color, mx::ImageDesc& imageDesc) override
    {
        PYBIND11_OVERLOAD(
            bool,
            mx::ImageHandler,
            createColorImage,
            color,
            imageDesc
        );
    }

    bool bindImage(const mx::FilePath& filePath, const mx::ImageSamplingProperties& samplingProperties) override
    {
        PYBIND11_OVERLOAD(
            bool,
            mx::ImageHandler,
            bindImage,
            filePath,
            samplingProperties
        );
    }

  protected:
    void deleteImage(mx::ImageDesc& imageDesc) override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ImageHandler,
            deleteImage,
            imageDesc
        );
    }
};

void bindPyImageHandler(py::module& mod)
{
    py::class_<mx::ImageBufferDeallocator>(mod, "ImageBufferDeallocator");

    py::class_<mx::ImageDesc>(mod, "ImageDesc")
        .def_readwrite("width", &mx::ImageDesc::width)
        .def_readwrite("height", &mx::ImageDesc::height)
        .def_readwrite("channelCount", &mx::ImageDesc::channelCount)
        .def_readwrite("mipCount", &mx::ImageDesc::mipCount)
        .def_readwrite("resourceBuffer", &mx::ImageDesc::resourceBuffer)
        .def_readwrite("baseType", &mx::ImageDesc::baseType)
        .def_readwrite("resourceId", &mx::ImageDesc::resourceId)
        .def_readwrite("resourceBufferDeallocator ", &mx::ImageDesc::resourceBufferDeallocator)
        .def("computeMipCount", &mx::ImageDesc::computeMipCount)
        .def("freeResourceBuffer", &mx::ImageDesc::freeResourceBuffer);

    py::class_<mx::ImageSamplingProperties>(mod, "ImageSamplingProperties")
        .def_readwrite("uaddressMode", &mx::ImageSamplingProperties::uaddressMode)
        .def_readwrite("vaddressMode", &mx::ImageSamplingProperties::vaddressMode)
        .def_readwrite("filterType", &mx::ImageSamplingProperties::filterType)
        .def_readwrite("defaultColor", &mx::ImageSamplingProperties::defaultColor);

    py::class_<mx::ImageLoader, PyImageLoader, mx::ImageLoaderPtr>(mod, "ImageLoader")
        .def_readwrite_static("BMP_EXTENSION", &mx::ImageLoader::BMP_EXTENSION)
        .def_readwrite_static("EXR_EXTENSION", &mx::ImageLoader::EXR_EXTENSION)
        .def_readwrite_static("GIF_EXTENSION", &mx::ImageLoader::GIF_EXTENSION)
        .def_readwrite_static("HDR_EXTENSION", &mx::ImageLoader::HDR_EXTENSION)
        .def_readwrite_static("JPG_EXTENSION", &mx::ImageLoader::JPG_EXTENSION)
        .def_readwrite_static("JPEG_EXTENSION", &mx::ImageLoader::JPEG_EXTENSION)
        .def_readwrite_static("PIC_EXTENSION", &mx::ImageLoader::PIC_EXTENSION)
        .def_readwrite_static("PNG_EXTENSION", &mx::ImageLoader::PNG_EXTENSION)
        .def_readwrite_static("PSD_EXTENSION", &mx::ImageLoader::PSD_EXTENSION)
        .def_readwrite_static("TGA_EXTENSION", &mx::ImageLoader::TGA_EXTENSION)
        .def_readwrite_static("TIF_EXTENSION", &mx::ImageLoader::TIF_EXTENSION)
        .def_readwrite_static("TIFF_EXTENSION", &mx::ImageLoader::TIFF_EXTENSION)
        .def_readwrite_static("TXT_EXTENSION", &mx::ImageLoader::TXT_EXTENSION)
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
        .def("createColorImage", &mx::ImageHandler::createColorImage)
        .def("bindImage", &mx::ImageHandler::bindImage)
        .def("clearImageCache", &mx::ImageHandler::clearImageCache)
        .def("setSearchPath", &mx::ImageHandler::setSearchPath)
        .def("getSearchPath", &mx::ImageHandler::getSearchPath)
        .def("findFile", &mx::ImageHandler::findFile);
}
