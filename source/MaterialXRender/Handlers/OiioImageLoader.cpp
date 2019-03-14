//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRender/Handlers/OiioImageLoader.h>

#if defined(OSWin_) || defined(_WIN32)
#pragma warning( push )
#pragma warning( disable: 4100)
#pragma warning( disable: 4505)
#pragma warning( disable: 4800)
#pragma warning( disable: 4244)
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#include <OpenImageIO/imageio.h>


#if defined(OSWin_) || defined(_WIN32)
#pragma warning( pop ) 
#elif defined(__clang__)
#pragma clang diagnostic pop
#else
#pragma GCC diagnostic pop
#endif

namespace MaterialX
{

bool OiioImageLoader::saveImage(const std::string& /*fileName*/,
                                const ImageDesc &/*imageDesc*/)
{
    throw Exception("Unimplemented method OiioImageLoader::saveImage.");
}

bool OiioImageLoader::acquireImage(const std::string& fileName,
                                  ImageDesc& imageDesc,
                                  bool /*generateMipMaps*/)
{
    imageDesc.width = imageDesc.height = imageDesc.channelCount = 0;
    imageDesc.resourceBuffer = nullptr;

    OIIO::ImageInput* imageInput = OIIO::ImageInput::open(fileName);
    if (!imageInput)
    {
        return false;
    }

    const OIIO::ImageSpec& imageSpec = imageInput->spec();
    if (imageSpec.format != OIIO::TypeDesc::UINT8 &&
        imageSpec.format != OIIO::TypeDesc::FLOAT)
    {
        return false;
    }

    imageDesc.width = imageSpec.width;
    imageDesc.height = imageSpec.height;
    imageDesc.channelCount = imageSpec.nchannels;
    imageDesc.floatingPoint = (imageSpec.format == OIIO::TypeDesc::FLOAT);
    imageDesc.computeMipCount();

    size_t imageBytes = (size_t) imageSpec.image_bytes();
    void* imageBuf = malloc(imageBytes);
    imageInput->read_image(imageSpec.format, imageBuf);
    imageDesc.resourceBuffer = imageBuf;
    
    return true;
}

} // namespace MaterialX

