#include <MaterialXCore/Types.h>
#include <MaterialXRender/OpenGL/GLTextureHandler.h>
#include <MaterialXRender/OpenGL/GlslProgram.h>
#include <MaterialXRender/External/GLew/glew.h>

namespace MaterialX
{
bool GLTextureHandler::createColorImage(float color[4],
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

bool GLTextureHandler::acquireImage(std::string& fileName,
                                    ImageDesc &imageDesc,
                                    bool generateMipMaps)
{
    if (fileName.empty())
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
    if (ParentClass::acquireImage(fileName, imageDesc, generateMipMaps))
    {

        imageDesc.resourceId = MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glGenTextures(1, &imageDesc.resourceId);
        glActiveTexture(GL_TEXTURE0 + imageDesc.resourceId);
        glBindTexture(GL_TEXTURE_2D, imageDesc.resourceId);

        GLint internalFormat = GL_RGBA;
        GLint format = GL_RGBA;
        switch (imageDesc.channelCount)
        {
        case 3:
            internalFormat = GL_RGB;
            format = GL_RGB;
            break;
        case 2:
            internalFormat = GL_RG;
            format = GL_RG;
            break;
        case 1:
            internalFormat = GL_R;
            format = GL_R;
            break;
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

    if (!textureLoaded)
    {
        const string BLACK_TEXTURE("@internal_black_texture@");
        const ImageDesc* cachedColorDesc = getCachedImage(BLACK_TEXTURE);
        if (cachedColorDesc)
        {
            imageDesc = *cachedColorDesc;
        }
        else
        {
            float BLACK_COLOR[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            ImageDesc desc;
            desc.channelCount = 4;
            desc.width = 1;
            desc.height = 1;
            createColorImage(BLACK_COLOR, desc);

            cacheImage(BLACK_TEXTURE, desc);
        }
        fileName = BLACK_TEXTURE;
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
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, samplingProperties.defaultColor);
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