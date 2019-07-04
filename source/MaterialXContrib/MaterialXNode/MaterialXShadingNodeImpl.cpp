#include "MaterialXShadingNodeImpl.h"

#include "MaterialXNode.h"
#include "Plugin.h"
#include "MaterialXUtil.h"

#include <maya/MShaderManager.h>
#include <maya/MTextureManager.h>
#include <maya/MPxShadingNodeOverride.h>
#include <maya/MPxSurfaceShadingNodeOverride.h>

#include <MaterialXData.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXRender/ImageHandler.h>

namespace mx = MaterialX;

namespace
{
struct Vp2TextureDeleter
{
    void operator () (MHWRender::MTexture* texture)
    {
        if (!texture)
        {
            return;
        }

        MHWRender::MRenderer* const renderer = MRenderer::theRenderer();
        if (!renderer)
        {
            return;
        }

        MHWRender::MTextureManager* const
            textureMgr = renderer->getTextureManager();

        if (!textureMgr)
        {
            return;
        }

        textureMgr->releaseTexture(texture);
    };
};

using VP2TextureUniquePtr = std::unique_ptr<
    MHWRender::MTexture,
    Vp2TextureDeleter
>;

struct Vp2SamplerDeleter
{
    void operator () (const MHWRender::MSamplerState* sampler)
    {
        if (sampler)
        {
            MHWRender::MStateManager::releaseSamplerState(sampler);
        }
    };
};

using Vp2SamplerUniquePtr = std::unique_ptr<
    const MHWRender::MSamplerState,
    Vp2SamplerDeleter
>;

// This should be a shared utility
MStatus bindFileTexture(MHWRender::MShaderInstance& shader, 
                        const std::string& parameterName,
                        const MaterialX::FileSearchPath& searchPath, 
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
        VP2TextureUniquePtr texturePtr;
        if (udimIdentifiers && !udimIdentifiers->empty())
        {
            std::vector<mx::Vector2> udimCoordinates{ mx::ImageHandler::getUdimCoordinates(*udimIdentifiers) };
            mx::FilePathVec udimPaths{ mx::ImageHandler::getUdimPaths(fileName, *udimIdentifiers) };
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
            for (size_t i=0; i<udimPaths.size(); i++)
            {
                MaterialX::FilePath resolvedPath = MaterialXMaya::findInSubdirectories(searchPath, udimPaths[i]);
                mTilePaths.append(resolvedPath.asString().c_str());
                mTilePositions.append(udimCoordinates[i][0]);
                mTilePositions.append(udimCoordinates[i][1]);
            }
            const unsigned int UDIM_BAKE_WIDTH = 4096;
            const unsigned int UDIM_BAKE_HEIGHT = 4096;
            // Note: we do not use the uv scale and offset. Ideally this should be used for the texture lookup code
            // but at this point the shader code has already been generated.
            texturePtr.reset(textureManager->acquireTiledTexture(fileName.c_str(), mTilePaths, mTilePositions,
                                                                 undefinedColor, UDIM_BAKE_WIDTH, UDIM_BAKE_HEIGHT, 
                                                                 failedTilePaths, uvScaleOffset));
        }
        else
        {
            MaterialX::FilePath imagePath = MaterialXMaya::findInSubdirectories(searchPath, fileName);
            if (!imagePath.isEmpty())
            {
                texturePtr.reset(textureManager->acquireTexture(imagePath.asString().c_str(), MaterialX::EMPTY_STRING.c_str()));
            }
        }

        MHWRender::MTextureAssignment textureAssignment;
        textureAssignment.texture = texturePtr.get();
        if (texturePtr)
        {
            // Get back the texture description
            texturePtr->textureDescription(textureDescription);
        }
        status = shader.setParameter(parameterName.c_str(), textureAssignment);
        if (!status)
        {
            std::cerr << "*Unable to find or bind image file: " << fileName << " in search paths: "
                << searchPath.asString() << std::endl;
        }
    }

    // Bind sampler. This is not correct as it's not taking into account
    // the MaterialX sampler state.
    const std::string SAMPLE_PREFIX_STRING("Sampler");
    std::string samplerParameterName(parameterName + SAMPLE_PREFIX_STRING);
    Vp2SamplerUniquePtr samplerState{ MHWRender::MStateManager::acquireSamplerState(samplerDescription) };
    if (samplerState)
    {
        status = shader.setParameter(samplerParameterName.c_str(), *samplerState);
    }

    return status;
}

