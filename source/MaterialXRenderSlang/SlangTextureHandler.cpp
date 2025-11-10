//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderSlang/SlangTextureHandler.h>
#include <MaterialXRenderSlang/SlangProgram.h>
#include <MaterialXRenderSlang/SlangTypeUtils.h>

#include <MaterialXRender/ShaderRenderer.h>

#include <iostream>
#include <cassert>

MATERIALX_NAMESPACE_BEGIN

SlangTextureHandler::SlangTextureHandler(SlangContextPtr context, ImageLoaderPtr imageLoader) :
    ImageHandler(std::move(imageLoader)), _context(std::move(context)), _device(_context->getDevice())
{
    getPipeline(1, Image::BaseType::FLOAT);
    getPipeline(1, Image::BaseType::UINT8);
    getPipeline(3, Image::BaseType::FLOAT);
    getPipeline(3, Image::BaseType::UINT8);
}

/// Release rendering resources for the given image, or for all cached images
/// if no image pointer is specified.
void SlangTextureHandler::releaseRenderResources(ImagePtr image)
{
    if (!image)
    {
        for (const auto& iter : _imageCache)
        {
            if (iter.second)
            {
                releaseRenderResources(iter.second);
            }
        }
        return;
    }

    if (image->getResourceId() == SlangProgram::UNDEFINED_SLANG_RESOURCE_ID)
    {
        return;
    }

    if (auto it = _slangTextureMap.find(image->getResourceId()); it != _slangTextureMap.end())
    {
        _slangTextureMap.erase(it);
    }
    if (auto it = _imageBindingInfo.find(image->getResourceId()); it != _imageBindingInfo.end())
    {
        _imageBindingInfo.erase(it);
    }
    image->setResourceId(SlangProgram::UNDEFINED_SLANG_RESOURCE_ID);
}

SlangTexturePtr SlangTextureHandler::getAssociatedSlangTexture(ImagePtr image)
{
    if (!image)
        return {};

    if (auto it = _slangTextureMap.find(image->getResourceId()); it != _slangTextureMap.end())
        return it->second;

    return {};
}

bool SlangTextureHandler::bindImage(ImagePtr image, const ImageSamplingProperties& samplingProperties)
{
    if (image->getResourceId() == SlangProgram::UNDEFINED_SLANG_RESOURCE_ID)
    {
        if (!createRenderResources(image, true))
        {
            return false;
        }
    }

    _imageBindingInfo[image->getResourceId()] = std::make_pair(image, samplingProperties);
    return true;
}

rhi::ComPtr<rhi::ISampler> SlangTextureHandler::getSamplerState(const ImageSamplingProperties& samplingProperties)
{
    auto samplerIt = _imageSamplerStateMap.find(samplingProperties);
    if (samplerIt != _imageSamplerStateMap.end())
        return samplerIt->second;

    rhi::SamplerDesc samplerDesc = {};
    samplerDesc.addressU = mapAddressModeToSlang(samplingProperties.uaddressMode);
    samplerDesc.addressV = mapAddressModeToSlang(samplingProperties.vaddressMode);
    samplerDesc.minFilter = mapFilterTypeToSlang(samplingProperties.filterType);
    samplerDesc.magFilter = mapFilterTypeToSlang(samplingProperties.filterType);
    samplerDesc.mipFilter = mapFilterTypeToSlang(samplingProperties.filterType);
    if (!samplingProperties.enableMipmaps)
        samplerDesc.maxLOD = 0.f;

    // Border color not supported in Vulkan
    samplerDesc.borderColor[0] = samplingProperties.defaultColor[0];
    samplerDesc.borderColor[1] = samplingProperties.defaultColor[1];
    samplerDesc.borderColor[2] = samplingProperties.defaultColor[2];
    samplerDesc.borderColor[3] = samplingProperties.defaultColor[3];
    if (samplingProperties.filterType != ImageSamplingProperties::FilterType::CLOSEST)
        samplerDesc.maxAnisotropy = 16;

    auto sampler = _device->createSampler(samplerDesc);
    _imageSamplerStateMap[samplingProperties] = sampler;

    return sampler;
}

bool SlangTextureHandler::bindImage(rhi::ShaderCursor cursor, ImagePtr image)
{
    if (image->getResourceId() == SlangProgram::UNDEFINED_SLANG_RESOURCE_ID)
    {
        if (!createRenderResources(image, true))
        {
            return false;
        }
    }

    cursor["tex"].setBinding(_slangTextureMap[image->getResourceId()]);
    cursor["sampler"].setBinding(getSamplerState(_imageBindingInfo[image->getResourceId()].second));

    return true;
}

