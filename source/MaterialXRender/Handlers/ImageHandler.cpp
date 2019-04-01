//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Types.h>
#include <MaterialXGenShader/Util.h>
#include <MaterialXRender/Handlers/ImageHandler.h>
#include <cmath>

namespace MaterialX
{
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
        for (auto extension : extensions)
        {
            _imageLoaders.insert(std::pair<string, ImageLoaderPtr>(extension, loader));
        }
    }
}

void ImageHandler::supportedExtensions(StringSet& extensions)
{
    extensions.clear();
    for (auto loader : _imageLoaders)
    {
        const StringSet& loaderExtensions = loader.second->supportedExtensions();
        extensions.insert(loaderExtensions.begin(), loaderExtensions.end());
    }
}

bool ImageHandler::saveImage(const FilePath& filePath,
                            const ImageDesc &imageDesc)
{
    FilePath foundFilePath = findFile(filePath);

    std::pair <ImageLoaderMap::iterator, ImageLoaderMap::iterator> range;
    string extension = MaterialX::getFileExtension(foundFilePath);
    range = _imageLoaders.equal_range(extension);
    ImageLoaderMap::iterator first = --range.second;
    ImageLoaderMap::iterator last = --range.first;
    for (auto it = first; it != last; --it)
    {
        bool saved = it->second->saveImage(foundFilePath, imageDesc);
        if (saved)
        {
            return true;
        }
    }
    return false;
}

bool ImageHandler::acquireImage(const FilePath& filePath, ImageDesc &imageDesc, bool generateMipMaps, const Color4* /*fallbackColor*/)
{
    FilePath foundFilePath = findFile(filePath);

    std::pair <ImageLoaderMap::iterator, ImageLoaderMap::iterator> range;
    string extension = MaterialX::getFileExtension(foundFilePath);
    range = _imageLoaders.equal_range(extension);
    ImageLoaderMap::iterator first = --range.second;
    ImageLoaderMap::iterator last= --range.first;
    for (auto it = first; it != last; --it)
    {
        bool acquired = it->second->acquireImage(foundFilePath, imageDesc, generateMipMaps);
        if (acquired)
        {
            return true;
        }
    }
    return false;
}

bool ImageHandler::createColorImage(const Color4& color,
                                    ImageDesc& desc)
{
    // Create a solid color image
    //
    desc.resourceBuffer = new float[desc.width * desc.height * desc.channelCount];
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
    return true;
}

bool ImageHandler::bindImage(const string& /*identifier*/, const ImageSamplingProperties& /*samplingProperties*/)
{
    return false;
}

void ImageHandler::cacheImage(const string& identifier, const ImageDesc& desc)
{
    if (!_imageCache.count(identifier))
    {
        _imageCache[identifier] = desc;
    }
}

void ImageHandler::uncacheImage(const string& identifier)
{
    _imageCache.erase(identifier);
}

const ImageDesc* ImageHandler::getCachedImage(const string& identifier)
{
    if (_imageCache.count(identifier))
    {
        return &(_imageCache[identifier]);
    }
    return nullptr;
}

void ImageHandler::setSearchPath(const FileSearchPath& path)
{
    _searchPath = path;
}

FilePath ImageHandler::findFile(const FilePath& filePath)
{
    return _searchPath.find(filePath);
}

void ImageHandler::deleteImage(ImageDesc& imageDesc)
{
    if (imageDesc.resourceBuffer)
    {
        free(imageDesc.resourceBuffer);
        imageDesc.resourceBuffer = nullptr;
    }
}

}