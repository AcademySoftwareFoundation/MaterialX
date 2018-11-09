#include <MaterialXRender/Window/HardwarePlatform.h>

#if defined(OSWin_) && defined(_WIN64)
#define TINYEXR_USABLE
#endif
#if defined(OSMac_) || defined(OSLinux)
#define TINYEXR_USABLE
#endif

#if defined(TINYEXR_USABLE)
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
#include <MaterialXRender/External/tinyexr/tinyexr.h>
#ifdef max_cache 
#define max max_cache
#endif
#endif

#include <MaterialXRender/Handlers/TinyEXRImageLoader.h>

namespace MaterialX
{
std::string TinyEXRImageLoader::EXR_EXTENSION("exr");

#if defined(TINYEXR_USABLE)

bool TinyEXRImageLoader::saveImage(const std::string& fileName,
                                    const ImageDesc& imageDesc)
{
    int returnValue = -1;
    // Fail with any type other than exr.
    std::string extension = (fileName.substr(fileName.find_last_of(".") + 1));
    if (extension == EXR_EXTENSION)
    { 
        returnValue = SaveEXR(imageDesc.resourceBuffer, static_cast<int>(imageDesc.width), static_cast<int>(imageDesc.height), imageDesc.channelCount, 1 /* save as 16 bit float format */, fileName.c_str());
    }
    return (returnValue == 0);
}

bool TinyEXRImageLoader::acquireImage(const std::string& fileName,
                                      ImageDesc& imageDesc,
                                      bool /*generateMipMaps*/)
{
    int returnValue = -1;
    imageDesc.width = imageDesc.height = imageDesc.channelCount = 0;
    imageDesc.resourceBuffer = nullptr;

    // Fail with any type other than exr.
    std::string extension = (fileName.substr(fileName.find_last_of(".") + 1));
    if (extension == EXR_EXTENSION)
    {
        const char* err = nullptr;
        int iwidth = 0;
        int iheight = 0;
        imageDesc.channelCount = 4;
        returnValue = LoadEXR(&(imageDesc.resourceBuffer), &iwidth, &iheight, fileName.c_str(), &err);
        if (returnValue == 0)
        {
            imageDesc.width = iwidth;
            imageDesc.height = iheight;
        }
    }
    imageDesc.computeMipCount();

    return (returnValue == 0);
}


#else
bool TinyEXRImageLoader::saveImage(const std::string& /*fileName*/,
                                    const ImageDesc& /*imageDesc*/)
{
    return false;
}

bool TinyEXRImageLoader::acquireImage(const std::string& /*fileName*/,
                                      ImageDesc& /*imageDesc*/, 
                                      bool /*generateMipMaps*/)
{
    return false;
}

#endif
} 

