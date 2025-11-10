//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_SLANGSHADERCACHE_H
#define MATERIALX_SLANGSHADERCACHE_H

#include <MaterialXRenderSlang/Export.h>
#include <MaterialXRenderSlang/SlangRhi.h>
#include <MaterialXFormat/File.h>

#include <map>

MATERIALX_NAMESPACE_BEGIN

class SlangShaderCache : public rhi::IPersistentCache
{
  public:
    SlangShaderCache(const FilePath& _cachePath);

    SLANG_NO_THROW rhi::Result SLANG_MCALL writeCache(ISlangBlob* key, ISlangBlob* data) override;
    SLANG_NO_THROW rhi::Result SLANG_MCALL queryCache(ISlangBlob* key, ISlangBlob** outData) override;

    SLANG_NO_THROW SlangResult SLANG_MCALL
    queryInterface(SlangUUID const& uuid, void** outObject) override
    {
        if (uuid == rhi::IPersistentCache::getTypeGuid())
        {
            *outObject = static_cast<rhi::IPersistentCache*>(this);
            return SLANG_OK;
        }
        return SLANG_E_NO_INTERFACE;
    }

    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() override
    {
        return ++_refCount;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() override
    {
        const uint32_t count = --_refCount;
        if (count == 0)
            delete this;
        return count;
    }

  private:
    FilePath _cachePath;
    std::atomic<uint32_t> _refCount{ 0 };
};

MATERIALX_NAMESPACE_END

#endif
