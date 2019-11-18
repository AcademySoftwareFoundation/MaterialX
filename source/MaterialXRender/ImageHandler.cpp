//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRender/ImageHandler.h>

#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Util.h>

#include <cmath>

namespace MaterialX
{

const string IMAGE_PROPERTY_SEPARATOR("_");
const string UADDRESS_MODE_SUFFIX(IMAGE_PROPERTY_SEPARATOR + "uaddressmode");
const string VADDRESS_MODE_SUFFIX(IMAGE_PROPERTY_SEPARATOR + "vaddressmode");
const string FILTER_TYPE_SUFFIX(IMAGE_PROPERTY_SEPARATOR + "filtertype");
const string DEFAULT_COLOR_SUFFIX(IMAGE_PROPERTY_SEPARATOR + "default");

const string ImageLoader::BMP_EXTENSION = "bmp";
const string ImageLoader::EXR_EXTENSION = "exr";
const string ImageLoader::GIF_EXTENSION = "gif";
const string ImageLoader::HDR_EXTENSION = "hdr";
const string ImageLoader::JPG_EXTENSION = "jpg";
const string ImageLoader::JPEG_EXTENSION = "jpeg";
const string ImageLoader::PIC_EXTENSION = "pic";
const string ImageLoader::PNG_EXTENSION = "png";
const string ImageLoader::PSD_EXTENSION = "psd";
const string ImageLoader::TGA_EXTENSION = "tga";
const string ImageLoader::TIF_EXTENSION = "tif";
const string ImageLoader::TIFF_EXTENSION = "tiff";
const string ImageLoader::TX_EXTENSION = "tx";
const string ImageLoader::TXT_EXTENSION = "txt";
const string ImageLoader::TXR_EXTENSION = "txr";

//
// Image methods
//

Image::Image(unsigned int width, unsigned int height, unsigned int channelCount, BaseType baseType) :
    _width(width),
    _height(height),
    _channelCount(channelCount),
    _baseType(baseType),
    _resourceBuffer(nullptr),
    _resourceBufferDeallocator(nullptr),
    _resourceId(0)
{
}

Image::~Image()
{
    releaseResourceBuffer();
}

unsigned int Image::getBaseStride() const
{
    if (_baseType == BaseType::HALF)
    {
        return 2;
    }
    if (_baseType == BaseType::FLOAT)
    {
        return 4;
    }
    return 1;
}

unsigned int Image::getMaxMipCount() const
{
    return (unsigned int) std::log2(std::max(_width, _height)) + 1;
}

ImagePtr Image::createConstantColor(unsigned int width, unsigned int height, const Color4& color)
{
    unsigned int channelCount = 4;
    size_t bufferSize = width * height * channelCount;
    if (!bufferSize)
    {
        return nullptr;
    }

    ImagePtr image = create(width, height, channelCount, Image::BaseType::FLOAT);
    image->createResourceBuffer();
    float* pixel = static_cast<float*>(image->getResourceBuffer());
    for (size_t i = 0; i < image->getWidth(); i++)
    {
        for (size_t j = 0; j < image->getHeight(); j++)
        {
            for (unsigned int c = 0; c < image->getChannelCount(); c++)
            {
                *pixel++ = color[c];
            }
        }
    }
    return image;
}

Color4 Image::getTexelColor(unsigned int x, unsigned int y) const
{
    if (x >= _width || y >= _height)
    {
        throw Exception("Invalid coordinates in getTexelColor");
    }
    if (!_resourceBuffer)
    {
        throw Exception("Invalid resource buffer in getTexelColor");
    }

    if (_baseType == BaseType::FLOAT)
    {
        float* data = static_cast<float*>(_resourceBuffer) + (y * _width + x) * _channelCount;
        if (_channelCount == 4)
        {
            return Color4(data[0], data[1], data[2], data[3]);
        }
        else if (_channelCount == 3)
        {
            return Color4(data[0], data[1], data[2], 1.0f);
        }
        else if (_channelCount == 1)
        {
            return Color4(data[0], data[0], data[0], 1.0f);
        }
        else
        {
            throw Exception("Unsupported channel count in getTexelColor");
        }
    }
    else
    {
        throw Exception("Unsupported base type in getTexelColor");
    }
}

void Image::createResourceBuffer()
{
    releaseResourceBuffer();
    _resourceBuffer = malloc(_width * _height * _channelCount * getBaseStride());
    _resourceBufferDeallocator = nullptr;
}

void Image::releaseResourceBuffer()
{
    if (_resourceBuffer)
    {
        if (_resourceBufferDeallocator)
        {
            _resourceBufferDeallocator(_resourceBuffer);
        }
        else
        {
            free(_resourceBuffer);
        }
        _resourceBuffer = nullptr;
    }
}

//
// ImageHandler methods
//

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
                             ImagePtr image,
                             bool verticalFlip)
{
    FilePath foundFilePath =  _searchPath.find(filePath);
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
            bool saved = iter->second->saveImage(foundFilePath, image, verticalFlip);
            if (saved)
            {
                return true;
            }
        }
    }
    return false;
}

