//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#if defined(_WIN32)
#pragma warning( push )
#pragma warning( disable: 4244)
#pragma warning( disable: 5208)
#endif

#define TINYEXR_IMPLEMENTATION
#include <cstdlib>
#include <cstdio>
#include <limits>
#include <stdio.h>
#include <string.h>
// Max may be defined in a macro so temporariy undef it.
#ifdef max
#define max_cache max
#undef max
#endif
#include <MaterialXContrib/External/tinyexr/tinyexr.h>
#ifdef max_cache
#define max max_cache
#endif

#if defined(_WIN32)
#pragma warning( pop )
#endif

#include <MaterialXContrib/Handlers/TinyEXRImageLoader.h>

namespace MaterialX
{
bool TinyEXRImageLoader::saveImage(const FilePath& filePath,
                                   ConstImagePtr image,
                                   bool /*verticalFlip*/)
{
    Image::BaseType baseType = image->getBaseType();
    if (baseType != Image::BaseType::FLOAT &&
        baseType != Image::BaseType::HALF)
    {
        return false;
    }

    int returnValue = -1;
    // Fail with any type other than exr.
    const string& fileName = filePath.asString();
    std::string extension = (fileName.substr(fileName.find_last_of(".") + 1));
    if (extension == EXR_EXTENSION)
    {
        // TODO: vertical flip is unsupported currently in loader

        int saveAsHalf = (baseType == Image::BaseType::HALF) ? 1 : 0;
        returnValue = SaveEXR(static_cast<float*>(image->getResourceBuffer()),
                                static_cast<int>(image->getWidth()),
                                static_cast<int>(image->getHeight()), image->getChannelCount(),
                                saveAsHalf, fileName.c_str());
    }
    return (returnValue == 0);
}

ImagePtr TinyEXRImageLoader::loadImage(const FilePath& filePath)
{
    // Fail with any type other than exr.
    const string& fileName = filePath.asString();
    std::string extension = (fileName.substr(fileName.find_last_of(".") + 1));
    if (extension != EXR_EXTENSION)
    {
        return nullptr;
    }

    const char* err = nullptr;
    int width = 0;
    int height = 0;
    unsigned int channelCount = 4;
    Image::BaseType baseType = Image::BaseType::FLOAT;
    float* buffer = nullptr;

    int returnValue = LoadEXR(&buffer, &width, &height, fileName.c_str(), &err);
    if (returnValue != 0)
    {
        return nullptr;
    }

    // Create the image object
    ImagePtr image = Image::create(width, height, channelCount, baseType);
    image->setResourceBuffer(buffer);
    image->setResourceBufferDeallocator([](void* buffer)
    {
        free(buffer);
    });
    return image;
}

}