// This should be a shared utility
void bindEnvironmentLighting(MHWRender::MShaderInstance& shader,
                            const MStringArray parameterList,
                            const MaterialX::FileSearchPath imageSearchPath,
                            const std::string& envRadiancePath,
                            const std::string& envIrradiancePath)
{
    MHWRender::MSamplerStateDesc samplerDescription;
    samplerDescription.filter = MHWRender::MSamplerState::kAnisotropic;
    samplerDescription.maxAnisotropy = 16;

    MStatus status;

    // Set irradiance map
    MHWRender::MTextureDescription textureDescription;
    if (parameterList.indexOf(mx::HW::ENV_IRRADIANCE.c_str()) >= 0)
    {
        status = bindFileTexture(shader, mx::HW::ENV_IRRADIANCE, imageSearchPath, envIrradiancePath, samplerDescription, textureDescription, nullptr);
    }

    // Set radiance map
    if (parameterList.indexOf(mx::HW::ENV_RADIANCE.c_str()) >= 0)
    {
        status = bindFileTexture(shader, mx::HW::ENV_RADIANCE, imageSearchPath, envRadiancePath, samplerDescription, textureDescription, nullptr);
        if (status == MStatus::kSuccess)
        {
            if (parameterList.indexOf(mx::HW::ENV_RADIANCE_MIPS.c_str()) >= 0)
            {
                const int mipCount = (int)std::log2(std::max(textureDescription.fWidth, textureDescription.fHeight)) + 1;
                status = shader.setParameter(mx::HW::ENV_RADIANCE_MIPS.c_str(), mipCount);
            }

            if (parameterList.indexOf(mx::HW::ENV_RADIANCE_SAMPLES.c_str()) >= 0)
            {
                const int envSamples = 16;
                status = shader.setParameter(mx::HW::ENV_RADIANCE_SAMPLES.c_str(), int(envSamples));
            }
        }
    }

    // Environment matrix
    if (parameterList.indexOf(mx::HW::ENV_MATRIX.c_str()) >= 0)
    {
        const float yRotationPI[4][4]
        {
            -1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, -1, 0,
            0, 0, 0, 1
        };
        MFloatMatrix matrix(yRotationPI);
        status = shader.setParameter(mx::HW::ENV_MATRIX.c_str(), matrix);
    }
}

} // anonymous namespace

template <class BASE>
MaterialXShadingNodeImpl<BASE>::MaterialXShadingNodeImpl(const MObject& obj)
    : BASE(obj)
    , _object(obj)
{
}

template <class BASE>
MaterialXShadingNodeImpl<BASE>::~MaterialXShadingNodeImpl()
{
}

template <class BASE>
MString
MaterialXShadingNodeImpl<BASE>::fragmentName() const
{
    MStatus status;
    MFnDependencyNode depNode(_object, &status);
    const auto* const node = dynamic_cast<MaterialXNode*>(depNode.userNode());
    const MaterialXData* const data = node ? node->getMaterialXData() : nullptr;
    return data ? data->getFragmentName().c_str() : "";
}

template <class BASE>
void MaterialXShadingNodeImpl<BASE>::updateDG()
{
}

template <class BASE>
void MaterialXShadingNodeImpl<BASE>::updateShader(MHWRender::MShaderInstance& shader,
                                                  const MHWRender::MAttributeParameterMappingList& mappings)
{
    MStatus status;
    MFnDependencyNode depNode(_object, &status);
    const auto* const node = dynamic_cast<MaterialXNode*>(depNode.userNode());
    if (!node)
    {
        return;
    }

    const MaterialXData* const materialXData = node->getMaterialXData();
    if (!materialXData)
    {
        return;
    }

    // Get the parameter list to check existence against.
    MStringArray parameterList;
    shader.parameterList(parameterList);

    // Set up image file name search path. Assume we are using built in images located in resource path
    // TODO: Be able to add more image search paths.
    //static std::string IMAGE_FOLDER("Images");
    MaterialX::FileSearchPath imageSearchPath = Plugin::instance().getResourceSearchPath();
        //(getResourcePath() / MaterialX::FilePath(IMAGE_FOLDER));

    // Bind environment lighting
    // TODO: These should be options
    static const std::string
        envRadiancePath = "san_giuseppe_bridge.hdr",
        envIrradiancePath = "san_giuseppe_bridge_diffuse.hdr";

    ::bindEnvironmentLighting(shader, parameterList, imageSearchPath,
        envRadiancePath, envIrradiancePath);

    MaterialX::DocumentPtr document = materialXData->getDocument();

    // Look for any udimset on the document to use for texture binding.
    mx::ValuePtr udimSetValue = document->getGeomAttrValue("udimset");
    const mx::StringVec* udimIdentifiers = nullptr;
    if (udimSetValue && udimSetValue->isA<mx::StringVec>())
    {
        udimIdentifiers = &(udimSetValue->asA<mx::StringVec>());
    }

    const MaterialX::StringMap& inputs = materialXData->getPathInputMap();
    for (const auto& input : inputs)
    {
        MaterialX::ElementPtr element = document->getDescendant(input.first);
        if (!element)
        {
            continue;
        }

        MaterialX::ValueElementPtr valueElement = element->asA<MaterialX::ValueElement>();
        if (!valueElement)
        {
            continue;
        }

        const std::string& inputName = input.second;
        const MHWRender::MAttributeParameterMapping* const mapping =
            mappings.findByParameterName(inputName.c_str());

        const MString resolvedName = mapping ? mapping->resolvedParameterName() : inputName.c_str();

        if (valueElement->getType() == MaterialX::FILENAME_TYPE_STRING)
        {
            // This is the hard-coded OGS convention to associate a texture with a sampler (via post-fix "Sampler" string)
            const std::string textureParameterName(resolvedName.asChar());

            // Bind texture and sampler
            const std::string& valueString = valueElement->getValueString();
            if (!valueString.empty())
            {
                MHWRender::MTextureDescription textureDescription;

                // TODO: This should come from the element and not hard-coded.
                MHWRender::MSamplerStateDesc samplerDescription;
                samplerDescription.filter = MHWRender::MSamplerState::kAnisotropic;
                samplerDescription.maxAnisotropy = 16;

                status = ::bindFileTexture(shader, textureParameterName, imageSearchPath, valueString,
                                           samplerDescription, textureDescription, udimIdentifiers);
            }
        }
    }
}

namespace MHWRender
{
    class MPxShadingNodeOverride;
    class MPxSurfaceShadingNodeOverride;
};

template class MaterialXShadingNodeImpl<MHWRender::MPxShadingNodeOverride>;
template class MaterialXShadingNodeImpl<MHWRender::MPxSurfaceShadingNodeOverride>;
