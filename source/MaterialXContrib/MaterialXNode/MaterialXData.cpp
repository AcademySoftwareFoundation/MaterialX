#include "MaterialXData.h"
#include "MaterialXUtil.h"

#include <MaterialXFormat/XmlIo.h>
#include <MaterialXGenShader/Util.h>
#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenOgsXml/GlslFragmentGenerator.h>

MaterialXData::MaterialXData(   mx::DocumentPtr document,
                                const std::string& elementPath,
                                const MaterialX::FileSearchPath& librarySearchPath )
    : _document(document)
{
    if (!_document)
    {
        throw mx::Exception("No document specified");
    }

    std::vector<mx::TypedElementPtr> renderableElements;
    mx::findRenderableElements(_document, renderableElements);

    // Nothing specified. Find the first renderable element and use that
    if (elementPath.empty())
    {
        if (renderableElements.empty())
        {
            throw mx::Exception(
                "No element path specified and no renderable elements in the document."
            );
        }

        _element = renderableElements.front();
    }
    else
    {
        _element = _document->getDescendant(elementPath);
        if (!_element)
        {
            std::string message = "Element '";
            message += elementPath;
            message += "' not found in the document.";
            throw mx::Exception(message);
        }

        auto it = std::find_if(
            renderableElements.begin(),
            renderableElements.end(),
            [this](mx::TypedElementPtr renderableElement) -> bool
            {
                return _element->getNamePath() == renderableElement->getNamePath();
            }
        );

        if (it == renderableElements.end())
        {
            throw mx::Exception("The specified element is not renderable");
        }
    }

    try
    {
        generateFragment(librarySearchPath);
    }
    catch (std::exception& e)
    {
        throw mx::Exception(std::string("Failed to generate OGS shader fragment: ") + e.what());
    }
}

MaterialXData::~MaterialXData()
{
}

const std::string& MaterialXData::getFragmentName() const
{
    return _fragmentName;
}

const std::string& MaterialXData::getFragmentSource() const
{
    return _fragmentSource;
}

const mx::StringMap& MaterialXData::getPathInputMap() const
{
    return _pathInputMap;
}

bool MaterialXData::elementIsAShader() const
{
    return _element && _element->isA<mx::ShaderRef>();
}

