//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#if defined(_WIN32)
#pragma warning( push )
#pragma warning( disable: 4244)
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
                                   const ImageDesc& imageDesc,
                                   bool /*verticalFlip*/)
{
    if (imageDesc.baseType != ImageDesc::BASETYPE_FLOAT &&
        imageDesc.baseType != ImageDesc::BASETYPE_HALF)
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

        int saveAsHalf = (imageDesc.baseType == ImageDesc::BASETYPE_HALF) ? 1 : 0;
        returnValue = SaveEXR(static_cast<float*>(imageDesc.resourceBuffer),
                                static_cast<int>(imageDesc.width),
                                static_cast<int>(imageDesc.height), imageDesc.channelCount,
                                saveAsHalf, fileName.c_str());
    }
    return (returnValue == 0);
}

bool TinyEXRImageLoader::loadImage(const FilePath& filePath,
                                   ImageDesc& imageDesc,
                                   const ImageDescRestrictions* restrictions)
{
    // Early out if returning float is unsupported
    if (restrictions && restrictions->supportedBaseTypes.count(ImageDesc::BASETYPE_FLOAT) == 0)
    {
        return false;
    }

    int returnValue = -1;
    imageDesc.width = imageDesc.height = imageDesc.channelCount = 0;
    imageDesc.resourceBuffer = nullptr;

    // Fail with any type other than exr.
    const string& fileName = filePath.asString();
    std::string extension = (fileName.substr(fileName.find_last_of(".") + 1));
    if (extension == EXR_EXTENSION)
    {
        const char* err = nullptr;
        int iwidth = 0;
        int iheight = 0;
        float* buffer = nullptr;
        imageDesc.channelCount = 4;
        returnValue = LoadEXR(&buffer, &iwidth, &iheight, fileName.c_str(), &err);
        if (returnValue == 0)
        {
            imageDesc.resourceBuffer = buffer;
            imageDesc.resourceBufferDeallocator = [](void* buffer)
            {
                free(buffer);
            };
            imageDesc.width = iwidth;
            imageDesc.height = iheight;
            imageDesc.baseType = ImageDesc::BASETYPE_FLOAT;
        }
    }
    imageDesc.computeMipCount();

    return (returnValue == 0);
}

}