bool SlangTextureHandler::createRenderResources(ImagePtr image, bool generateMipMaps, bool useAsRenderTarget)
{
    if (image->getResourceId() == SlangProgram::UNDEFINED_SLANG_RESOURCE_ID)
    {
        static unsigned int resourceId = 0;
        image->setResourceId(++resourceId);
    }

    auto commandEncoder = _context->createCommandEncoder();
    auto texture = makeTexture(commandEncoder, image.get(), generateMipMaps, useAsRenderTarget);
    _context->submitCommandEncoder(commandEncoder);
    _context->waitOnHost();
    _slangTextureMap[image->getResourceId()] = std::move(texture);

    return true;
}

/// Utility to map an address mode enumeration to an Slang RHI address mode
rhi::TextureAddressingMode SlangTextureHandler::mapAddressModeToSlang(ImageSamplingProperties::AddressMode addressModeEnum)
{
    switch (addressModeEnum)
    {
        case ImageSamplingProperties::AddressMode::UNSPECIFIED:
            return rhi::TextureAddressingMode::Wrap;
        case ImageSamplingProperties::AddressMode::CONSTANT:
            return rhi::TextureAddressingMode::ClampToBorder;
        case ImageSamplingProperties::AddressMode::CLAMP:
            return rhi::TextureAddressingMode::ClampToEdge;
        case ImageSamplingProperties::AddressMode::PERIODIC:
            return rhi::TextureAddressingMode::Wrap;
        case ImageSamplingProperties::AddressMode::MIRROR:
            return rhi::TextureAddressingMode::MirrorRepeat;
    }
    throw ExceptionRenderError("Unknown ImageSamplingProperties::AddressMode: " + std::to_string((int) addressModeEnum));
}

/// Utility to map a filter type enumeration to an Slang RHI filter type
rhi::TextureFilteringMode SlangTextureHandler::mapFilterTypeToSlang(ImageSamplingProperties::FilterType filterTypeEnum)
{
    if (filterTypeEnum == ImageSamplingProperties::FilterType::CLOSEST)
        return rhi::TextureFilteringMode::Point;

    return rhi::TextureFilteringMode::Linear;
}

/// Utility to map generic texture properties to Slang RHI texture formats.
rhi::Format SlangTextureHandler::mapTextureFormatToSlang(Image::BaseType baseType, unsigned int channelCount, bool srgb)
{
    return getRHIFormat(baseType, channelCount, srgb);
}

/// Makes sure the image has CPU data. If force is set to true, it will overwrite the data from GPU even if it exists.
ImagePtr SlangTextureHandler::uploadImageToCPU(ImagePtr image, bool force)
{
    /// already have CPU data
    if (image->getResourceBuffer() && !force)
        return image;

    SlangTexturePtr imageTexture = getAssociatedSlangTexture(image);

    /// There is no associated GPU texture to upload
    if (!imageTexture)
        return {};

    rhi::Format format = imageTexture->getDesc().format;
    uint32_t width = imageTexture->getDesc().size.width;
    uint32_t height = imageTexture->getDesc().size.height;
    if (format != getRHIFormat(image->getBaseType(), image->getChannelCount()) ||
        width != image->getWidth() || height != image->getHeight())
    {
        auto [channelCount, baseType] = getImageConfig(format);
        if (channelCount == 0)
            throw ExceptionRenderError("Incompatible image format");
        image = Image::create(width, height, channelCount, baseType);
    }
    image->createResourceBuffer();

    rhi::ComPtr<ISlangBlob> resultBlob;
    rhi::SubresourceLayout textureLayout;
    if (SLANG_FAILED(_device->readTexture(imageTexture.get(), 0, 0, resultBlob.writeRef(), &textureLayout)))
        throw ExceptionRenderError("Failed to read texture data from GPU");

    if (image->getRowStride() * image->getHeight() > textureLayout.sizeInBytes || image->getRowStride() > textureLayout.rowPitch)
        throw ExceptionRenderError("Mismatched sizes");

    if (image->getRowStride() != textureLayout.rowPitch)
    {
        auto dst = reinterpret_cast<uint8_t*>(image->getResourceBuffer());
        auto src = reinterpret_cast<const uint8_t*>(resultBlob->getBufferPointer());
        auto dstStride = image->getRowStride();
        auto srcStride = textureLayout.rowPitch;

        for (uint32_t y = 0; y < image->getHeight(); ++y)
            memcpy(dst + y * dstStride, src + y * srcStride, dstStride);
    }
    else
    {
        memcpy(image->getResourceBuffer(), resultBlob->getBufferPointer(), image->getRowStride() * image->getHeight());
    }

    return image;
}

namespace
{
/// Returns true if the data can be directly uploaded to a texture
bool isCompatible(rhi::IDevice* device, Image::BaseType baseType, uint32_t channelCount)
{
    if (channelCount == 1)
        return false;

    rhi::Format format = getRHIFormat(baseType, channelCount, false);
    if (format == rhi::Format::Undefined)
        return false;

    rhi::FormatSupport support;
    device->getFormatSupport(format, &support);
    return is_set(support, rhi::FormatSupport::Texture) && is_set(support, rhi::FormatSupport::RenderTarget);
}
} // namespace

