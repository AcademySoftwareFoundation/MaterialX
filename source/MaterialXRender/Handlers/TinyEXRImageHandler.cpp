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

#include <MaterialXRender/Handlers/TinyEXRImageHandler.h>

namespace MaterialX
{
#if defined(TINYEXR_USABLE)
bool TinyEXRImageHandler::saveImage(const std::string& fileName,
                                    unsigned int width, 
                                    unsigned int height, 
                                    unsigned int channelCount, 
                                    const float* buffer)
{
    int returnValue = -1;
    // Fail with any type other than exr.
    std::string extension = (fileName.substr(fileName.find_last_of(".") + 1));
    if (extension == "exr")
    { 
        returnValue = SaveEXR(buffer, width, height, channelCount, 1 /* save as 16 bit float format */, fileName.c_str());
    }
    return (returnValue == 0);
}

bool TinyEXRImageHandler::loadImage(const std::string& fileName,
                                    unsigned int &width,
                                    unsigned int &height,
                                    unsigned int &channelCount,
                                    float** buffer)
{
    int returnValue = -1;
    width = height = channelCount = 0;
    *buffer = nullptr;

    // Fail with any type other than exr.
    std::string extension = (fileName.substr(fileName.find_last_of(".") + 1));
    if (extension == "exr")
    {
        const char* err = nullptr;
        int iwidth = 0;
        int iheight = 0;
        channelCount = 4;
        returnValue = LoadEXR(buffer, &iwidth, &iheight, fileName.c_str(), &err);
        if (returnValue == 0)
        {
            width = iwidth;
            height = iheight;
        }
    }
    return (returnValue == 0);
}


#else
bool TinyEXRImageHandler::saveImage(const std::string& /*fileName*/,
    unsigned int /*width*/,
    unsigned int /*height*/,
    unsigned int /*channelCount*/,
    const float* /*buffer*/)
{
    return false;
}

bool TinyEXRImageHandler::loadImage(const std::string& /*fileName*/,
    unsigned int &/*width*/,
    unsigned int &/*height*/,
    unsigned int &/*channelCount*/,
    float** /*buffer*/)
{
    return false;
}

#endif
} 

