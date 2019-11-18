//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRender/OiioImageLoader.h>

#if defined(_WIN32)
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

#if defined(_WIN32)
#pragma warning( pop )
#elif defined(__clang__)
#pragma clang diagnostic pop
#else
#pragma GCC diagnostic pop
#endif

namespace MaterialX
{

bool OiioImageLoader::saveImage(const FilePath& filePath,
                                ImagePtr image,
                                bool verticalFlip)
{
    OIIO::ImageSpec imageSpec;
    imageSpec.width = image->getWidth();
    imageSpec.height = image->getHeight();
    imageSpec.nchannels = image->getChannelCount();

    unsigned int byteCount = 1;
    OIIO::TypeDesc format = OIIO::TypeDesc::UINT8;
    if (image->getBaseType() == Image::BaseType::FLOAT)
    {
        format = OIIO::TypeDesc::FLOAT;
        byteCount = 4;
    }
    else if (image->getBaseType() == Image::BaseType::HALF)
    {
        format = OIIO::TypeDesc::HALF;
        byteCount = 2;
    }
    else if (image->getBaseType() != Image::BaseType::UINT8)
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
                int scanlinesize = image->getWidth() * image->getChannelCount() * byteCount;
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
                            image->getResourceBuffer());
            }
            imageOutput->close();
        }
    }
    return written;
}

ImagePtr OiioImageLoader::loadImage(const FilePath& filePath)
{
    OIIO::ImageInput* imageInput = OIIO::ImageInput::open(filePath);
    if (!imageInput)
    {
        return nullptr;
    }

    OIIO::ImageSpec imageSpec = imageInput->spec();
    Image::BaseType baseType = Image::BaseType::UINT8;
    switch (imageSpec.format.basetype)
    {
        case OIIO::TypeDesc::UINT8:
        {
            baseType = Image::BaseType::UINT8;
            break;
        }
        case OIIO::TypeDesc::FLOAT:
        {
            baseType = Image::BaseType::FLOAT;
            break;
        }
        case OIIO::TypeDesc::HALF:
        {
            baseType = Image::BaseType::HALF;
            break;
        }
        default:
            break;
    };

    ImagePtr image = Image::create(imageSpec.width, imageSpec.height, imageSpec.nchannel, baseType);
    size_t imageBytes = (size_t) imageSpec.image_bytes();
    void* imageBuf = malloc(imageBytes);
    if (imageInput->read_image(imageSpec.format, imageBuf))
    {
        image->setResourceBuffer(imageBuf);
    }
    else
    {
        image = nullptr;
    }
    imageInput->close();

    return image;
}

} // namespace MaterialX
