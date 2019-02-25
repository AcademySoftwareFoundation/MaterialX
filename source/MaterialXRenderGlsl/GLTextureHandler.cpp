#include <MaterialXCore/Types.h>
#include <MaterialXRenderGlsl/GLTextureHandler.h>
#include <MaterialXRenderGlsl/GlslProgram.h>
#include <MaterialXRenderGlsl/External/GLew/glew.h>

namespace MaterialX
{
bool GLTextureHandler::createColorImage(const std::array<float,4>& color,
                                        ImageDesc& imageDesc)
{
    if (!glActiveTexture)
    {
        glewInit();
    }

    ParentClass::createColorImage(color, imageDesc);
    if ((imageDesc.width * imageDesc.height > 0) && imageDesc.resourceBuffer)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glGenTextures(1, &imageDesc.resourceId);
        glActiveTexture(GL_TEXTURE0 + imageDesc.resourceId);
        glBindTexture(GL_TEXTURE_2D, imageDesc.resourceId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, imageDesc.width, imageDesc.height, 0, GL_RGBA, GL_FLOAT, imageDesc.resourceBuffer);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        return true;
    }
    return false;
}

bool GLTextureHandler::acquireImage(const FilePath& fileName,
                                    ImageDesc &imageDesc,
                                    bool generateMipMaps,
                                    const std::array<float,4>* fallbackColor)
{
    if (fileName.isEmpty())
    {
        return false;
    }

    if (!glActiveTexture)
    {
        glewInit();
    }

    // Check to see if we have already loaded in the texture.
    // If so, reuse the existing texture id
    const ImageDesc* cachedDesc = getCachedImage(fileName);
    if (cachedDesc)
    {
        imageDesc = *cachedDesc;
        return true;
    }

    bool textureLoaded = false;
    if (ParentClass::acquireImage(fileName, imageDesc, generateMipMaps, fallbackColor))
    {
        imageDesc.resourceId = MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glGenTextures(1, &imageDesc.resourceId);
        glActiveTexture(GL_TEXTURE0 + imageDesc.resourceId);
        glBindTexture(GL_TEXTURE_2D, imageDesc.resourceId);

        GLint internalFormat = imageDesc.floatingPoint ? GL_RGBA32F : GL_RGBA;
        GLint format = GL_RGBA;
        switch (imageDesc.channelCount)
        {
        case 3:
        {
            format = GL_RGB;
            // Map {RGB} to {RGB, 1} at shader access time
            GLint swizzleMaskRGB[] = { GL_RED, GL_GREEN, GL_BLUE, GL_ONE };
            glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMaskRGB);
            break;
        }
        case 2:
        {
            format = GL_RG;
            // Map {red, green} to {red, alpha} at shader access time
            GLint swizzleMaskRG[] = { GL_RED, GL_RED, GL_RED, GL_GREEN };
            glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMaskRG);
            break;
        }
        case 1:
        {
            format = GL_RED;
            // Map { red } to {red, green, blue, 1} at shader access time
            GLint swizzleMaskR[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
            glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMaskR);
            break;
        }
        default:
            break;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, imageDesc.width, imageDesc.height,
            0, format, imageDesc.floatingPoint ? GL_FLOAT : GL_UNSIGNED_BYTE, imageDesc.resourceBuffer);

        if (generateMipMaps)
        {
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        glBindTexture(GL_TEXTURE_2D, 0);

        free(imageDesc.resourceBuffer);
        imageDesc.resourceBuffer = nullptr;

        cacheImage(fileName, imageDesc);
        textureLoaded = true;
    }

    // Create a fallback texture if failed to load
    if (!textureLoaded && fallbackColor)
    {
        ImageDesc desc;
        desc.channelCount = 4;
        desc.width = 1;
        desc.height = 1;
        desc.floatingPoint = true;
        createColorImage(*fallbackColor, desc);
        cacheImage(fileName, desc);
        textureLoaded = true;
    }

    return textureLoaded;
}


bool GLTextureHandler::bindImage(const string &identifier, const ImageSamplingProperties& samplingProperties)
{
    const ImageDesc* cachedDesc = getCachedImage(identifier);
    if (cachedDesc)
    {
        if (!glActiveTexture)
        {
            glewInit();
        }

        unsigned int resourceId = cachedDesc->resourceId;

        // Bind a texture to the next available slot
        if (_maxImageUnits < 0)
        {
            glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &_maxImageUnits);
        }
        if (resourceId == MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID ||
            resourceId == static_cast<unsigned int>(_maxImageUnits))
        {
            return false;
        }
        glActiveTexture(GL_TEXTURE0 + resourceId);
        glBindTexture(GL_TEXTURE_2D, resourceId);

        // Set up texture properties
        //
        GLint minFilterType = mapFilterTypeToGL(samplingProperties.filterType);
        // Note: Magnification filters are more restrictive than minification
        GLint magFilterType = (minFilterType == GL_LINEAR || minFilterType == GL_REPEAT) ? minFilterType : GL_LINEAR;
        GLint uaddressMode = mapAddressModeToGL(samplingProperties.uaddressMode);
        GLint vaddressMode = mapAddressModeToGL(samplingProperties.vaddressMode);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, samplingProperties.defaultColor.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, uaddressMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, vaddressMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilterType);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilterType);

        return true;
    }
    return false;
}

int GLTextureHandler::mapAddressModeToGL(int addressModeEnum)
{
    int addressMode = GL_REPEAT;

    // Mapping is from "black". Use clamp to border
    // with border color black to achieve this
    if (addressModeEnum == 0)
    {
        addressMode = GL_CLAMP_TO_BORDER;
    }
    // Clamp
    else if (addressModeEnum == 1)
    {
        addressMode = GL_CLAMP;
    }
    return addressMode;
}

int GLTextureHandler::mapFilterTypeToGL(int filterTypeEnum)
{
    int filterType = GL_LINEAR_MIPMAP_LINEAR;
    // 0 = closest
    if (filterTypeEnum == 0)
    {
        filterType = GL_NEAREST;
    }
    // 1 == linear
    else if (filterTypeEnum == 1)
    {
        filterType = GL_LINEAR;
    }
    return filterType;
}

void GLTextureHandler::clearImageCache()
{
    ImageDescCache& cache = getImageCache();
    for (auto iter : cache)
    {
        deleteImage(iter.second);
    }
    ParentClass::clearImageCache();
}

void GLTextureHandler::deleteImage(MaterialX::ImageDesc& imageDesc)
{
    if (!glActiveTexture)
    {
        glewInit();
    }

    if (imageDesc.resourceId != MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID)
    {
        // Unbind a texture from image unit and delete the texture
        glActiveTexture(GL_TEXTURE0 + imageDesc.resourceId);
        glBindTexture(GL_TEXTURE_2D, MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
        glDeleteTextures(1, &imageDesc.resourceId);
        imageDesc.resourceId = MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;
    }
    // Delete any CPU side memory
    if (imageDesc.resourceBuffer)
    {
        free(imageDesc.resourceBuffer);
        imageDesc.resourceBuffer = nullptr;
    }
}

}
