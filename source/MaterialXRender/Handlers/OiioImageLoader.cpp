#include <MaterialXRender/Handlers/OiioImageLoader.h>

#ifdef MATERIALX_BUILD_OIIO

#include <OpenImageIO/imageio.h>

namespace MaterialX
{

bool OiioImageLoader::saveImage(const std::string& fileName,
                                const ImageDesc &imageDesc)
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

#endif // MATERIALX_BUILD_OIIO