rhi::ComPtr<rhi::ITexture> SlangTextureHandler::makeTexture(SlangCommandEncoderPtr& commandEncoder, Image* image, bool generateMipMaps, bool useAsRenderTarget)
{
    uint32_t channelCount = image->getChannelCount();

    if (channelCount == 1)
        channelCount = 3;

    if (!isCompatible(_device, image->getBaseType(), channelCount))
    {
        channelCount = 4;
    }

    if (!isCompatible(_device, image->getBaseType(), channelCount))
    {
        throw ExceptionRenderError("Current image configuration cannot be made compatible with texture format");
    }

    rhi::TextureDesc textureDesc = {};
    textureDesc.size.width = image->getWidth();
    textureDesc.size.height = image->getHeight();
    textureDesc.usage = rhi::TextureUsage::ShaderResource | rhi::TextureUsage::UnorderedAccess |
                        ((useAsRenderTarget || generateMipMaps) ? rhi::TextureUsage::RenderTarget : rhi::TextureUsage::None);
    textureDesc.defaultState = rhi::ResourceState::ShaderResource;
    textureDesc.format = mapTextureFormatToSlang(image->getBaseType(), channelCount, false);
    textureDesc.mipCount = generateMipMaps ? image->getMaxMipCount() : 1;

    SlangTexturePtr texture = _device->createTexture(textureDesc);

    /// If we can just upload the data, do that
    if (channelCount == image->getChannelCount())
    {
        rhi::SubresourceRange subresourceRange;
        subresourceRange.mip = 0;
        subresourceRange.mipCount = 1;
        subresourceRange.layer = 0;
        subresourceRange.layerCount = 1;

        rhi::SubresourceData subresourceData;
        subresourceData.data = image->getResourceBuffer();
        subresourceData.rowPitch = image->getRowStride();
        subresourceData.slicePitch = 0;

        commandEncoder->uploadTextureData(
            texture,
            subresourceRange,
            rhi::Offset3D{ 0, 0, 0 },
            rhi::Extent3D::kWholeTexture,
            &subresourceData,
            1);
    }
    else
    {
        /// Otherwise, we use compute shader to set the texture
        rhi::BufferDesc bufferDesc = {};
        bufferDesc.size = image->getWidth() * image->getHeight() * image->getChannelCount() * image->getBaseStride();
        bufferDesc.usage = rhi::BufferUsage::ShaderResource | rhi::BufferUsage::UnorderedAccess | rhi::BufferUsage::CopyDestination;

        auto srcBuffer = _device->createBuffer(bufferDesc, image->getResourceBuffer());
        auto passEncoder = commandEncoder->beginComputePass();
        auto rootObject = passEncoder->bindPipeline(getPipeline(image));
        rhi::ShaderCursor cursor(rootObject->getEntryPoint(0));

        cursor["dst"].setBinding(texture);
        cursor["src"].setBinding(srcBuffer);
        passEncoder->dispatchCompute(image->getWidth(), image->getHeight(), 1);
        passEncoder->end();
    }

    if (generateMipMaps)
        _context->_blitter->generateMips(commandEncoder, texture);

    return texture;
}

rhi::ComPtr<rhi::IComputePipeline> SlangTextureHandler::getPipeline(Image* image)
{
    return getPipeline(image->getChannelCount(), image->getBaseType());
}

