//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HWLIGHTSHADERS_H
#define MATERIALX_HWLIGHTSHADERS_H

/// @file
/// Hardware shader generator lights

#include <MaterialXGenHw/Export.h>

#include <MaterialXGenShader/GenUserData.h>
#include <MaterialXGenShader/Shader.h>

MATERIALX_NAMESPACE_BEGIN

class HwLightShaders;
/// Shared pointer to a HwLightShaders
using HwLightShadersPtr = shared_ptr<class HwLightShaders>;

/// @class HwLightShaders
/// Hardware light shader user data
class MX_GENHW_API HwLightShaders : public GenUserData
{
  public:
    /// Create and return a new instance.
    static HwLightShadersPtr create()
    {
        return std::make_shared<HwLightShaders>();
    }

    /// Bind a light shader to a light type id.
    void bind(unsigned int type, ShaderNodePtr shader)
    {
        _shaders[type] = shader;
    }

    /// Unbind a light shader previously bound to a light type id.
    void unbind(unsigned int type)
    {
        _shaders.erase(type);
    }

    /// Clear all light shaders previously bound.
    void clear()
    {
        _shaders.clear();
    }

    /// Return the light shader bound to the given light type,
    /// or nullptr if not light shader is bound to this type.
    const ShaderNode* get(unsigned int type) const
    {
        auto it = _shaders.find(type);
        return it != _shaders.end() ? it->second.get() : nullptr;
    }

    /// Return the map of bound light shaders.
    const std::unordered_map<unsigned int, ShaderNodePtr>& get() const
    {
        return _shaders;
    }

  protected:
    std::unordered_map<unsigned int, ShaderNodePtr> _shaders;
};

MATERIALX_NAMESPACE_END

#endif
