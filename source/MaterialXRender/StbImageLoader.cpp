//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#if defined(_WIN32)
    #pragma warning(push)
    #pragma warning(disable: 4100)
    #pragma warning(disable: 4505)
#endif

// Make the functions static to avoid multiple definitions if other libraries
// are also using stb
#define STB_IMAGE_STATIC 1

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <MaterialXRender/External/StbImage/stb_image_write.h>

#define STB_IMAGE_IMPLEMENTATION
#include <MaterialXRender/External/StbImage/stb_image.h>

#if defined(_WIN32)
    #pragma warning(pop)
#endif

#include <MaterialXRender/StbImageLoader.h>

namespace MaterialX
{

bool StbImageLoader::saveImage(const FilePath& filePath,
                               ConstImagePtr image,
                               bool verticalFlip)
{
    bool isChar = image->getBaseType() == Image::BaseType::UINT8;
    bool isFloat = image->getBaseType() == Image::BaseType::FLOAT;
    if (!isChar && !isFloat)
    {
        return false;
    }

    int returnValue = -1;

    // Set global "flip" flag
    int prevFlip = stbi__flip_vertically_on_write;
    stbi__flip_vertically_on_write = verticalFlip ? 1 : 0;

    int w = static_cast<int>(image->getWidth());
    int h = static_cast<int>(image->getHeight());
    int channels = static_cast<int>(image->getChannelCount());
    void* data = image->getResourceBuffer();

    const string filePathName = filePath.asString();

    string extension = filePath.getExtension();
    if (!isFloat)
    {
        if (extension == PNG_EXTENSION)
        {
            returnValue = stbi_write_png(filePathName.c_str(), w, h, channels, data, w * 4);
        }
        else if (extension == BMP_EXTENSION)
        {
            returnValue = stbi_write_bmp(filePathName.c_str(), w, h, channels, data);
        }
        else if (extension == TGA_EXTENSION)
        {
            returnValue = stbi_write_tga(filePathName.c_str(), w, h, channels, data);
        }
        else if (extension == JPG_EXTENSION || extension == JPEG_EXTENSION)
        {
            returnValue = stbi_write_jpg(filePathName.c_str(), w, h, channels, data, 100);
        }
    }
    else
    {
        if (extension == HDR_EXTENSION)
        {
            returnValue = stbi_write_hdr(filePathName.c_str(), w, h, channels, static_cast<float*>(data));
        }
    }

    if (verticalFlip)
    {
        stbi__flip_vertically_on_write = prevFlip;
    }
    return (returnValue == 1);
}

ImagePtr StbImageLoader::loadImage(const FilePath& filePath)
{
    int width = 0;
    int height = 0;
    int channelCount = 0;
    Image::BaseType baseType = Image::BaseType::UINT8;
    void *buffer = nullptr;

    // Select standard or float reader based on file extension.
    string extension = filePath.getExtension();
    if (extension == HDR_EXTENSION)
    {
        buffer = stbi_loadf(filePath.asString().c_str(), &width, &height, &channelCount, 0);
        baseType = Image::BaseType::FLOAT;
    }
    else
    {
        buffer = stbi_load(filePath.asString().c_str(), &width, &height, &channelCount, 0);
        baseType = Image::BaseType::UINT8;
    }
    if (!buffer)
    {
        return nullptr;
    }

    // Create the image object.
    ImagePtr image = Image::create(width, height, channelCount, baseType);
    image->setResourceBuffer(buffer);
    image->setResourceBufferDeallocator(&stbi_image_free);
    return image;
}

} // namespace MaterialX
