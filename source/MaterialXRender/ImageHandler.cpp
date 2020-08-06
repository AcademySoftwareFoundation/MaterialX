//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRender/ImageHandler.h>

#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Util.h>

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
// ImageHandler methods
//

ImageHandler::ImageHandler(ImageLoaderPtr imageLoader)
{
    addLoader(imageLoader);
    _zeroImage = createUniformImage(1, 1, 4, Image::BaseType::UINT8, Color4(0.0f));
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
                             ConstImagePtr image,
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
    FilePath foundFilePath = _searchPath.find(filePath);
    string extension = stringToLower(foundFilePath.getExtension());
    bool extensionSupported = false;
    for (ImageLoaderMap::reverse_iterator iter = _imageLoaders.rbegin(); iter != _imageLoaders.rend(); ++iter)
    {
        ImageLoaderPtr loader = iter->second;
        if (loader && loader->supportedExtensions().count(extension))
        {
            extensionSupported = true;
            ImagePtr image = loader->loadImage(foundFilePath);
            if (image)
            {
                // Workaround for sampling issues with 1x1 textures.
                if (image->getWidth() == 1 && image->getHeight() == 1)
                {
                    image = createUniformImage(2, 2, image->getChannelCount(),
                                               image->getBaseType(), image->getTexelColor(0, 0));
                }

                cacheImage(foundFilePath, image);
                return image;
            }
        }
    }

    if (message && !filePath.isEmpty())
    {
        if (!foundFilePath.exists())
        {
            *message = string("Image file not found: ") + filePath.asString();
        }
        else if (!extensionSupported)
        {
            *message = string("Unsupported image extension: ") + filePath.asString();
        }
        else
        {
            *message = string("Image loader failed to parse image: ") + filePath.asString();
        }
    }
    if (fallbackColor)
    {
        ImagePtr image = createUniformImage(2, 2, 4, Image::BaseType::UINT8, *fallbackColor);
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

void ImageHandler::unbindImages()
{
    for (auto iter : _imageCache)
    {
        unbindImage(iter.second);
    }
}

bool ImageHandler::createRenderResources(ImagePtr, bool)
{
    return false;
}

void ImageHandler::releaseRenderResources(ImagePtr)
{
}

void ImageHandler::cacheImage(const string& filePath, ImagePtr image)
{
    _imageCache[filePath] = image;
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

void ImageHandler::clearImageCache()
{
    for (auto iter : _imageCache)
    {
        releaseRenderResources(iter.second);
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
