
// Restrict to only run on Windows 
#if defined(_WIN64) || defined(_WIN32)
#if defined(_WIN64)
#define TINYEXR_USABLE
#endif
#else
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
#include <MaterialXView/External/tinyexr/tinyexr.h>
#ifdef max_cache 
#define max max_cache
#endif
#endif

#include <MaterialXView/Image/TinyEXRImageHandler.h>

namespace MaterialX
{
#if defined(TINYEXR_USABLE)
bool TinyEXRImageHandler::saveImage(const std::string& fileName,
                                    const std::string& extension, 
                                    unsigned int width, 
                                    unsigned int height, 
                                    unsigned int channelCount, 
                                    const float* buffer)
{
    int returnValue = -1;
    // Fail with any type other than exr.
    if (extension == "exr")
    { 
        returnValue = SaveEXR(buffer, width, height, channelCount, 1 /* save as 16 bit float format */, fileName.c_str());
    }
    return (returnValue == 0);
}
#else
    bool TinyEXRImageHandler::saveImage(const std::string& /*fileName*/,
        const std::string& /*extension*/,
        unsigned int /*width*/,
        unsigned int /*height*/,
        unsigned int /*channelCount*/,
        const float* /*buffer*/)
    {
        return false;
    }
#endif
} 

