//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Types.h>
#include <MaterialXRenderGlsl/GLTextureHandler.h>
#include <MaterialXRenderGlsl/GlslProgram.h>
#include <MaterialXRenderGlsl/External/GLew/glew.h>

namespace MaterialX
{

GLTextureHandler::GLTextureHandler(ImageLoaderPtr imageLoader) :
    ImageHandler(imageLoader),
    _maxImageUnits(-1)
{
    _restrictions.supportedBaseTypes = { ImageDesc::BASETYPE_HALF, ImageDesc::BASETYPE_FLOAT, ImageDesc::BASETYPE_UINT8 };
}

bool GLTextureHandler::acquireImage(const FilePath& filePath,
                                    ImageDesc& imageDesc,
                                    bool generateMipMaps,
                                    const Color4* fallbackColor)
{
    if (_boundTextureLocations.empty())
    {
        int maxTextureUnits;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
        if (maxTextureUnits <= 0)
        {
            ShaderValidationErrorList errors;
            errors.push_back("No texture units available");
            throw ExceptionShaderValidationError("OpenGL context error.", errors);
        }
        _boundTextureLocations.resize(maxTextureUnits, MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
    }

    if (!glActiveTexture)
    {
        glewInit();
    }

    // Check to see if we have already loaded in the texture.
    // If so, reuse the existing texture id
    const ImageDesc* cachedDesc = getCachedImage(filePath);
    if (cachedDesc)
    {
        imageDesc = *cachedDesc;
        return true;
    }

    bool textureLoaded = false;
    if (ImageHandler::acquireImage(filePath, imageDesc, generateMipMaps, fallbackColor))
    {
        imageDesc.resourceId = MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glGenTextures(1, &imageDesc.resourceId);

        int textureUnit = getNextAvailableTextureLocation();
        if (textureUnit < 0)
            return false;

        _boundTextureLocations[textureUnit] = imageDesc.resourceId;
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, imageDesc.resourceId);

        GLint internalFormat = GL_RGBA;
        GLenum type = GL_UNSIGNED_BYTE;

        if (imageDesc.baseType == ImageDesc::BASETYPE_FLOAT)
        {
            internalFormat = GL_RGBA32F;
            type = GL_FLOAT;
        }
        else if (imageDesc.baseType == ImageDesc::BASETYPE_HALF)
        {
            internalFormat = GL_RGBA16F;
            type = GL_HALF_FLOAT;
        }

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
            0, format, type, imageDesc.resourceBuffer);

        if (generateMipMaps)
        {
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        glBindTexture(GL_TEXTURE_2D, 0);

        imageDesc.freeResourceBuffer();
        cacheImage(filePath, imageDesc);
        textureLoaded = true;
    }

    // Create a fallback texture if failed to load
    if (!textureLoaded && fallbackColor)
    {
        imageDesc.channelCount = 4;
        imageDesc.width = 1;
        imageDesc.height = 1;
        imageDesc.baseType = ImageDesc::BASETYPE_FLOAT;
        createColorImage(*fallbackColor, imageDesc);
        if ((imageDesc.width * imageDesc.height > 0) && imageDesc.resourceBuffer)
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glGenTextures(1, &imageDesc.resourceId);

            int textureUnit = getNextAvailableTextureLocation();
            if (textureUnit < 0)
                return false;

            _boundTextureLocations[textureUnit] = imageDesc.resourceId;
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            glBindTexture(GL_TEXTURE_2D, imageDesc.resourceId);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, imageDesc.width, imageDesc.height, 0,
                GL_RGBA, GL_FLOAT, imageDesc.resourceBuffer);
            if (generateMipMaps)
            {
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        imageDesc.freeResourceBuffer();
        cacheImage(filePath, imageDesc);
    }

    return textureLoaded;
}


bool GLTextureHandler::bindImage(const FilePath& filePath, const ImageSamplingProperties& samplingProperties)
{
    const ImageDesc* cachedDesc = getCachedImage(filePath);
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

        int textureUnit = getBoundTextureLocation(resourceId);
        if (textureUnit < 0)
            return false;

        glActiveTexture(GL_TEXTURE0 + textureUnit);
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
        addressMode = GL_CLAMP_TO_EDGE;
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

void GLTextureHandler::deleteImage(MaterialX::ImageDesc& imageDesc)
{
    if (!glActiveTexture)
    {
        glewInit();
    }

    if (imageDesc.resourceId != MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID)
    {
        // Unbind a texture from image unit and delete the texture
        int textureUnit = getBoundTextureLocation(imageDesc.resourceId);
        if(textureUnit < 0) return;
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
        glDeleteTextures(1, &imageDesc.resourceId);
        _boundTextureLocations[textureUnit] = MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;
        imageDesc.resourceId = MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;
    }

    // Delete any CPU side memory
    ImageHandler::deleteImage(imageDesc);
}

int GLTextureHandler::getBoundTextureLocation(unsigned int resourceId)
{
    for(size_t i=0; i<_boundTextureLocations.size(); i++)
    {
        if(_boundTextureLocations[i] == resourceId)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int GLTextureHandler::getNextAvailableTextureLocation()
{
    for(size_t i=0; i<_boundTextureLocations.size(); i++)
    {
        if(_boundTextureLocations[i] == MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

}
