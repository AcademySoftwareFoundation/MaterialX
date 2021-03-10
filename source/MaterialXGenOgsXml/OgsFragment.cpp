#include "OgsFragment.h"

#include <MaterialXFormat/XmlIo.h>
#include <MaterialXGenShader/Util.h>
#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenOgsXml/GlslFragmentGenerator.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenOgsXml/OgsXmlGenerator.h>

#ifdef MATERIALX_BUILD_CROSS
#include <MaterialXCross/Cross.h>
#endif

#include <iostream>

namespace MaterialXMaya
{
namespace
{
// The base class for classes wrapping GLSL fragment generators for use during
// OgsFragment construction.
class GlslGeneratorWrapperBase
{
    GlslGeneratorWrapperBase() = delete;

  protected:
    explicit GlslGeneratorWrapperBase(mx::ElementPtr element)
        : _element(element)
    {
        if (!_element)
            throw mx::Exception("No element specified");

        mx::TypedElementPtr typeElement = element->asA<mx::TypedElement>();
        if (typeElement && typeElement->getType() == mx::SURFACE_SHADER_TYPE_STRING)
        {
            _isSurface = true;
        }
        else if (element->isA<mx::Node>())
        {
            mx::NodePtr outputNode = element->asA<mx::Node>();
            if (outputNode->getType() == mx::MATERIAL_TYPE_STRING)
            {
                std::unordered_set<mx::NodePtr> shaderNodes =
                    mx::getShaderNodes(outputNode, mx::SURFACE_SHADER_TYPE_STRING);
                if (!shaderNodes.empty())
                {
                    _element = *shaderNodes.begin();
                    _isSurface = true;
                }
            }
            else if (outputNode->getType() == mx::SURFACE_SHADER_TYPE_STRING)
            {
                _isSurface = true;
            }
        }
        else if (!element->asA<mx::Output>())
        {
            throw mx::Exception("Invalid element to create fragment for " + element->getName());
        }
    }

  protected:
    void setCommonOptions(mx::GenOptions& genOptions, const mx::ShaderGenerator& generator)
    {
        // Use FIS environment lookup for surface shader generation but
        // disable for texture nodes to avoid additional unneeded XML parameter
        // generation.    
        genOptions.hwSpecularEnvironmentMethod = mx::SPECULAR_ENVIRONMENT_NONE;

        // Set to use no direct lighting
        genOptions.hwMaxActiveLightSources = _isSurface ? 16 : 0;

        // Maya images require a texture coordinates to be flipped in V.
        genOptions.fileTextureVerticalFlip = true;
        genOptions.hwTransparency = mx::isTransparentSurface(_element, generator.getTarget());

        // Maya viewport uses texture atlas for tile image so enabled
        // texture coordinate transform to go from original UDIM range to
        // normalized 0..1 range.
        genOptions.hwNormalizeUdimTexCoords = true;
    }

