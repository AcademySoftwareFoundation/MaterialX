//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifdef MATERIALX_BUILD_OIIO

#include <MaterialXRender/OiioImageLoader.h>

#if defined(_MSC_VER)
    #define FMT_UNICODE 0
#endif

#include <OpenImageIO/imageio.h>

MATERIALX_NAMESPACE_BEGIN

bool OiioImageLoader::saveImage(const FilePath& filePath,
                                ConstImagePtr image,
                                bool verticalFlip)
{
    OIIO::ImageSpec imageSpec(image->getWidth(), image->getHeight(), image->getChannelCount());
    OIIO::TypeDesc format;
    switch (image->getBaseType())
    {
        case Image::BaseType::UINT8:
            format = OIIO::TypeDesc::UINT8;
            break;
        case Image::BaseType::INT8:
            format = OIIO::TypeDesc::INT8;
            break;
        case Image::BaseType::UINT16:
            format = OIIO::TypeDesc::UINT16;
            break;
        case Image::BaseType::INT16:
            format = OIIO::TypeDesc::INT16;
            break;
        case Image::BaseType::HALF:
            format = OIIO::TypeDesc::HALF;
            break;
        case Image::BaseType::FLOAT:
            format = OIIO::TypeDesc::FLOAT;
            break;
        default:
            return false;
    }

    bool written = false;
    auto imageOutput = OIIO::ImageOutput::create(filePath.asString());
    if (imageOutput)
    {
        if (imageOutput->open(filePath, imageSpec))
        {
            if (verticalFlip)
            {
                int scanlinesize = image->getWidth() * image->getChannelCount() * image->getBaseStride();
                written = imageOutput->write_image(
                    format,
                    static_cast<char*>(image->getResourceBuffer()) + (image->getHeight() - 1) * scanlinesize,
                    OIIO::AutoStride, // default x stride
                    static_cast<OIIO::stride_t>(-scanlinesize), // special y stride
                    OIIO::AutoStride);
            }
            else
            {
                written = imageOutput->write_image(format, image->getResourceBuffer());
            }
            imageOutput->close();
        }
    }
    return written;
}

ImagePtr OiioImageLoader::loadImage(const FilePath& filePath)
{
    auto imageInput = OIIO::ImageInput::open(filePath);
    if (!imageInput)
    {
        return nullptr;
    }

    OIIO::ImageSpec imageSpec = imageInput->spec();
    Image::BaseType baseType;
    switch (imageSpec.format.basetype)
    {
        case OIIO::TypeDesc::UINT8:
            baseType = Image::BaseType::UINT8;
            break;
        case OIIO::TypeDesc::INT8:
            baseType = Image::BaseType::INT8;
            break;
        case OIIO::TypeDesc::UINT16:
            baseType = Image::BaseType::UINT16;
            break;
        case OIIO::TypeDesc::INT16:
            baseType = Image::BaseType::INT16;
            break;
        case OIIO::TypeDesc::HALF:
            baseType = Image::BaseType::HALF;
            break;
        case OIIO::TypeDesc::FLOAT:
            baseType = Image::BaseType::FLOAT;
            break;
        default:
            imageInput->close();
            return nullptr;
    };

    ImagePtr image = Image::create(imageSpec.width, imageSpec.height, imageSpec.nchannels, baseType);
    image->createResourceBuffer();
    if (!imageInput->read_image(0, 0, 0, imageSpec.nchannels, imageSpec.format, image->getResourceBuffer()))
    {
        image = nullptr;
    }
    imageInput->close();

    return image;
}

MATERIALX_NAMESPACE_END

#endif