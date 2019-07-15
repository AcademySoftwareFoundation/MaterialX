//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Types.h>
#include <MaterialXGenShader/Util.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXRender/ImageHandler.h>
#include <cmath>

namespace MaterialX
{
string ImageDesc::BASETYPE_UINT8 = "UINT8";
string ImageDesc::BASETYPE_HALF = "HALF";
string ImageDesc::BASETYPE_FLOAT = "FLOAT";

string ImageDesc::IMAGETYPE_2D = "IMAGE2D";

void ImageDesc::freeResourceBuffer()
{
    if (resourceBuffer)
    {
        if (resourceBufferDeallocator)
        {
            resourceBufferDeallocator(resourceBuffer);
        }
        else
        {
            free(resourceBuffer);
        }
        resourceBuffer = nullptr;
    }
}

string ImageLoader::BMP_EXTENSION = "bmp";
string ImageLoader::EXR_EXTENSION = "exr";
string ImageLoader::GIF_EXTENSION = "gif";
string ImageLoader::HDR_EXTENSION = "hdr";
string ImageLoader::JPG_EXTENSION = "jpg";
string ImageLoader::JPEG_EXTENSION = "jpeg";
string ImageLoader::PIC_EXTENSION = "pic";
string ImageLoader::PNG_EXTENSION = "png";
string ImageLoader::PSD_EXTENSION = "psd";
string ImageLoader::TGA_EXTENSION = "tga";
string ImageLoader::TIF_EXTENSION = "tif";
string ImageLoader::TIFF_EXTENSION = "tiff";
string ImageLoader::TX_EXTENSION = "tx";
string ImageLoader::TXT_EXTENSION = "txt";
string ImageLoader::TXR_EXTENSION = "txr";

ImageHandler::ImageHandler(ImageLoaderPtr imageLoader)
{
    addLoader(imageLoader);
}

void ImageHandler::addLoader(ImageLoaderPtr loader)
{
    if (loader)
    {
        const StringSet& extensions = loader->supportedExtensions();
        for (const auto& extension : extensions)
        {
            _imageLoaders.insert(std::pair<string, ImageLoaderPtr>(extension, loader));
        }
    }
}

void ImageHandler::supportedExtensions(StringSet& extensions)
{
    extensions.clear();
    for (const auto& loader : _imageLoaders)
    {
        const StringSet& loaderExtensions = loader.second->supportedExtensions();
        extensions.insert(loaderExtensions.begin(), loaderExtensions.end());
    }
}

bool ImageHandler::saveImage(const FilePath& filePath,
                             const ImageDesc &imageDesc,
                             bool verticalFlip)
{
    FilePath foundFilePath = findFile(filePath);
    if (foundFilePath.isEmpty())
    {
        return false;
    }

    string extension = foundFilePath.getExtension();
    ImageLoaderMap::reverse_iterator iter;
    for (iter = _imageLoaders.rbegin(); iter != _imageLoaders.rend(); ++iter)
    {
        ImageLoaderPtr loader = iter->second;
        if (loader && loader->supportedExtensions().count(extension))
        {
            bool saved = iter->second->saveImage(foundFilePath, imageDesc, verticalFlip);
            if (saved)
            {
                return true;
            }
        }
    }
    return false;
}

bool ImageHandler::acquireImage(const FilePath& filePath, ImageDesc& imageDesc, bool /*generateMipMaps*/, const Color4* /*fallbackColor*/)
{
    if (filePath.isEmpty())
    {
        return false;
    }

    string extension = filePath.getExtension();
    ImageLoaderMap::reverse_iterator iter;
    for (iter = _imageLoaders.rbegin(); iter != _imageLoaders.rend(); ++iter)
    {
        ImageLoaderPtr loader = iter->second;
        if (loader && loader->supportedExtensions().count(extension))
        {
            bool acquired = loader->loadImage(filePath, imageDesc, getRestrictions());
            if (acquired)
            {
                return true;
            }
        }
    }
    return false;
}

bool ImageHandler::createColorImage(const Color4& color,
                                    ImageDesc& desc)
{
    unsigned int bufferSize = desc.width * desc.height * desc.channelCount;
    if (bufferSize < 1)
    {
        return false;
    }

    // Create a solid color image
    //
    desc.resourceBuffer = new float[bufferSize];
    float* pixel = static_cast<float*>(desc.resourceBuffer);
    for (size_t i = 0; i<desc.width; i++)
    {
        for (size_t j = 0; j<desc.height; j++)
        {
            for (unsigned int c = 0; c < desc.channelCount; c++)
            {
                *pixel++ = color[c];
            }
        }
    }
    desc.computeMipCount();
    desc.resourceBufferDeallocator = [](void *buffer)
    {
        delete[] static_cast<float*>(buffer);
    };
    return true;
}

bool ImageHandler::bindImage(const FilePath& /*filePath*/, const ImageSamplingProperties& /*samplingProperties*/)
{
    return false;
}

bool ImageHandler::unbindImage(const FilePath& /*filePath*/)
{
    return false;
}

void ImageHandler::cacheImage(const string& filePath, const ImageDesc& desc)
{
    if (!_imageCache.count(filePath))
    {
        _imageCache[filePath] = desc;
    }
}

void ImageHandler::uncacheImage(const string& filePath)
{
    _imageCache.erase(filePath);
}

const ImageDesc* ImageHandler::getCachedImage(const string& filePath)
{
    if (_imageCache.count(filePath))
    {
        return &(_imageCache[filePath]);
    }
    return nullptr;
}

FilePath ImageHandler::findFile(const FilePath& filePath)
{
    return _searchPath.find(filePath);
}

void ImageHandler::deleteImage(ImageDesc& imageDesc)
{
    imageDesc.freeResourceBuffer();
}

void ImageHandler::clearImageCache()
{
    for (auto iter : _imageCache)
    {
        deleteImage(iter.second);
    }
    _imageCache.clear();
}

void ImageSamplingProperties::setProperties(const string& fileNameUniform,
                                            const VariableBlock& uniformBlock)
{
    const std::string IMAGE_SEPARATOR("_");
    const std::string UADDRESS_MODE_POST_FIX("_uaddressmode");
    const std::string VADDRESS_MODE_POST_FIX("_vaddressmode");
    const std::string FILTER_TYPE_POST_FIX("_filtertype");
    const std::string DEFAULT_COLOR_POST_FIX("_default");
    const int INVALID_MAPPED_INT_VALUE = -1; // Any value < 0 is not considered to be invalid

    // Get the additional texture parameters based on image uniform name
    // excluding the trailing "_file" postfix string
    std::string root = fileNameUniform;
    size_t pos = root.find_last_of(IMAGE_SEPARATOR);
    if (pos != std::string::npos)
    {
        root = root.substr(0, pos);
    }

    const std::string uaddressmodeStr = root + UADDRESS_MODE_POST_FIX;
    const ShaderPort* port = uniformBlock.find(uaddressmodeStr);
    ValuePtr intValue = port ? port->getValue() : nullptr;
    uaddressMode = ImageSamplingProperties::AddressMode(intValue && intValue->isA<int>() ? intValue->asA<int>() : INVALID_MAPPED_INT_VALUE);

    const std::string vaddressmodeStr = root + VADDRESS_MODE_POST_FIX;
    port = uniformBlock.find(vaddressmodeStr);
    intValue = port ? port->getValue() : nullptr;
    vaddressMode = ImageSamplingProperties::AddressMode(intValue && intValue->isA<int>() ? intValue->asA<int>() : INVALID_MAPPED_INT_VALUE);

    const std::string filtertypeStr = root + FILTER_TYPE_POST_FIX;
    port = uniformBlock.find(filtertypeStr);
    intValue = port ? port->getValue() : nullptr;
    filterType = ImageSamplingProperties::FilterType(intValue && intValue->isA<int>() ? intValue->asA<int>() : INVALID_MAPPED_INT_VALUE);

    const std::string defaultColorStr = root + DEFAULT_COLOR_POST_FIX;
    port = uniformBlock.find(defaultColorStr);
    ValuePtr colorValue = port ? port->getValue() : nullptr;
    if (colorValue)
    {
        mapValueToColor(colorValue, defaultColor);
    }
}

} // namespace MaterialX
