#ifndef MATERIALX_MAYA_MAYAUTIL_H
#define MATERIALX_MAYA_MAYAUTIL_H

/// @file
/// Maya Viewport 2.0 utilities.

#include <maya/MStateManager.h>
#include <maya/MTextureManager.h>

#include <string>
#include <memory>

namespace MaterialXMaya
{
namespace MayaUtil
{

/// Registers an OGS shade fragment in VP2. Throws an exception if a fragment
/// with the same name is already registered.
/// @param The unique name of the fragments to use for registration.
/// @param The code of the fragments stored in a string.
void registerFragment(const std::string& fragmentName, const std::string& fragmentSource,
                      const std::string& lightRigName, const std::string& lightRigSource);

struct TextureDeleter
{
    /// Releases the reference to the VP2 texture owned by a smart pointer.
    void operator()(MHWRender::MTexture* texture);
};

using TextureUniquePtr = std::unique_ptr<MHWRender::MTexture, TextureDeleter>;

struct SamplerDeleter
{
    /// Releases the reference to the VP2 sampler owned by a smart pointer.
    void operator()(const MHWRender::MSamplerState* sampler);
};

using SamplerUniquePtr = std::unique_ptr<const MHWRender::MSamplerState, SamplerDeleter>;

} // namespace MayaUtil
} // namespace MaterialXMaya

#endif
