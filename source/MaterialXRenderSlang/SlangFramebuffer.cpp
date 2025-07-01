//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderSlang/SlangFramebuffer.h>
#include <MaterialXRenderSlang/SlangTypeUtils.h>
#include <MaterialXRenderSlang/SlangContext.h>
#include <MaterialXRender/ShaderRenderer.h>

MATERIALX_NAMESPACE_BEGIN
SlangFramebuffer::SlangFramebuffer(SlangContextPtr context,
                                   unsigned int width, unsigned int height,
                                   unsigned channelCount,
                                   Image::BaseType baseType,
                                   SlangTexturePtr colorTexture,
                                   bool encodeSrgb,
                                   rhi::Format pixelFormat) :
    _context(std::move(context)),
    _device(_context->getDevice()),
    _width(0),
    _height(0),
    _channelCount(channelCount),
    _baseType(baseType),
    _encodeSrgb(encodeSrgb)
{
    if (colorTexture)
        std::tie(_channelCount, _baseType) = getImageConfig(colorTexture->getDesc().format);

    resize(width, height, false, pixelFormat, std::move(colorTexture));
}

SlangFramebuffer::SlangFramebuffer(SlangContextPtr context,
                                   SlangTexturePtr colorTexture) :
    _context(std::move(context)),
    _device(_context->getDevice()),
    _width(0),
    _height(0),
    _channelCount(0),
    _baseType(Image::BaseType::UINT8),
    _encodeSrgb(false)
{
    std::tie(_channelCount, _baseType) = getImageConfig(colorTexture->getDesc().format);

    resize(
        colorTexture->getDesc().size.width,
        colorTexture->getDesc().size.height,
        false, colorTexture->getDesc().format, colorTexture);
}

/// Destructor
SlangFramebuffer::~SlangFramebuffer() { }

void SlangFramebuffer::resize(unsigned int width, unsigned int height, bool forceRecreate,
                              rhi::Format pixelFormat,
                              SlangTexturePtr extColorTexture)
{
    if (width <= 0 || height <= 0)
    {
        return;
    }
    if (width != _width || height != _height || forceRecreate)
    {
        _format = pixelFormat;
        if (_format == rhi::Format::Undefined && extColorTexture)
            _format = extColorTexture->getDesc().format;
        if (_format == rhi::Format::Undefined)
            _format = getRHIFormat(_baseType, _channelCount, _encodeSrgb);
        if (_format == rhi::Format::Undefined)
            throw ExceptionRenderError("Invalid combination channels and base type for framebuffer.");

        if (extColorTexture)
        {
            _colorTexture = extColorTexture;
        }
        else
        {
            rhi::TextureDesc textureDesc = {};
            textureDesc.usage = rhi::TextureUsage::RenderTarget | rhi::TextureUsage::CopySource;
            textureDesc.type = rhi::TextureType::Texture2D;
            textureDesc.arrayLength = 1;
            textureDesc.format = _format;
            textureDesc.size.width = width;
            textureDesc.size.height = height;
            textureDesc.mipCount = 1;
            textureDesc.defaultState = rhi::ResourceState::Present;
            _colorTexture = _device->createTexture(textureDesc);
        }

        rhi::TextureDesc depthDesc = {};
        depthDesc.usage = rhi::TextureUsage::DepthStencil;
        depthDesc.size.width = width;
        depthDesc.size.height = height;
        depthDesc.format = rhi::Format::D32Float;
        _depthTexture = _device->createTexture(depthDesc);

        _width = width;
        _height = height;
    }
}

void SlangFramebuffer::setColorTexture(SlangTexturePtr newColorTexture)
{
    auto sameDim = [](SlangTexturePtr tex0, SlangTexturePtr tex1) -> bool
    {
        return tex0->getDesc().size.width == tex1->getDesc().size.width &&
               tex0->getDesc().size.height == tex1->getDesc().size.height;
    };
    if ((sameDim(_colorTexture, newColorTexture)) && sameDim(newColorTexture, _depthTexture))
    {
        _colorTexture = newColorTexture;
    }
    else
    {
        throw ExceptionRenderError("Would be a mismatch between color and depth texture");
    }
}

/// Return the color data of this framebuffer as an image.
/// If an input image is provided, it will be used to store the color data;
/// otherwise a new image of the required format will be created.
ImagePtr SlangFramebuffer::getColorImage(ImagePtr image, bool flipVertically)
{
    if (!image || image->getWidth() != _width || image->getHeight() != _height)
    {
        image = Image::create(_width, _height, _channelCount, _baseType);
        image->createResourceBuffer();
    }

    rhi::ComPtr<ISlangBlob> resultBlob;
    rhi::SubresourceLayout textureLayout;
    if (SLANG_FAILED(_device->readTexture(_colorTexture.get(), 0, 0, resultBlob.writeRef(), &textureLayout)))
        throw ExceptionRenderError("Failed to read texture data from GPU");

    if (image->getRowStride() * image->getHeight() > textureLayout.sizeInBytes || image->getRowStride() > textureLayout.rowPitch)
        throw ExceptionRenderError("Mismatched sizes");

    if (flipVertically || image->getRowStride() != textureLayout.rowPitch)
    {
        auto dst = reinterpret_cast<uint8_t*>(image->getResourceBuffer());
        auto src = reinterpret_cast<const uint8_t*>(resultBlob->getBufferPointer());
        auto dstStride = image->getRowStride();
        auto srcStride = textureLayout.rowPitch;

        for (uint32_t srcy = 0; srcy < image->getHeight(); ++srcy)
        {
            uint32_t dsty = flipVertically ? (image->getHeight() - srcy - 1) : srcy;
            memcpy(dst + dsty * dstStride, src + srcy * srcStride, dstStride);
        }
    }
    else
    {
        memcpy(image->getResourceBuffer(), resultBlob->getBufferPointer(), image->getRowStride() * image->getHeight());
    }

    return image;
}

void SlangFramebuffer::bindRenderPassDesc(SlangRenderPassDesc& desc, int mipMapLevel)
{
    rhi::TextureViewDesc viewDesc = {};
    if (mipMapLevel > 0)
    {
        viewDesc.subresourceRange.mipCount = getColorTexture()->getDesc().mipCount;
        viewDesc.subresourceRange.mip = mipMapLevel;
    }

    desc.colorAttachments.resize(1);
    desc.colorAttachments[0].setView(_device->createTextureView(getColorTexture(), viewDesc));
    desc.colorAttachments[0].setLoadOp(SlangLoadOp::Clear);

    desc.depthStencilAttachment = SlangRenderPassDepthStencilAttachment();
    desc.depthStencilAttachment->setView(_device->createTextureView(getDepthTexture(), {}));
    desc.depthStencilAttachment->setDepthLoadOp(SlangLoadOp::Clear);
}

void SlangFramebuffer::bindRenderState(SlangRenderState& state)
{
    state.viewports[0] = rhi::Viewport::fromSize((float) getWidth(), (float) getHeight());
    state.viewportCount = 1;
    state.scissorRects[0] = rhi::ScissorRect::fromSize(getWidth(), getHeight());
    state.scissorRectCount = 1;
}

MATERIALX_NAMESPACE_END
