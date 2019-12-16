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
}

ImagePtr GLTextureHandler::acquireImage(const FilePath& filePath,
                                        bool generateMipMaps,
                                        const Color4* fallbackColor,
                                        string* message)
{
    // Resolve the input filepath.
    FilePath resolvedFilePath = filePath;
    if (_resolver)
    {
        resolvedFilePath = _resolver->resolve(resolvedFilePath, FILENAME_TYPE_STRING);
    }

    // Return a cached image if available.
    ImagePtr cachedDesc = getCachedImage(resolvedFilePath);
    if (cachedDesc)
    {
        return cachedDesc;
    }

    // Call the base acquire method.
    return ImageHandler::acquireImage(resolvedFilePath, generateMipMaps, fallbackColor, message);
}

bool GLTextureHandler::bindImage(ImagePtr image, const ImageSamplingProperties& samplingProperties)
{
    // Create renderer resources if needed.
    if (image->getResourceId() == GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID)
    {
        if (!createRenderResources(image, true))
        {
            return false;
        }
    }

    // Bind a texture to the next available slot
    if (_maxImageUnits < 0)
    {
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &_maxImageUnits);
    }
    if (image->getResourceId() == GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID ||
        image->getResourceId() == static_cast<unsigned int>(_maxImageUnits))
    {
        return false;
    }

    // Update bound location if not already bound
    int textureUnit = getBoundTextureLocation(image->getResourceId());
    if (textureUnit < 0)
    {
        textureUnit = getNextAvailableTextureLocation();
    }
    if (textureUnit < 0)
    {
        return false;
    }      
    _boundTextureLocations[textureUnit] = image->getResourceId();

    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, image->getResourceId());

    // Set up texture properties
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

bool GLTextureHandler::unbindImage(ImagePtr image)
{
    if (!glActiveTexture)
    {
        glewInit();
    }

    if (image->getResourceId() != GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID)
    {
        int textureUnit = getBoundTextureLocation(image->getResourceId());
        if (textureUnit >= 0)
        {
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            glBindTexture(GL_TEXTURE_2D, GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
            _boundTextureLocations[textureUnit] = GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;
            return true;
        }
    }
    return false;
}

bool GLTextureHandler::createRenderResources(ImagePtr image, bool generateMipMaps)
{
    // Initialize OpenGL setup if needed.
    if (!glActiveTexture)
    {
        glewInit();
    }
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
        _boundTextureLocations.resize(maxTextureUnits, GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
    }

    unsigned int resourceId = GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &resourceId);
    image->setResourceId(resourceId);

    int textureUnit = getNextAvailableTextureLocation();
    if (textureUnit < 0)
    {
        return false;
    }

    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, image->getResourceId());

    GLint internalFormat = GL_RGBA;
    GLenum type = GL_UNSIGNED_BYTE;

    if (image->getBaseType() == Image::BaseType::FLOAT)
    {
        internalFormat = GL_RGBA32F;
        type = GL_FLOAT;
    }
    else if (image->getBaseType() == Image::BaseType::HALF)
    {
        internalFormat = GL_RGBA16F;
        type = GL_HALF_FLOAT;
    }

    GLint format = GL_RGBA;
    switch (image->getChannelCount())
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

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image->getWidth(), image->getHeight(),
        0, format, type, image->getResourceBuffer());

    if (generateMipMaps)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}
    
void GLTextureHandler::releaseRenderResources(ImagePtr image)
{
    if (!image || image->getResourceId() == GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID)
    {
        return;
    }

    unbindImage(image);
    unsigned int resourceId = image->getResourceId();
    glDeleteTextures(1, &resourceId);
    image->setResourceId(GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
}

int GLTextureHandler::getBoundTextureLocation(unsigned int resourceId)
{
    for (size_t i = 0; i < _boundTextureLocations.size(); i++)
    {
        if (_boundTextureLocations[i] == resourceId)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int GLTextureHandler::getNextAvailableTextureLocation()
{
    for (size_t i = 0; i < _boundTextureLocations.size(); i++)
    {
        if (_boundTextureLocations[i] == GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID)
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
