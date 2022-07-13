//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRender/ImageHandler.h>

#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Util.h>

#include <iostream>

MATERIALX_NAMESPACE_BEGIN

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
// ImageLoader methods
//

bool ImageLoader::saveImage(const FilePath&, ConstImagePtr, bool)
{
    return false;
}

ImagePtr ImageLoader::loadImage(const FilePath&)
{
    return nullptr;
}

//
// ImageHandler methods
//

ImageHandler::ImageHandler(ImageLoaderPtr imageLoader)
{
    addLoader(imageLoader);
    _zeroImage = createUniformImage(2, 2, 4, Image::BaseType::UINT8, Color4(0.0f));

    // Generated shaders interpret 1x1 textures as invalid images.
    _invalidImage = createUniformImage(1, 1, 4, Image::BaseType::UINT8, Color4(0.0f));
}

void ImageHandler::addLoader(ImageLoaderPtr loader)
{
    if (loader)
    {
        const StringSet& extensions = loader->supportedExtensions();
        for (const auto& extension : extensions)
        {
            _imageLoaders[extension].push_back(loader);
        }
    }
}

StringSet ImageHandler::supportedExtensions()
{
    StringSet extensions;
    for (const auto& pair : _imageLoaders)
    {
        extensions.insert(pair.first);
    }
    return extensions;
}

bool ImageHandler::saveImage(const FilePath& filePath,
                             ConstImagePtr image,
                             bool verticalFlip)
{
    if (!image)
    {
        return false;
    }

    FilePath foundFilePath = _searchPath.find(filePath);
    if (foundFilePath.isEmpty())
    {
        return false;
    }

    string extension = foundFilePath.getExtension();
    for (ImageLoaderPtr loader : _imageLoaders[extension])
    {
        bool saved = false;
        try
        {
            saved = loader->saveImage(foundFilePath, image, verticalFlip);
        }
        catch (std::exception& e)
        {
            std::cerr << "Exception in image I/O library: " << e.what() << std::endl;
        }
        if (saved)
        {
            return true;
        }
    }
    return false;
}

ImagePtr ImageHandler::acquireImage(const FilePath& filePath)
{
    // Resolve the input filepath.
    FilePath resolvedFilePath = filePath;
    if (_resolver)
    {
        resolvedFilePath = _resolver->resolve(resolvedFilePath, FILENAME_TYPE_STRING);
    }

    // Return a cached image if available.
    ImagePtr cachedImage = getCachedImage(resolvedFilePath);
    if (cachedImage)
    {
        return cachedImage;
    }

    // Load and cache the requested image.
    ImagePtr image = loadImage(_searchPath.find(resolvedFilePath));
    if (image)
    {
        cacheImage(resolvedFilePath, image);
        return image;
    }

    // No valid image was found, so cache the sentinel invalid image.
    cacheImage(resolvedFilePath, _invalidImage);
    return _invalidImage;
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

ImageVec ImageHandler::getReferencedImages(DocumentPtr doc)
{
    ImageVec imageVec;
    for (ElementPtr elem : doc->traverseTree())
    {
        if (elem->getActiveSourceUri() != doc->getSourceUri())
        {
            continue;
        }

        NodePtr node = elem->asA<Node>();
        InputPtr file = node ? node->getInput("file") : nullptr;
        if (file)
        {
            ImagePtr image = acquireImage(file->getResolvedValueString());
            if (image && image != _invalidImage)
            {
                imageVec.push_back(image);
            }
        }
    }
    return imageVec;
}

ImagePtr ImageHandler::loadImage(const FilePath& filePath)
{
    string extension = stringToLower(filePath.getExtension());
    for (ImageLoaderPtr loader : _imageLoaders[extension])
    {
        ImagePtr image;
        try
        {
            image = loader->loadImage(filePath);
        }
        catch (std::exception& e)
        {
            std::cerr << "Exception in image I/O library: " << e.what() << std::endl;
        }
        if (image)
        {
            // Generated shaders interpret 1x1 textures as invalid images, so valid 1x1
            // images must be resized.
            if (image->getWidth() == 1 && image->getHeight() == 1)
            {
                image = createUniformImage(2, 2, image->getChannelCount(),
                                           image->getBaseType(), image->getTexelColor(0, 0));
            }

            return image;
        }
    }

    if (!filePath.isEmpty())
    {
        if (!filePath.exists())
        {
            std::cerr << string("Image file not found: ") + filePath.asString() << std::endl;
        }
        else if (!_imageLoaders.count(extension))
        {
            std::cerr << string("Unsupported image extension: ") + filePath.asString() << std::endl;
        }
        else
        {
            std::cerr << string("Image loader failed to parse image: ") + filePath.asString() << std::endl;
        }
    }

    return nullptr;
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

MATERIALX_NAMESPACE_END
