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

bool OiioImageLoader::saveImage(const FilePath& filePath,
                                const ImageDesc &imageDesc,
                                bool verticalFlip)
{
    OIIO::ImageSpec imageSpec;
    imageSpec.width = imageDesc.width;
    imageSpec.height = imageDesc.height;
    imageSpec.nchannels = imageDesc.channelCount;

    unsigned int byteCount = 1;
    OIIO::TypeDesc format = OIIO::TypeDesc::UINT8;
    if (imageDesc.baseType == ImageDesc::BASETYPE_FLOAT)
    {
        format = OIIO::TypeDesc::FLOAT;
        byteCount = 4;
    }
    else if (imageDesc.baseType == ImageDesc::BASETYPE_HALF)
    {
        format = OIIO::TypeDesc::HALF;
        byteCount = 2;
    }
    else if (imageDesc.baseType != ImageDesc::BASETYPE_UINT8)
    {
        return false;
    }

    bool written = false;
    OIIO::ImageOutput* imageOutput = OIIO::ImageOutput::create(filePath);
    if (imageOutput)
    {
        if (imageOutput->open(filePath, imageSpec))
        {
            if (verticalFlip)
            {
                int scanlinesize = imageDesc.width * imageDesc.channelCount * byteCount;
                written = imageOutput->write_image(
                            format, 
                            static_cast<char *>(imageDesc.resourceBuffer) + (imageDesc.height - 1) * scanlinesize,
                            OIIO::AutoStride, // default x stride
                            static_cast<OIIO::stride_t>(-scanlinesize), // special y stride
                            OIIO::AutoStride);
            }
            else
            {
                written = imageOutput->write_image(
                            format,
                            imageDesc.resourceBuffer);
            }
            imageOutput->close();
        }
    }
    return written;
}

bool OiioImageLoader::acquireImage(const FilePath& filePath,
                                  ImageDesc& imageDesc,
                                  const ImageDescRestrictions* restrictions)
{
    imageDesc.width = imageDesc.height = imageDesc.channelCount = 0;
    imageDesc.resourceBuffer = nullptr;

    OIIO::ImageInput* imageInput = OIIO::ImageInput::open(filePath);
    if (!imageInput)
    {
        return false;
    }

    OIIO::ImageSpec imageSpec = imageInput->spec();
    switch (imageSpec.format.basetype)
    {
        case OIIO::TypeDesc::UINT8:
        {
            imageDesc.baseType = ImageDesc::BASETYPE_UINT8;
            break;
        }
        case OIIO::TypeDesc::FLOAT:
        {
            imageDesc.baseType = ImageDesc::BASETYPE_FLOAT;
            break;
        }
        case OIIO::TypeDesc::HALF:
        {
            // If 16-bit float is not support try loading in 32-bit float.
            if (restrictions && restrictions->supportedBaseTypes.count(ImageDesc::BASETYPE_HALF) == 0)
            {
                imageSpec.set_format(OIIO::TypeDesc::FLOAT);
                imageDesc.baseType = ImageDesc::BASETYPE_FLOAT;
            }
            else
            {
                imageDesc.baseType = ImageDesc::BASETYPE_HALF;
            }
            break;
        }
        default:
            break;
    };

    bool read = false;
    // Check if base type is unsupported before trying to load
    if (!restrictions || restrictions->supportedBaseTypes.count(imageDesc.baseType))
    {
        imageDesc.width = imageSpec.width;
        imageDesc.height = imageSpec.height;
        imageDesc.channelCount = imageSpec.nchannels;
        imageDesc.computeMipCount();

        size_t imageBytes = (size_t)imageSpec.image_bytes();
        void* imageBuf = malloc(imageBytes);
        if (imageInput->read_image(imageSpec.format, imageBuf))
        {
            imageDesc.resourceBuffer = imageBuf;
            read = true;
        }
    }

    imageInput->close();
    return read;
}

} // namespace MaterialX

