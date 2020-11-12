#include "MayaUtil.h"
#include "Plugin.h"

#include <maya/MViewport2Renderer.h>
#include <maya/MFragmentManager.h>

namespace MaterialXMaya
{

namespace MayaUtil
{

void registerFragment(const std::string& fragmentName, const std::string& fragmentSource)
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

    if (!fragmentManager->hasFragment(fragmentName.c_str()))
    {
        constexpr bool hidden = false;
        const MString registeredFragment = fragmentManager->addShadeFragmentFromBuffer(fragmentSource.c_str(), hidden);

        if (registeredFragment.length() == 0)
        {
            throw std::runtime_error("Failed to register shader fragment '" + fragmentName + "'");
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
