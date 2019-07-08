#include "MayaUtil.h"
#include "Plugin.h"

#include <maya/MViewport2Renderer.h>
#include <maya/MFragmentManager.h>

namespace MaterialXMaya
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
            const MString registeredFragment =
                fragmentManager->addShadeFragmentFromBuffer(fragmentSource.c_str(), hidden);

            if (registeredFragment.length() == 0)
            {
                throw std::runtime_error("Failed to add shader fragment: (" + fragmentName + ")");
            }
        }
    }
}

