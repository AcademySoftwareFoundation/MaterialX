//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRender/HardwarePlatform.h>

#if defined(OSWin_) || defined(_WIN32)
    #pragma warning( push )
    #pragma warning( disable: 4100)
    #pragma warning( disable: 4505)
#elif defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused-function"
#else
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-function"
#endif

// Make the functions static to avoid multiple definitions if other libraries
// are also using stb
#define STB_IMAGE_STATIC 1

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <MaterialXRender/External/StbImage/stb_image_write.h>

#define STB_IMAGE_IMPLEMENTATION
#include <MaterialXRender/External/StbImage/stb_image.h>

#if defined(OSWin_) || defined(_WIN32)
    #pragma warning( pop ) 
#elif defined(__clang__)
    #pragma clang diagnostic pop
#else
    #pragma GCC diagnostic pop
#endif


#include <MaterialXRender/Handlers/StbImageLoader.h>

namespace MaterialX
{
bool StbImageLoader::saveImage(const FilePath& filePath,
                               const ImageDesc& imageDesc,
                               bool verticalFlip)
{
    bool isChar = imageDesc.baseType == ImageDesc::BASETYPE_UINT8;
    bool isFloat = imageDesc.baseType == ImageDesc::BASETYPE_FLOAT;
    if (!isChar && !isFloat)
    {
        return false;
    }

    int returnValue = -1;

    // Set global "flip" flag
    int prevFlip = stbi__flip_vertically_on_write;
    stbi__flip_vertically_on_write = verticalFlip ? 1 : 0;

    int w = static_cast<int>(imageDesc.width);
    int h = static_cast<int>(imageDesc.height);
    int channels = static_cast<int>(imageDesc.channelCount);
    void* data = imageDesc.resourceBuffer;

    const string filePathName = filePath.asString();

    std::string extension = (filePathName.substr(filePathName.find_last_of(".") + 1));
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

bool StbImageLoader::acquireImage(const FilePath& filePath, ImageDesc &imageDesc,
                                  const ImageDescRestrictions* restrictions) 
{
    imageDesc.width = imageDesc.height = imageDesc.channelCount = 0;
    imageDesc.resourceBuffer = nullptr;

    int iwidth = 0;
    int iheight = 0;
    int ichannelCount = 0;
    void *buffer = nullptr;

    // Set to 0 to mean to not override the read-in number of channels
    const int REQUIRED_CHANNEL_COUNT = 0;

    const string fileName = filePath.asString();

    // If HDR, switch to float reader
    std::string extension = (fileName.substr(fileName.find_last_of(".") + 1));
    if (extension == HDR_EXTENSION)
    {
        // Early out if base type is unsupported
        if (restrictions && restrictions->supportedBaseTypes.count(ImageDesc::BASETYPE_FLOAT) == 0)
        {
            return false;
        }
        buffer = stbi_loadf(fileName.c_str(), &iwidth, &iheight, &ichannelCount, REQUIRED_CHANNEL_COUNT);
        imageDesc.baseType = ImageDesc::BASETYPE_FLOAT;
    }
    // Otherwise use fixed point reader
    else
    {
        // Early out if base type is unsupported
        if (restrictions && restrictions->supportedBaseTypes.count(ImageDesc::BASETYPE_UINT8) == 0)
        {
            return false;
        }
        buffer = stbi_load(fileName.c_str(), &iwidth, &iheight, &ichannelCount, REQUIRED_CHANNEL_COUNT);
        imageDesc.baseType = ImageDesc::BASETYPE_UINT8;
    }
    if (buffer)
    {
        imageDesc.resourceBuffer = buffer;
        imageDesc.width = iwidth;
        imageDesc.height = iheight;
        imageDesc.channelCount = ichannelCount;
        imageDesc.computeMipCount();
    }
    return (imageDesc.resourceBuffer != nullptr);
}

}