void MaterialXData::generateFragment(const mx::FileSearchPath& librarySearchPath)
{
    if (!_element)
    {
        throw mx::Exception("Element is NULL");
    }

    mx::OutputPtr output = _element->asA<mx::Output>();
    mx::ShaderRefPtr shaderRef = _element->asA<mx::ShaderRef>();
    if (!output && !shaderRef)
    {
        // Should never occur as we pre-filter renderables before creating the node + override
        throw mx::Exception("Invalid element to create wrapper for " + _element->getName());
    }

    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::GlslFragmentGenerator::create();
        mx::GenContext genContext(shaderGenerator);
        mx::GenOptions& genOptions = genContext.getOptions();

        // Set up color management. We assume the target render space is linear
        // if not found in the document. Currently the default system has no other color space targets.
        //
        static std::string MATERIALX_LINEAR_WORKING_SPACE("lin_rec709");
        const std::string language = shaderGenerator->getLanguage();
        mx::DefaultColorManagementSystemPtr colorManagementSystem = mx::DefaultColorManagementSystem::create(language);
        if (colorManagementSystem)
        {
            shaderGenerator->setColorManagementSystem(colorManagementSystem);
            colorManagementSystem->loadLibrary(_document);
            const std::string& documentColorSpace = _document->getAttribute(mx::Element::COLOR_SPACE_ATTRIBUTE);
            if (documentColorSpace.empty())
            {
                genOptions.targetColorSpaceOverride = MATERIALX_LINEAR_WORKING_SPACE;
            }
            else
            {
                genOptions.targetColorSpaceOverride = documentColorSpace;
            }
        }

        // Use FIS environment lookup for surface shader generation but
        // disable for textures to avoid additional unneeded XML parameter
        // generation.
        genContext.registerSourceCodeSearchPath(librarySearchPath);
        genOptions.hwSpecularEnvironmentMethod =
            shaderRef ? mx::SPECULAR_ENVIRONMENT_FIS : mx::SPECULAR_ENVIRONMENT_NONE;

        // Set to use no direct lighting
        genOptions.hwMaxActiveLightSources = 0;
        
        // Maya images require a texture coordaintes to be flipped in V.
        genOptions.fileTextureVerticalFlip = true;

        // Maya viewport uses texture atlas for tile image so enabled
        // texture coordinate transform to go from original UDIM range to
        // normalized 0..1 range.
        genOptions.hwNormalizeUdimTexCoords = true;

        // Generate the fragment source (shader and XML wrapper).
        _fragmentName = _element->getNamePath();
        _fragmentName = mx::createValidName(_fragmentName);

        _shader = shaderGenerator->generate(_fragmentName, _element, genContext);
        if (!_shader)
        {
            throw mx::Exception("Failed to generate shader");
        }
    }

    static const std::string FRAGMENT_NAME_TOKEN = "$fragmentName";

    {
        std::ostringstream sourceStream;
        mx::OgsXmlGenerator ogsXmlGenerator;

        // Note: This name must match the the fragment name used for registration
        // or the registration will fail.
        ogsXmlGenerator.generate(FRAGMENT_NAME_TOKEN, _shader.get(), nullptr, sourceStream);
        _fragmentSource = sourceStream.str();
        if (_fragmentSource.empty())
        {
            throw mx::Exception("Generated shader source is empty");
        }
    }

    // Strip out any '\r' characters.
    _fragmentSource.erase(
        std::remove(_fragmentSource.begin(), _fragmentSource.end(), '\r'), _fragmentSource.end()
    );

    {
        std::ostringstream nameStream;

        const size_t sourceHash = std::hash<std::string>{}(_fragmentSource);
        nameStream << _fragmentName << "__" << std::hex << sourceHash;

        _fragmentName = nameStream.str();
    }

    const mx::StringMap substitutions{ {FRAGMENT_NAME_TOKEN, _fragmentName} };
    mx::tokenSubstitution(substitutions, _fragmentSource);

    // Extract out the input fragment parameter names along with their
    // associated Element paths to allow for value binding.
    const mx::ShaderStage& pixelShader = _shader->getStage(mx::Stage::PIXEL);
    for (const auto& blockMap : pixelShader.getUniformBlocks())
    {
        const mx::VariableBlock& uniforms = *blockMap.second;

        // Skip light uniforms
        if (uniforms.getName() == mx::HW::LIGHT_DATA)
        {
            continue;
        }

        for (size_t i = 0; i < uniforms.size(); i++)
        {
            const mx::ShaderPort* port = uniforms[i];
            const std::string& path = port->getPath();
            if (!path.empty())
            {
                std::string name = port->getVariable();
                if (port->getType()->getSemantic() == mx::TypeDesc::SEMANTIC_FILENAME)
                {
                    // Strip out the "sampler" post-fix to get the texture name.
                    size_t pos = name.find(mx::OgsXmlGenerator::SAMPLER_SUFFIX);
                    name.erase(pos, mx::OgsXmlGenerator::SAMPLER_SUFFIX.length());
                }
                _pathInputMap[path] = name;
            }
        }
    }
}

mx::ImageSamplingProperties MaterialXData::getImageSamplngProperties(const std::string& fileParameterName) const
{
    mx::ImageSamplingProperties samplingProperties;
    if (_shader && _shader->hasStage(mx::Stage::PIXEL))
    {
        mx::ShaderStage& stage = _shader->getStage(mx::Stage::PIXEL);
        mx::VariableBlock& block = stage.getUniformBlock(mx::HW::PUBLIC_UNIFORMS);
        samplingProperties.setProperties(fileParameterName, block);

    }
    return samplingProperties;
}

std::string MaterialXData::getMatrix4Name(const std::string& matrix3Name)
{
    return (matrix3Name + mx::GlslFragmentGenerator::MATRIX3_TO_MATRIX4_POSTFIX);
}
