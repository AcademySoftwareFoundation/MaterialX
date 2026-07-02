//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_WGSLRESOURCEBINDING_H
#define MATERIALX_WGSLRESOURCEBINDING_H

/// @file
/// WebGPU WGSL resource binding context

#include <MaterialXGenGlsl/Export.h>

#include <MaterialXGenGlsl/vk/VkResourceBindingContext.h>

#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/Syntax.h>

MATERIALX_NAMESPACE_BEGIN

/// Shared pointer to a WgslResourceBindingContext
using WgslResourceBindingContextPtr = shared_ptr<class WgslResourceBindingContext>;

/// @class WgslResourceBindingContext
/// Class representing a resource binding for WGSL shader resources.
class MX_GENGLSL_API WgslResourceBindingContext : public VkResourceBindingContext
{
  public:
    WgslResourceBindingContext(size_t uniformBindingLocation);

    static WgslResourceBindingContextPtr create(size_t uniformBindingLocation = 0)
    {
        return std::make_shared<WgslResourceBindingContext>(uniformBindingLocation);
    }

    // Emit uniforms with binding information
    void emitResourceBindings(GenContext& context, const VariableBlock& uniforms, ShaderStage& stage) override;

    // Emit structured uniforms with binding information
    void emitStructuredResourceBindings(GenContext& context, const VariableBlock& uniforms,
                                        ShaderStage& stage, const std::string& structInstanceName,
                                        const std::string& arraySuffix) override;

    // Current @binding index (advanced by emitResourceBindings)
    size_t getBindingLocation() const { return _hwUniformBindLocation; }

    void setBindingLocation(size_t location) { _hwUniformBindLocation = location; }

    // Emit a WGSL type name for a uniform port (maps bool to i32)
    string getWgslUniformType(const ShaderPort* port, const Syntax& syntax) const;
};

MATERIALX_NAMESPACE_END

#endif