ImagePtr ImageHandler::acquireImage(const FilePath& filePath, bool, const Color4* fallbackColor, string* message)
{
    FilePath foundFilePath =  _searchPath.find(filePath);
    string extension = foundFilePath.getExtension();
    ImageLoaderMap::reverse_iterator iter;
    for (iter = _imageLoaders.rbegin(); iter != _imageLoaders.rend(); ++iter)
    {
        ImageLoaderPtr loader = iter->second;
        if (loader && loader->supportedExtensions().count(extension))
        {
            ImagePtr image = loader->loadImage(foundFilePath);
            if (image)
            {
                cacheImage(foundFilePath, image);
                return image;
            }
        }
    }

    if (message && !filePath.isEmpty())
    {
        *message = string("Error loading image: ") + filePath.asString();
    }
    if (fallbackColor)
    {
        ImagePtr image = Image::createConstantColor(1, 1, *fallbackColor);
        if (image)
        {
            cacheImage(filePath, image);
            return image;
        }
    }

    return nullptr;
}

bool ImageHandler::bindImage(ImagePtr, const ImageSamplingProperties&)
{
    return false;
}

bool ImageHandler::unbindImage(ImagePtr)
{
    return false;
}

void ImageHandler::cacheImage(const string& filePath, ImagePtr image)
{
    if (!_imageCache.count(filePath))
    {
        _imageCache[filePath] = image;
    }
}

ImagePtr ImageHandler::getCachedImage(const FilePath& filePath)
{
    if (_imageCache.count(filePath))
    {
        return _imageCache[filePath];
    }
    if (!filePath.isAbsolute())
    {
        for (const FilePath& path : _searchPath)
        {
            FilePath combined = path / filePath;
            if (_imageCache.count(combined))
            {
                return _imageCache[combined];
            }
        }
    }
    return nullptr;
}

void ImageHandler::deleteImage(ImagePtr image)
{
    image->releaseResourceBuffer();
}

void ImageHandler::clearImageCache()
{
    for (auto iter : _imageCache)
    {
        deleteImage(iter.second);
    }
    _imageCache.clear();
}

//
// ImageSamplingProperties methods
//

void ImageSamplingProperties::setProperties(const string& fileNameUniform,
                                            const VariableBlock& uniformBlock)
{
    const int INVALID_MAPPED_INT_VALUE = -1; // Any value < 0 is not considered to be invalid

    // Get the additional texture parameters based on image uniform name
    // excluding the trailing "_file" postfix string
    string root = fileNameUniform;
    size_t pos = root.find_last_of(IMAGE_PROPERTY_SEPARATOR);
    if (pos != string::npos)
    {
        root = root.substr(0, pos);
    }

    const string uaddressmodeStr = root + UADDRESS_MODE_SUFFIX;
    const ShaderPort* port = uniformBlock.find(uaddressmodeStr);
    ValuePtr intValue = port ? port->getValue() : nullptr;
    uaddressMode = ImageSamplingProperties::AddressMode(intValue && intValue->isA<int>() ? intValue->asA<int>() : INVALID_MAPPED_INT_VALUE);

    const string vaddressmodeStr = root + VADDRESS_MODE_SUFFIX;
    port = uniformBlock.find(vaddressmodeStr);
    intValue = port ? port->getValue() : nullptr;
    vaddressMode = ImageSamplingProperties::AddressMode(intValue && intValue->isA<int>() ? intValue->asA<int>() : INVALID_MAPPED_INT_VALUE);

    const string filtertypeStr = root + FILTER_TYPE_SUFFIX;
    port = uniformBlock.find(filtertypeStr);
    intValue = port ? port->getValue() : nullptr;
    filterType = ImageSamplingProperties::FilterType(intValue && intValue->isA<int>() ? intValue->asA<int>() : INVALID_MAPPED_INT_VALUE);

    const string defaultColorStr = root + DEFAULT_COLOR_SUFFIX;
    port = uniformBlock.find(defaultColorStr);
    ValuePtr colorValue = port ? port->getValue() : nullptr;
    if (colorValue)
    {
        mapValueToColor(colorValue, defaultColor);
    }
}

} // namespace MaterialX
