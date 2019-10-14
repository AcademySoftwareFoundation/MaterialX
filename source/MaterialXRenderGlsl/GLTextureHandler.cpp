//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRenderGlsl/GLTextureHandler.h>

#include <MaterialXRenderGlsl/GlslProgram.h>
#include <MaterialXRenderGlsl/External/GLew/glew.h>

#include <MaterialXRender/ShaderRenderer.h>

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
            StringVec errors;
            errors.push_back("No texture units available");
            throw ExceptionShaderRenderError("OpenGL context error.", errors);
        }
        _boundTextureLocations.resize(maxTextureUnits, MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
    }

    if (!glActiveTexture)
    {
        glewInit();
    }

    // Return a cached image if available.
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

        // Update bound location if not already bound
        int textureUnit = getBoundTextureLocation(resourceId);
        if (textureUnit < 0)
        {
            textureUnit = getNextAvailableTextureLocation();
        }
        if (textureUnit < 0)
        {
            return false;
        }      
        _boundTextureLocations[textureUnit] = resourceId;

        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, resourceId);

        // Set up texture properties
        //
        GLint minFilterType = mapFilterTypeToGL(samplingProperties.filterType);
        GLint magFilterType = GL_LINEAR; // Magnification filters are more restrictive than minification
        GLint uaddressMode = mapAddressModeToGL(samplingProperties.uaddressMode);
        GLint vaddressMode = mapAddressModeToGL(samplingProperties.vaddressMode);
        Color4 borderColor(samplingProperties.defaultColor);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, uaddressMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, vaddressMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilterType);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilterType);

        return true;
    }
    return false;
}

bool GLTextureHandler::unbindImage(const FilePath& filePath)
{
    const ImageDesc* cachedDesc = getCachedImage(filePath);
    if (cachedDesc)
    {
        return unbindImage(*cachedDesc);
    }
    return false;
}

bool GLTextureHandler::unbindImage(const ImageDesc& imageDesc)
{
    if (!glActiveTexture)
    {
        glewInit();
    }

    if (imageDesc.resourceId != MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID)
    {
        int textureUnit = getBoundTextureLocation(imageDesc.resourceId);
        if (textureUnit >= 0)
        {
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            glBindTexture(GL_TEXTURE_2D, MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
            _boundTextureLocations[textureUnit] = MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;
            return true;
        }
    }
    return false;
}


void GLTextureHandler::deleteImage(ImageDesc& imageDesc)
{
    if (!glActiveTexture)
    {
        glewInit();
    }

    // Unbind a texture from image unit if bound and delete the texture
    if (imageDesc.resourceId != MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID)
    {
        unbindImage(imageDesc);
        glDeleteTextures(1, &imageDesc.resourceId);
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


int GLTextureHandler::mapAddressModeToGL(ImageSamplingProperties::AddressMode addressModeEnum)
{
    const vector<int> addressModes
    {
        // Constant color. Use clamp to border
        // with border color to achieve this
        GL_CLAMP_TO_BORDER,

        // Clamp
        GL_CLAMP_TO_EDGE,

        // Repeat
        GL_REPEAT,

        // Mirror
        GL_MIRRORED_REPEAT
    };

    int addressMode = GL_REPEAT;
    if (addressModeEnum != ImageSamplingProperties::AddressMode::UNSPECIFIED)
    {
        addressMode = addressModes[static_cast<int>(addressModeEnum)];
    }
    return addressMode;
}

int GLTextureHandler::mapFilterTypeToGL(ImageSamplingProperties::FilterType filterTypeEnum)
{
    int filterType = GL_LINEAR_MIPMAP_LINEAR;
    if (filterTypeEnum == ImageSamplingProperties::FilterType::CLOSEST)
    {
        filterType = GL_NEAREST_MIPMAP_NEAREST;
    }
    return filterType;
}

}
