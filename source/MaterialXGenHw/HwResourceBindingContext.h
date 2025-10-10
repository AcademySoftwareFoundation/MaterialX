//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HWRESOURCEBINDINGCONTEXT_H
#define MATERIALX_HWRESOURCEBINDINGCONTEXT_H

/// @file
/// Hardware resource bindings context base class

#include <MaterialXGenHw/Export.h>

#include <MaterialXGenShader/GenUserData.h>

#include <MaterialXCore/Util.h>

MATERIALX_NAMESPACE_BEGIN

class VariableBlock;

class HwResourceBindingContext;
/// Shared pointer to a HwResourceBindingContext
using HwResourceBindingContextPtr = shared_ptr<class HwResourceBindingContext>;

/// @class HwResourceBindingContext
/// Class representing a context for resource binding for hardware resources.
class MX_GENHW_API HwResourceBindingContext : public GenUserData
{
  public:
    virtual ~HwResourceBindingContext() { }

    // Initialize the context before generation starts.
    virtual void initialize() = 0;

    // Emit directives required for binding support
    virtual void emitDirectives(GenContext& context, ShaderStage& stage) = 0;

    // Emit uniforms with binding information
    virtual void emitResourceBindings(GenContext& context, const VariableBlock& uniforms, ShaderStage& stage) = 0;

    // Emit struct uniforms with binding information
    virtual void emitStructuredResourceBindings(GenContext& context, const VariableBlock& uniforms,
                                                ShaderStage& stage, const std::string& structInstanceName,
                                                const std::string& arraySuffix = EMPTY_STRING) = 0;
};

MATERIALX_NAMESPACE_END

#endif
