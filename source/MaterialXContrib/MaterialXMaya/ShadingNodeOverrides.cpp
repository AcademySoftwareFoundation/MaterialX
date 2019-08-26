#include "ShadingNodeOverrides.h"
#include "MaterialXNode.h"
#include "Plugin.h"
#include "MaterialXUtil.h"
#include "MayaUtil.h"
#include "OgsFragment.h"

#include <maya/MDGModifier.h>
#include <maya/MShaderManager.h>
#include <maya/MPxShadingNodeOverride.h>
#include <maya/MPxSurfaceShadingNodeOverride.h>

#include <MaterialXGenShader/Util.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenOgsXml/OgsXmlGenerator.h>
#include <MaterialXRender/ImageHandler.h>

#include <iostream>

namespace mx = MaterialX;

namespace MHWRender
{
    class MPxShadingNodeOverride;
    class MPxSurfaceShadingNodeOverride;
};

namespace MaterialXMaya
{

const MString
    SurfaceOverride::REGISTRANT_ID = "materialXSurface",
    SurfaceOverride::DRAW_CLASSIFICATION = "drawdb/shader/surface/materialX";

const MString
    TextureOverride::REGISTRANT_ID = "materialXTexture",
    TextureOverride::DRAW_CLASSIFICATION = "drawdb/shader/texture/2d/materialX";

namespace
{
// This should be a shared utility
MStatus bindFileTexture(MHWRender::MShaderInstance& shader, 
                        const std::string& parameterName,
                        const mx::FileSearchPath& searchPath, 
                        const std::string& fileName,
                        const MHWRender::MSamplerStateDesc& samplerDescription,
                        MHWRender::MTextureDescription& textureDescription,
                        const mx::StringVec* udimIdentifiers = nullptr)
{
    MStatus status = MStatus::kFailure;

    // Bind file texture
    MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
    MHWRender::MTextureManager* textureManager = renderer ? textureManager = renderer->getTextureManager() : nullptr;
    if (textureManager)
    {
        MayaUtil::TextureUniquePtr texturePtr;
        if (udimIdentifiers && !udimIdentifiers->empty())
        {
            std::vector<mx::Vector2> udimCoordinates{ mx::getUdimCoordinates(*udimIdentifiers) };
            mx::FilePathVec udimPaths{ mx::getUdimPaths(fileName, *udimIdentifiers) };
            // Make sure we have 1:1 match of paths to coordinates.
            if (udimCoordinates.size() != udimPaths.size())
            {
                throw mx::Exception("Failed to resolve UDIM information properly for file: " + fileName);
            }
            MStringArray mTilePaths;
            MFloatArray mTilePositions;
            MColor undefinedColor;
            MStringArray failedTilePaths;
            MFloatArray uvScaleOffset;
            for (size_t i = 0; i < udimPaths.size(); ++i)
            {
                mx::FilePath resolvedPath = MaterialXUtil::findInSubdirectories(searchPath, udimPaths[i]);
                mTilePaths.append(resolvedPath.asString().c_str());
                mTilePositions.append(udimCoordinates[i][0]);
                mTilePositions.append(udimCoordinates[i][1]);
            }
            mx::Vector2 scaleUV;
            mx::Vector2 offsetUV;
            mx::getUdimScaleAndOffset(udimCoordinates, scaleUV, offsetUV);

            unsigned int udimBakeWidth = 4096;
            unsigned int udimBakeHeight = 4096;
            float ratio = scaleUV[1] / scaleUV[0];
            if (ratio > 1.0)
            {
                udimBakeHeight = static_cast<unsigned int>(std::truncf(static_cast<float>(udimBakeHeight) * ratio));
            }
            else
            {
                udimBakeWidth = static_cast<unsigned int>(std::truncf(static_cast<float>(udimBakeWidth) * ratio));
            }
            // Note: we do not use the uv scale and offset. Ideally this should be used for the texture lookup code
            // but at this point the shader code has already been generated.
            texturePtr.reset(textureManager->acquireTiledTexture(fileName.c_str(), mTilePaths, mTilePositions,
                                                                 undefinedColor, udimBakeWidth, udimBakeHeight,
                                                                 failedTilePaths, uvScaleOffset));
        }
        else
        {
            mx::FilePath imagePath = MaterialXUtil::findInSubdirectories(searchPath, fileName);
            if (!imagePath.isEmpty())
            {
                texturePtr.reset(textureManager->acquireTexture(imagePath.asString().c_str(), mx::EMPTY_STRING.c_str()));
            }
        }

        MHWRender::MTextureAssignment textureAssignment;
        textureAssignment.texture = texturePtr.get();
        if (texturePtr)
        {
            // Get back the texture description
            texturePtr->textureDescription(textureDescription);
        }
        else
        {
            std::cerr << "*Unable to find image file: " << fileName << " in search paths: "
                << searchPath.asString() << std::endl;
        }
        status = shader.setParameter(parameterName.c_str(), textureAssignment);
        if (!status)
        {
            std::cerr << "*Unable to bind image file: " << fileName << std::endl;
        }
    }

    // Bind sampler
    static const std::string SAMPLE_PREFIX_STRING("Sampler");
    const std::string samplerParameterName = parameterName + SAMPLE_PREFIX_STRING;
    MayaUtil::SamplerUniquePtr samplerState{ MHWRender::MStateManager::acquireSamplerState(samplerDescription) };
    if (samplerState)
    {
        status = shader.setParameter(samplerParameterName.c_str(), *samplerState);
    }

    return status;
}

// This should be a shared utility
void bindEnvironmentLighting(MHWRender::MShaderInstance& shader,
                            const MStringArray parameterList,
                            const mx::FileSearchPath imageSearchPath,
                            const MaterialXNode& node)
{
    MHWRender::MSamplerStateDesc samplerDescription;
    samplerDescription.filter = MHWRender::MSamplerState::kAnisotropic;
    samplerDescription.maxAnisotropy = 16;

    MStatus status;

    // Set irradiance map
    MHWRender::MTextureDescription textureDescription;
    if (parameterList.indexOf(mx::HW::ENV_IRRADIANCE.c_str()) >= 0)
    {
        status = bindFileTexture(shader, mx::HW::ENV_IRRADIANCE, imageSearchPath,
                                 node.getEnvIrradianceFileName().asChar(), samplerDescription, textureDescription, nullptr);
    }

    // Set radiance map
    if (parameterList.indexOf(mx::HW::ENV_RADIANCE.c_str()) >= 0)
    {
        status = bindFileTexture(shader, mx::HW::ENV_RADIANCE, imageSearchPath,
                                 node.getEnvRadianceFileName().asChar(), samplerDescription, textureDescription, nullptr);
        if (status == MStatus::kSuccess)
        {
            if (parameterList.indexOf(mx::HW::ENV_RADIANCE_MIPS.c_str()) >= 0)
            {
                const int mipCount = static_cast<int>(std::log2(std::max(textureDescription.fWidth, textureDescription.fHeight))) + 1;
                status = shader.setParameter(mx::HW::ENV_RADIANCE_MIPS.c_str(), mipCount);
            }

            if (parameterList.indexOf(mx::HW::ENV_RADIANCE_SAMPLES.c_str()) >= 0)
            {
                constexpr int envSamples = 16;
                status = shader.setParameter(mx::HW::ENV_RADIANCE_SAMPLES.c_str(), envSamples);
            }
        }
    }

    // Environment matrix
    if (parameterList.indexOf(mx::HW::ENV_MATRIX.c_str()) >= 0)
    {
        constexpr float Y_ROTATION_PI[4][4]
        {
            -1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, -1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };

        static const MFloatMatrix ENV_MATRIX(Y_ROTATION_PI);
        status = shader.setParameter(mx::HW::ENV_MATRIX.c_str(), ENV_MATRIX);
    }
}

} // anonymous namespace

template <class BASE>
ShadingNodeOverride<BASE>::ShadingNodeOverride(const MObject& obj)
    : BASE(obj)
    , _object(obj)
{
}

template <class BASE>
ShadingNodeOverride<BASE>::~ShadingNodeOverride()
{
}

template <class BASE>
MString
ShadingNodeOverride<BASE>::fragmentName() const
{
    MStatus status;
    MFnDependencyNode depNode(_object, &status);
    const auto* const node = dynamic_cast<MaterialXNode*>(depNode.userNode());
    const OgsFragment* const data = node ? node->getOgsFragment() : nullptr;
    return data ? data->getFragmentName().c_str() : "";
}

template <class BASE>
bool
ShadingNodeOverride<BASE>::valueChangeRequiresFragmentRebuild(const MPlug* plug) const
{
    if (   *plug == MaterialXNode::DOCUMENT_ATTRIBUTE
        || *plug == MaterialXNode::ELEMENT_ATTRIBUTE
        || *plug == MaterialXNode::ENV_RADIANCE_ATTRIBUTE
        || *plug == MaterialXNode::ENV_IRRADIANCE_ATTRIBUTE
    )
    {
        return true;
    }

    return BASE::valueChangeRequiresFragmentRebuild(plug);
}

template <class BASE>
void ShadingNodeOverride<BASE>::updateShader(MHWRender::MShaderInstance& shader,
                                             const MHWRender::MAttributeParameterMappingList& mappings)
{
    MStatus status;
    MFnDependencyNode depNode(_object, &status);
    const auto* const node = dynamic_cast<MaterialXNode*>(depNode.userNode());
    if (!node)
    {
        return;
    }

    const OgsFragment* const ogsFragment = node->getOgsFragment();
    if (!ogsFragment)
    {
        return;
    }

    // Get the parameter list to check existence against.
    MStringArray parameterList;
    shader.parameterList(parameterList);

    // Set up image file name search path.
    mx::FilePath documentPath(node->getDocumentFilePath().asChar());
    documentPath = documentPath.getParentPath();
    mx::FileSearchPath imageSearchPath = Plugin::instance().getResourceSearchPath(); 
    imageSearchPath.prepend(documentPath);

    bindEnvironmentLighting(shader, parameterList, imageSearchPath, *node);

    mx::DocumentPtr document = ogsFragment->getDocument();

    // Look for any udimset on the document to use for texture binding.
    mx::ValuePtr udimSetValue = document->getGeomAttrValue("udimset");
    const mx::StringVec* udimIdentifiers = nullptr;
    if (udimSetValue && udimSetValue->isA<mx::StringVec>())
    {
        udimIdentifiers = &(udimSetValue->asA<mx::StringVec>());
    }

    // Address mode mapping
    const std::vector<MSamplerState::TextureAddress> addressModes
    {
        MSamplerState::TextureAddress::kTexBorder,
        MSamplerState::TextureAddress::kTexClamp,
        MSamplerState::TextureAddress::kTexWrap,
        MSamplerState::TextureAddress::kTexMirror
    };

    const std::vector<MHWRender::MSamplerState::TextureFilter> filterModes
    { 
        MHWRender::MSamplerState::kMinMagMipPoint,
        MHWRender::MSamplerState::kMinMagMipLinear,
        MHWRender::MSamplerState::kAnisotropic
    };

    const mx::StringMap& inputs = ogsFragment->getPathInputMap();
    for (const auto& input : inputs)
    {
        mx::ElementPtr element = document->getDescendant(input.first);
        if (!element)
        {
            continue;
        }

        mx::ValueElementPtr valueElement = element->asA<mx::ValueElement>();
        if (!valueElement)
        {
            continue;
        }

        const std::string& inputName = input.second;
        const MHWRender::MAttributeParameterMapping* const mapping =
            mappings.findByParameterName(inputName.c_str());

        const MString resolvedName = mapping ? mapping->resolvedParameterName() : inputName.c_str();

        // Bind values
        mx::ValuePtr mtxValue = valueElement->getValue();
        if (mtxValue)
        {
            if (mtxValue->isA<mx::Matrix44>())
            {
                mx::Matrix44 matrix44 = mtxValue->asA<mx::Matrix44>();
                MFloatMatrix mayaValue;
                for (unsigned int i = 0; i < 4; i++)
                {
                    for (unsigned int j = 0; j < 4; j++)
                    {
                        mayaValue[i][j] = matrix44[i][j];
                    }
                }
                status = shader.setParameter(resolvedName, mayaValue);
            }
            else if (mtxValue->isA<mx::Matrix33>())
            {
                // Must promote matrix33 to matrix44 as the API
                // has no matrix33 parameter binding interface.
                mx::Matrix33 matrix33 = mtxValue->asA<mx::Matrix33>();
                MFloatMatrix mayaValue;
                mayaValue.setToIdentity();
                for (unsigned int i = 0; i < 3; i++)
                {
                    for (unsigned int j = 0; j < 3; j++)
                    {
                        mayaValue[i][j] = matrix33[i][j];
                    }
                }
                // Note: the parameter exposed uses a derived matrix44 name.
                std::string matrix4Name = OgsFragment::getMatrix4Name(resolvedName.asChar());
                status = shader.setParameter(matrix4Name.c_str(), mayaValue);
            }

            else if (valueElement->getType() == mx::FILENAME_TYPE_STRING)
            {
                // This is the hard-coded OGS convention to associate a texture with a sampler (via post-fix "Sampler" string)
                const std::string textureParameterName(resolvedName.asChar());

                // Bind texture and sampler
                const std::string& valueString = valueElement->getValueString();
                if (!valueString.empty())
                {
                    MHWRender::MTextureDescription textureDescription;

                    mx::ImageSamplingProperties samplingProperties
                        = ogsFragment->getImageSamplngProperties(textureParameterName);

                    MHWRender::MSamplerStateDesc samplerDescription;

                    // Set border color
                    samplerDescription.borderColor[0] = samplingProperties.defaultColor[0];
                    samplerDescription.borderColor[1] = samplingProperties.defaultColor[1];
                    samplerDescription.borderColor[2] = samplingProperties.defaultColor[2];
                    samplerDescription.borderColor[3] = samplingProperties.defaultColor[3];

                    // Map address modes
                    samplerDescription.addressV = MSamplerState::TextureAddress::kTexWrap;
                    if (samplingProperties.vaddressMode != mx::ImageSamplingProperties::AddressMode::UNSPECIFIED)
                    {
                        samplerDescription.addressV = addressModes[static_cast<int>(samplingProperties.vaddressMode)];
                    }
                    samplerDescription.addressU = MSamplerState::TextureAddress::kTexWrap;
                    if (samplingProperties.uaddressMode != mx::ImageSamplingProperties::AddressMode::UNSPECIFIED)
                    {
                        samplerDescription.addressU = addressModes[static_cast<int>(samplingProperties.uaddressMode)];
                    }

                    // Map filter type
                    samplerDescription.filter = MHWRender::MSamplerState::kMinMagMipLinear;
                    samplerDescription.maxAnisotropy = 16;
                    if (samplingProperties.filterType != mx::ImageSamplingProperties::FilterType::UNSPECIFIED)
                    {
                        samplerDescription.filter = filterModes[static_cast<int>(samplingProperties.filterType)];
                    }

                    status = bindFileTexture(shader, textureParameterName, imageSearchPath, valueString,
                        samplerDescription, textureDescription, udimIdentifiers);
                }
            }
        }
    }
}

template class ShadingNodeOverride<MHWRender::MPxShadingNodeOverride>;
template class ShadingNodeOverride<MHWRender::MPxSurfaceShadingNodeOverride>;

MHWRender::MPxSurfaceShadingNodeOverride*
SurfaceOverride::creator(const MObject& obj)
{
    std::cout.rdbuf(std::cerr.rdbuf());
    return new SurfaceOverride(obj);
}

MString
SurfaceOverride::transparencyParameter() const
{
    return mx::OgsXmlGenerator::VP_TRANSPARENCY_NAME.c_str();
}

MHWRender::MPxShadingNodeOverride* TextureOverride::creator(const MObject& obj)
{
    std::cout.rdbuf(std::cerr.rdbuf());
    return new TextureOverride(obj);
}

} // namespace MaterialXMaya
