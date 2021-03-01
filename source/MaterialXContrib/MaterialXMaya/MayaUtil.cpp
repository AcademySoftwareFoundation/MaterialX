#include "MayaUtil.h"
#include "Plugin.h"

#include <maya/MViewport2Renderer.h>
#include <maya/MFragmentManager.h>

namespace MaterialXMaya
{

namespace MayaUtil
{

void registerFragment(
    const std::string& fragmentName,
    const std::string& fragmentSource,
    const std::string& lightRigName,
    const std::string& lightRigSource)
{
    if (fragmentName.empty())
    {
        throw std::runtime_error("Cannot register fragment with an empty name");
    }

    if (fragmentSource.empty())
    {
        throw std::runtime_error("Cannot register fragment with an empty source");
    }

    MHWRender::MRenderer* const theRenderer = MHWRender::MRenderer::theRenderer();
    MHWRender::MFragmentManager* const fragmentManager = theRenderer ? theRenderer->getFragmentManager() : nullptr;

    if (!fragmentManager)
    {
        throw std::runtime_error("Failed to get the VP2 fragment manager");
    }

    constexpr bool hidden = false;

    if (!fragmentManager->hasFragment(fragmentName.c_str()))
    {
        const MString registeredFragment = fragmentManager->addShadeFragmentFromBuffer(fragmentSource.c_str(), hidden);
        if (fragmentName != registeredFragment.asChar())
        {
            throw std::runtime_error("Failed to register shader fragment '" + fragmentName + "'");
        }
    }

    if (!lightRigSource.empty()) {
        if (lightRigName.empty())
        {
            throw std::runtime_error("Cannot register light rig with an empty name");
        }
        if (!fragmentManager->hasFragment(lightRigName.c_str())) {
            const MString registeredFragment = fragmentManager->addFragmentGraphFromBuffer(lightRigSource.c_str());
            if (lightRigName != registeredFragment.asChar())
            {
                throw std::runtime_error("Failed to register light rig '" + lightRigName + "'");
            }
        }
    }
}

void TextureDeleter::operator()(MHWRender::MTexture* texture)
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

    MHWRender::MTextureManager* const textureMgr = renderer->getTextureManager();

    if (!textureMgr)
    {
        return;
    }

    textureMgr->releaseTexture(texture);
};

void SamplerDeleter::operator()(const MHWRender::MSamplerState* sampler)
{
    if (sampler)
    {
        MHWRender::MStateManager::releaseSamplerState(sampler);
    }
};

} // namespace MayaUtil
} // namespace MaterialXMaya
