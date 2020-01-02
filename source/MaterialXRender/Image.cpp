//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRender/Image.h>

#include <MaterialXRender/Types.h>

namespace MaterialX
{

//
// Image methods
//

Image::Image(unsigned int width, unsigned int height, unsigned int channelCount, BaseType baseType) :
    _width(width),
    _height(height),
    _channelCount(channelCount),
    _baseType(baseType),
    _resourceBuffer(nullptr),
    _resourceBufferDeallocator(nullptr),
    _resourceId(0)
{
}

Image::~Image()
{
    releaseResourceBuffer();
}

unsigned int Image::getBaseStride() const
{
    if (_baseType == BaseType::FLOAT)
    {
        return 4;
    }
    if (_baseType == BaseType::HALF)
    {
        return 2;
    }
    if (_baseType == BaseType::UINT8)
    {
        return 1;
    }
    throw Exception("Unsupported base type in getBaseStride");
}

unsigned int Image::getMaxMipCount() const
{
    return (unsigned int) std::log2(std::max(_width, _height)) + 1;
}

ImagePtr Image::createConstantColor(unsigned int width, unsigned int height, const Color4& color)
{
    unsigned int channelCount = 4;
    size_t bufferSize = width * height * channelCount;
    if (!bufferSize)
    {
        return nullptr;
    }

    ImagePtr image = create(width, height, channelCount, Image::BaseType::FLOAT);
    image->createResourceBuffer();
    float* pixel = static_cast<float*>(image->getResourceBuffer());
    for (size_t i = 0; i < image->getWidth(); i++)
    {
        for (size_t j = 0; j < image->getHeight(); j++)
        {
            for (unsigned int c = 0; c < image->getChannelCount(); c++)
            {
                *pixel++ = color[c];
            }
        }
    }
    return image;
}

void Image::setTexelColor(unsigned int x, unsigned int y, const Color4& color)
{
    if (x >= _width || y >= _height)
    {
        throw Exception("Invalid coordinates in setTexelColor");
    }
    if (!_resourceBuffer)
    {
        throw Exception("Invalid resource buffer in setTexelColor");
    }

    unsigned int writeChannels = std::min(_channelCount, (unsigned int) 4);
    if (_baseType == BaseType::FLOAT)
    {
        float* data = static_cast<float*>(_resourceBuffer) + (y * _width + x) * _channelCount;
        for (unsigned int c = 0; c < writeChannels; c++)
        {
            data[c] = color[c];
        }
    }
    else if (_baseType == BaseType::HALF)
    {
        Half* data = static_cast<Half*>(_resourceBuffer) + (y * _width + x) * _channelCount;
        for (unsigned int c = 0; c < writeChannels; c++)
        {
            data[c] = (Half) color[c];
        }
    }
    else
    {
        throw Exception("Unsupported base type in setTexelColor");
    }
}

Color4 Image::getTexelColor(unsigned int x, unsigned int y) const
{
    if (x >= _width || y >= _height)
    {
        throw Exception("Invalid coordinates in getTexelColor");
    }
    if (!_resourceBuffer)
    {
        throw Exception("Invalid resource buffer in getTexelColor");
    }

    if (_baseType == BaseType::FLOAT)
    {
        float* data = static_cast<float*>(_resourceBuffer) + (y * _width + x) * _channelCount;
        if (_channelCount == 4)
        {
            return Color4(data[0], data[1], data[2], data[3]);
        }
        else if (_channelCount == 3)
        {
            return Color4(data[0], data[1], data[2], 1.0f);
        }
        else if (_channelCount == 1)
        {
            return Color4(data[0], data[0], data[0], 1.0f);
        }
        else
        {
            throw Exception("Unsupported channel count in getTexelColor");
        }
    }
    else if (_baseType == BaseType::HALF)
    {
        Half* data = static_cast<Half*>(_resourceBuffer) + (y * _width + x) * _channelCount;
        if (_channelCount == 4)
        {
            return Color4(data[0], data[1], data[2], data[3]);
        }
        else if (_channelCount == 3)
        {
            return Color4(data[0], data[1], data[2], 1.0f);
        }
        else if (_channelCount == 1)
        {
            return Color4(data[0], data[0], data[0], 1.0f);
        }
        else
        {
            throw Exception("Unsupported channel count in getTexelColor");
        }
    }
    else
    {
        throw Exception("Unsupported base type in getTexelColor");
    }
}

void Image::createResourceBuffer()
{
    releaseResourceBuffer();
    _resourceBuffer = malloc(_width * _height * _channelCount * getBaseStride());
    _resourceBufferDeallocator = nullptr;
}

void Image::releaseResourceBuffer()
{
    if (_resourceBuffer)
    {
        if (_resourceBufferDeallocator)
        {
            _resourceBufferDeallocator(_resourceBuffer);
        }
        else
        {
            free(_resourceBuffer);
        }
        _resourceBuffer = nullptr;
    }
}

} // namespace MaterialX