    mx::ElementPtr _element;

private:
    bool _isSurface = false;
};

// Knows how to create a temporary local GLSL fragment generator to generate
// GLSL fragment code during OgsFragment construction.
//
class LocalGlslGeneratorWrapper
    : public GlslGeneratorWrapperBase
{
public:
    LocalGlslGeneratorWrapper(
        mx::ElementPtr element,
        const mx::FileSearchPath& librarySearchPath
    )
        : GlslGeneratorWrapperBase(element)
        , _librarySearchPath(librarySearchPath)
    {
    }

    ~LocalGlslGeneratorWrapper()
    {}

    mx::ShaderPtr operator()(const std::string& baseFragmentName)
    {
        mx::ShaderGeneratorPtr generator = mx::GlslFragmentGenerator::create();
        mx::GenContext genContext(generator);
        mx::GenOptions& genOptions = genContext.getOptions();

        // Set up color management. We assume the target render space is linear
        // if not found in the document. Currently the default system has no other color space targets.
        //
        if ( mx::DefaultColorManagementSystemPtr colorManagementSystem =
            mx::DefaultColorManagementSystem::create(generator->getTarget())
        )
        {
            generator->setColorManagementSystem(colorManagementSystem);

            mx::DocumentPtr document = _element->getDocument();
            colorManagementSystem->loadLibrary(document);
            const std::string& documentColorSpace = document->getAttribute(mx::Element::COLOR_SPACE_ATTRIBUTE);

            static const std::string MATERIALX_LINEAR_WORKING_SPACE("lin_rec709");
            genOptions.targetColorSpaceOverride =
                documentColorSpace.empty() ? MATERIALX_LINEAR_WORKING_SPACE : documentColorSpace;
        }

        // Set up unit system. We assume default distance unit of 1 meter:
        if (mx::UnitSystemPtr unitSystem = mx::UnitSystem::create(generator->getTarget()))
        {
            generator->setUnitSystem(unitSystem);
            mx::UnitConverterRegistryPtr registry = mx::UnitConverterRegistry::create();
            mx::DocumentPtr document = _element->getDocument();
            mx::UnitTypeDefPtr distanceTypeDef = document->getUnitTypeDef("distance");
            registry->addUnitConverter(distanceTypeDef, mx::LinearUnitConverter::create(distanceTypeDef));
            mx::UnitTypeDefPtr angleTypeDef = document->getUnitTypeDef("angle");
            registry->addUnitConverter(angleTypeDef, mx::LinearUnitConverter::create(angleTypeDef));
            generator->getUnitSystem()->loadLibrary(document);
            generator->getUnitSystem()->setUnitConverterRegistry(registry);

            // Set target unit space
            genOptions.targetDistanceUnit = "meter";
        }

        genContext.registerSourceCodeSearchPath(_librarySearchPath);
        
        setCommonOptions(genOptions, *generator);

        // Every light ends up as a directional light once processed thru Maya:
        mx::DocumentPtr document = _element->getDocument();
        mx::NodeDefPtr directionalLightShader = document->getNodeDef("ND_directional_light");
        mx::HwShaderGenerator::bindLightShader(*directionalLightShader, 1, genContext);

        return generator->generate(baseFragmentName, _element, genContext);
    }

    const mx::FileSearchPath& _librarySearchPath;
};

// Wraps an externally-provided GLSL fragment generator (such as the one
// created once for multiple tests by the test harness) to generate
// GLSL fragment code during OgsFragment construction.
//
class ExternalGlslGeneratorWrapper
    : public GlslGeneratorWrapperBase
{
public:
    ExternalGlslGeneratorWrapper(mx::ElementPtr element, mx::GenContext& genContext)
        : GlslGeneratorWrapperBase(element)
        , _genContext(genContext)
    {}

    ~ExternalGlslGeneratorWrapper()
    {}

    mx::ShaderPtr operator()(const std::string& baseFragmentName)
    {
        mx::ShaderGenerator& generator = _genContext.getShaderGenerator();
        mx::GenOptions& genOptions = _genContext.getOptions();

        setCommonOptions(genOptions, generator);

        return generator.generate(baseFragmentName, _element, _genContext);
    }

private:
    mx::GenContext& _genContext;
};

/// Generate the complete XML fragment source embedding both GLSL and HLSL code.
/// @return The unique name of the fragment
std::string
generateFragment(
    std::string& fragmentSource,
    const mx::Shader& glslShader,
    const std::string& baseFragmentName
)
{
    static const std::string FRAGMENT_NAME_TOKEN = "$fragmentName";

    {
        std::string hlslSource;
#ifdef MATERIALX_BUILD_CROSS
        try
        {
            // Cross-compile GLSL fragment code generated by MaterialX to HLSL
            //
            hlslSource = mx::Cross::glslToHlsl(
                glslShader.getSourceCode(mx::Stage::UNIFORMS),
                glslShader.getSourceCode(mx::Stage::PIXEL),
                baseFragmentName
            );
        }
        catch (std::exception& e)
        {
            std::cerr << "Failed to cross-compile GLSL fragment to HLSL: " << e.what() << "\n";
        }
#endif
        // Generate the XML wrapper for the fragment embedding both the GLSL
        // and HLSL code.
        // Supply a placeholder name token to be replaced with an actual unique
        // name later.
        //
        fragmentSource = mx::OgsXmlGenerator::generate(
            FRAGMENT_NAME_TOKEN,
            glslShader,
            hlslSource
        );
        if (fragmentSource.empty())
        {
            throw mx::Exception("Generated fragment source is empty");
        }
    }

    // Strip out any '\r' characters.
    fragmentSource.erase(
        std::remove(fragmentSource.begin(), fragmentSource.end(), '\r'),
        fragmentSource.end()
    );

    // Hash the generated fragment source and use it to generate a unique
    // fragment name to use for registration with Maya API that won't clash
    // with other fragments (possibly different versions of the same
    // MaterialX fragment).
    std::ostringstream nameStream;
    const size_t sourceHash = std::hash<std::string>{}(fragmentSource);
    nameStream << baseFragmentName << "__" << std::hex << sourceHash;
    std::string fragmentName = nameStream.str();

    // Substitute the placeholder name token with the actual name.
    //
    const mx::StringMap substitutions{ {FRAGMENT_NAME_TOKEN, fragmentName} };
    mx::tokenSubstitution(substitutions, fragmentSource);

    return fragmentName;
}

/// Generate a fragment graph linking Maya lights to the generated fragment:
/// @return The unique name of the fragment
std::string
generateLightRig(
    std::string& lightRigSource,
    const mx::Shader& glslShader,
    const std::string& baseFragmentName
)
{
    static const std::string FRAGMENT_NAME_TOKEN = "$fragmentName";
    static const std::string BASE_FRAGMENT_NAME_TOKEN = "$baseFragmentName";

    {
        // Supply a placeholder name token to be replaced with an actual unique
        // name later.
        //
        lightRigSource = mx::OgsXmlGenerator::generateLightRig(
            FRAGMENT_NAME_TOKEN,
            BASE_FRAGMENT_NAME_TOKEN,
            glslShader
        );
        if (lightRigSource.empty())
        {
            throw mx::Exception("Generated light rig is empty");
        }
    }

    // Strip out any '\r' characters.
    lightRigSource.erase(
        std::remove(lightRigSource.begin(), lightRigSource.end(), '\r'),
        lightRigSource.end()
    );

    std::ostringstream nameStream;
    const size_t sourceHash = std::hash<std::string>{}(lightRigSource);
    nameStream << "Lit_" << baseFragmentName << "__" << std::hex << sourceHash;
    std::string fragmentName = nameStream.str();

    // Substitute the placeholder name token with the actual name.
    //
    const mx::StringMap substitutions{ {FRAGMENT_NAME_TOKEN, fragmentName},
                                       {BASE_FRAGMENT_NAME_TOKEN, baseFragmentName} };
    mx::tokenSubstitution(substitutions, lightRigSource);

    return fragmentName;
}
} // anonymous namespace

OgsFragment::OgsFragment(
    mx::ElementPtr element,
    const mx::FileSearchPath& librarySearchPath
)
    : OgsFragment(
        element,
        LocalGlslGeneratorWrapper(element, librarySearchPath)
    )
{
}

OgsFragment::OgsFragment(mx::ElementPtr element, mx::GenContext& genContext)
    : OgsFragment(
        element,
        ExternalGlslGeneratorWrapper(element, genContext)
    )
{
}

template <typename GLSL_GENERATOR_WRAPPER>
OgsFragment::OgsFragment(
    mx::ElementPtr element,
    GLSL_GENERATOR_WRAPPER&& glslGeneratorWrapper
)
    : _element(element)
{
    if (!_element)
        throw mx::Exception("No element specified");

    // The non-unique name of the fragment.
    // Must match the name of the root function of the fragment.
    const std::string baseFragmentName = mx::createValidName(_element->getNamePath());

    // Generate the GLSL version of the fragment.
    //
    _glslShader = glslGeneratorWrapper(baseFragmentName);
    if (!_glslShader)
    {
        throw mx::Exception("Failed to generate GLSL fragment code");
    }

    // Generate the complete XML fragment source embedding both GLSL and HLSL
    // code.
    _fragmentName = generateFragment(
        _fragmentSource, *_glslShader, baseFragmentName
    );

    const mx::ShaderGraph& graph = _glslShader->getGraph();
    bool lighting = graph.hasClassification(mx::ShaderNode::Classification::SHADER | mx::ShaderNode::Classification::SURFACE) ||
                    graph.hasClassification(mx::ShaderNode::Classification::BSDF);
    if (lighting)
    {
        _lightRigName = generateLightRig(
            _lightRigSource, *_glslShader, _fragmentName
        );
    }

    // Extract the input fragment parameter names along with their
    // associated element paths to allow for value binding.
    //
    const mx::ShaderStage& pixelShader = _glslShader->getStage(mx::Stage::PIXEL);
    for (const auto& uniformBlock : pixelShader.getUniformBlocks())
    {
        const mx::VariableBlock& uniforms = *uniformBlock.second;

        // Skip light uniforms
        if (uniforms.getName() == mx::HW::LIGHT_DATA)
        {
            continue;
        }

        for (size_t i = 0; i < uniforms.size(); ++i)
        {
            const mx::ShaderPort* const port = uniforms[i];
            const std::string& path = port->getPath();
            if (!path.empty())
            {
                const std::string& variableName = port->getVariable();
                if (port->getType()->getSemantic() == mx::TypeDesc::SEMANTIC_FILENAME)
                {
                    std::string textureName = mx::OgsXmlGenerator::samplerToTextureName(variableName);
                    if (!textureName.empty())
                    {
                        _pathInputMap[path] = textureName;
                        continue;
                    }
                }
                _pathInputMap[path] = variableName;
            }
        }
    }
}

OgsFragment::~OgsFragment()
{
}

const std::string& OgsFragment::getFragmentName() const
{
    return _fragmentName;
}

const std::string& OgsFragment::getFragmentSource() const
{
    return _fragmentSource;
}

const std::string& OgsFragment::getLightRigName() const
{
    return _lightRigName;
}

const std::string& OgsFragment::getLightRigSource() const
{
    return _lightRigSource;
}

const mx::StringMap& OgsFragment::getPathInputMap() const
{
    return _pathInputMap;
}

bool OgsFragment::isElementAShader() const
{
    mx::TypedElementPtr typeElement = _element ? _element->asA<mx::TypedElement>() : nullptr;
    return typeElement && typeElement->getType() == mx::SURFACE_SHADER_TYPE_STRING;
}

bool OgsFragment::isTransparent() const
{
    return _glslShader && _glslShader->hasAttribute(mx::HW::ATTR_TRANSPARENT);
}

mx::ImageSamplingProperties
OgsFragment::getImageSamplingProperties(const std::string& fileParameterName) const
{
    mx::ImageSamplingProperties samplingProperties;
    if (_glslShader && _glslShader->hasStage(mx::Stage::PIXEL))
    {
        mx::ShaderStage& stage = _glslShader->getStage(mx::Stage::PIXEL);
        mx::VariableBlock& block = stage.getUniformBlock(mx::HW::PUBLIC_UNIFORMS);
        samplingProperties.setProperties(fileParameterName, block);

    }
    return samplingProperties;
}

std::string OgsFragment::getMatrix4Name(const std::string& matrix3Name)
{
    return matrix3Name + mx::GlslFragmentGenerator::MATRIX3_TO_MATRIX4_POSTFIX;
}

} // namespace MaterialXMaya
