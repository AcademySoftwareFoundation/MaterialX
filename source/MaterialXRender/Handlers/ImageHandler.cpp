#include <MaterialXCore/Types.h>
#include <MaterialXGenShader/Util.h>
#include <MaterialXRender/Handlers/ImageHandler.h>
#include <cmath>

namespace MaterialX
{
std::string ImageLoader::BMP_EXTENSION = "bmp";
std::string ImageLoader::EXR_EXTENSION = "exr";
std::string ImageLoader::GIF_EXTENSION = "gif";
std::string ImageLoader::HDR_EXTENSION = "hdr";
std::string ImageLoader::JPG_EXTENSION = "jpg";
std::string ImageLoader::JPEG_EXTENSION = "jpeg";
std::string ImageLoader::PIC_EXTENSION = "pic";
std::string ImageLoader::PNG_EXTENSION = "png";
std::string ImageLoader::PSD_EXTENSION = "psd";
std::string ImageLoader::TGA_EXTENSION = "tga";

ImageHandler::ImageHandler(ImageLoaderPtr imageLoader)
{
    addLoader(imageLoader);
}

void ImageHandler::addLoader(ImageLoaderPtr loader)
{
    const StringVec& extensions = loader->supportedExtensions();
    for (auto extension : extensions)
    {
        _imageLoaders.insert(std::pair<std::string, ImageLoaderPtr>(extension, loader));
    }
}

bool ImageHandler::saveImage(const std::string& fileName,
                            const ImageDesc &imageDesc)
{
    FilePath filePath = findFile(fileName);

    std::pair <ImageLoaderMap::iterator, ImageLoaderMap::iterator> range;
    string extension = MaterialX::getFileExtension(fileName);
    range = _imageLoaders.equal_range(extension);
    ImageLoaderMap::iterator first = --range.second;
    ImageLoaderMap::iterator last = --range.first;
    for (auto it = first; it != last; --it)
    {
        bool saved = it->second->saveImage(filePath, imageDesc);
        if (saved)
        {
            return true;
        }
    }
    return false;
}

bool ImageHandler::acquireImage(const FilePath& fileName, ImageDesc &imageDesc, bool generateMipMaps, const std::array<float, 4>* /*fallbackColor*/)
{
    FilePath filePath = findFile(fileName);

    std::pair <ImageLoaderMap::iterator, ImageLoaderMap::iterator> range;
    string extension = MaterialX::getFileExtension(fileName);
    range = _imageLoaders.equal_range(extension);
    ImageLoaderMap::iterator first = --range.second;
    ImageLoaderMap::iterator last= --range.first;
    for (auto it = first; it != last; --it)
    {
        bool acquired = it->second->acquireImage(filePath, imageDesc, generateMipMaps);
        if (acquired)
        {
            return true;
        }
    }
    return false;
}

bool ImageHandler::createColorImage(const std::array<float,4>& color,
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

void ImageHandler::cacheImage(const std::string& identifier, const ImageDesc& desc)
{
    if (!_imageCache.count(identifier))
    {
        _imageCache[identifier] = desc;
    }
}

void ImageHandler::uncacheImage(const std::string& identifier)
{
    _imageCache.erase(identifier);
}

const ImageDesc* ImageHandler::getCachedImage(const std::string& identifier)
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

FilePath ImageHandler::findFile(const FilePath& filename)
{
    return _searchPath.find(filename);
}


}