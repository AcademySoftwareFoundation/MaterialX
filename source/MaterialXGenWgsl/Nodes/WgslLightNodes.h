//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_WGSLLIGHTNODES_H
#define MATERIALX_WGSLLIGHTNODES_H

#include <MaterialXGenWgsl/Export.h>

#include <MaterialXGenHw/Nodes/HwLightSamplerNode.h>
#include <MaterialXGenHw/Nodes/HwLightCompoundNode.h>
#include <MaterialXGenHw/Nodes/HwNumLightsNode.h>

MATERIALX_NAMESPACE_BEGIN

/// Light sampler node emitting the WGSL signature:
/// fn sampleLightSource(light: LightData, position: vec3f, result: ptr<function, lightshader>).
class MX_GENWGSL_API WgslLightSamplerNode : public HwLightSamplerNode
{
  public:
    WgslLightSamplerNode();
    static ShaderNodeImplPtr create();
    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

/// Light compound node emitting a WGSL light function definition and call.
class MX_GENWGSL_API WgslLightCompoundNode : public HwLightCompoundNode
{
  public:
    static ShaderNodeImplPtr create();
    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

/// Num-lights node emitting fn numActiveLightSources() -> i32.
class MX_GENWGSL_API WgslNumLightsNode : public HwNumLightsNode
{
  public:
    WgslNumLightsNode();
    static ShaderNodeImplPtr create();
    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