rhi::ComPtr<rhi::IComputePipeline> SlangTextureHandler::getPipeline(unsigned channelCount, Image::BaseType baseType)
{
    if (auto it = _pipelineCache.find({ channelCount, baseType }); it != _pipelineCache.end())
        return it->second;

    using namespace rhi;

    ComPtr<slang::ISession> slangSession = _context->getDevice()->getSlangSession();
    ComPtr<slang::IBlob> diagnosticsBlob;
    std::vector<slang::IComponentType*> componentTypes;
    ComPtr<slang::IEntryPoint> entryPoint;
    ComPtr<slang::IComponentType> linkedProgram;
    StringVec diagnosticVec;

    auto validateResult = [&](bool success)
    {
        if (diagnosticsBlob)
            diagnosticVec.push_back((const char*) diagnosticsBlob->getBufferPointer());

        if (!success)
        {
            for (const auto& it : diagnosticVec)
                std::cerr << it << std::endl;
            throw ExceptionRenderError("Failed to compile blit shaders", diagnosticVec);
        }
    };

    std::string source;
    source += "#define SRC_SIZE " + std::to_string(channelCount) + "\n";
    switch (baseType)
    {
        case Image::BaseType::UINT8:
            source += "#define SRC_TYPE 0\n";
            break;
        case Image::BaseType::FLOAT:
            source += "#define SRC_TYPE 1\n";
            break;
        default:
            throw ExceptionRenderError("Cannot convert other formats yet.");
    }
    source += "#define SRC_SIZE " + std::to_string(channelCount);
    source += CONVERSION_SHADER;

    std::string name = _context->deduplicateName("conversion");
    slang::IModule* module = slangSession->loadModuleFromSourceString(
        name.c_str(), name.c_str(), source.c_str(), diagnosticsBlob.writeRef());

    validateResult(module);

    Result result;

    componentTypes.push_back(module);
    result = module->findEntryPointByName("copyData", entryPoint.writeRef());
    validateResult(SLANG_SUCCEEDED(result));
    componentTypes.push_back(entryPoint);

    ComPtr<slang::IComponentType> composedProgram;
    result = slangSession->createCompositeComponentType(
        componentTypes.data(),
        componentTypes.size(),
        composedProgram.writeRef(),
        diagnosticsBlob.writeRef());
    validateResult(SLANG_SUCCEEDED(result));

    result = composedProgram->link(linkedProgram.writeRef(), diagnosticsBlob.writeRef());
    validateResult(SLANG_SUCCEEDED(result));

    auto program = _context->getDevice()->createShaderProgram(linkedProgram, diagnosticsBlob.writeRef());
    validateResult(program);

    rhi::ComputePipelineDesc pipelineDesc = {};
    pipelineDesc.program = program;
    auto pipeline = _context->getDevice()->createComputePipeline(pipelineDesc);

    _pipelineCache[{ channelCount, baseType }] = pipeline;
    return pipeline;
}

const std::string SlangTextureHandler::CONVERSION_SHADER = R"===(

#define SRC_TYPE_UINT8 0
#define SRC_TYPE_FLOAT 1

#ifndef SRC_SIZE
    #define SRC_SIZE 1
#endif

#ifndef SRC_TYPE
    #define SRC_TYPE SRC_TYPE_FLOAT
#endif

#if SRC_TYPE == SRC_TYPE_UINT8
    #define DST_TYPE float4
#else
    #define DST_TYPE float4
#endif

uint4 loadUint8_tX(ByteAddressBuffer buffer, uint pixelIndex)
{
    const uint size = SRC_SIZE;
    uint byteIndex = pixelIndex * size;
    uint alignedByteIndex = (byteIndex / 4) * 4;
    /// This many bytes need to shift to align the values where we want them
    uint32_t shift = byteIndex - alignedByteIndex;

    /// Load into alignedValues, dealing with alignment issues (shift out previous data)
    uint32_t alignedValues = buffer.Load(alignedByteIndex) >> (shift * 8);
    if ((byteIndex - alignedByteIndex + size) > 4)
    {
        uint32_t temp = buffer.Load(alignedByteIndex + 4) << ((4 - shift) * 8);
        alignedValues |= temp;
    }

    uint4 result = { 0, 0, 0, 255 };
    for (uint i = 0; i < size; ++i)
    {
        result[i] = alignedValues & 255;
        alignedValues >>= 8;
    }

    return result;
};

float4 loadFloat4(ByteAddressBuffer buffer, uint pixelIndex)
{
    float4 result = { 0, 0, 0, 1 };

    const uint size = SRC_SIZE;
    uint byteIndex = pixelIndex * size * 4;

    for (uint i = 0; i < size; ++i)
    {
        result[i] = buffer.Load<float>(byteIndex + 4 * i);
    }

    return result;
};

[shader("compute")]
[numthreads(16, 16, 1)]
void copyData(uint3 sv_dispatchThreadID: SV_DispatchThreadID,
              uniform RWTexture2D<DST_TYPE> dst,
              uniform ByteAddressBuffer src)
{
    uint width;
    uint height;
    dst.GetDimensions(width, height);

    if (sv_dispatchThreadID.x >= width || sv_dispatchThreadID.y >= height)
        return;

    uint pixelIndex = sv_dispatchThreadID.x + sv_dispatchThreadID.y * width;
#if SRC_TYPE == SRC_TYPE_FLOAT
    DST_TYPE dstValue = loadFloat4(src, pixelIndex);
#elif SRC_TYPE == SRC_TYPE_UINT8
    uint4 dstValue = loadUint8_tX(src, pixelIndex);
#endif

#if SRC_SIZE == 1
    dstValue[1] = dstValue[0];
    dstValue[2] = dstValue[0];
#endif

#if SRC_TYPE == SRC_TYPE_FLOAT
    dst[sv_dispatchThreadID.xy] = dstValue;
#elif SRC_TYPE == SRC_TYPE_UINT8
    dst[sv_dispatchThreadID.xy] = dstValue / 255.0;
#endif
}

)===";

MATERIALX_NAMESPACE_END
