//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenWgsl/Nodes/WgslSurfaceNode.h>

#include <MaterialXGenHw/HwConstants.h>
#include <MaterialXGenHw/HwShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr WgslSurfaceNode::create()
{
    return std::make_shared<WgslSurfaceNode>();
}

void WgslSurfaceNode::emitShadowOcclusionCall(const ShaderNode& /*node*/, GenContext& context, ShaderStage& stage, const string& vertexPrefix) const
{
    const HwShaderGenerator& shadergen = static_cast<const HwShaderGenerator&>(context.getShaderGenerator());
    shadergen.emitLine("occlusion = mx_shadow_occlusion(" + HW::SHADOW_MAP + "_texture, " + HW::SHADOW_MAP + "_sampler, "
                       + HW::T_SHADOW_MATRIX + ", " + vertexPrefix + HW::T_POSITION_WORLD + ")", stage);
}

void WgslSurfaceNode::emitAmbientOcclusionCall(GenContext& context, ShaderStage& stage, const string& ambOccUv) const
{
    const HwShaderGenerator& shadergen = static_cast<const HwShaderGenerator&>(context.getShaderGenerator());
    shadergen.emitLine("occlusion = mix(1.0, textureSample(" + HW::AMB_OCC_MAP + "_texture, " + HW::AMB_OCC_MAP + "_sampler, "
                       + ambOccUv + ").x, " + HW::T_AMB_OCC_GAIN + ")", stage);
}

MATERIALX_NAMESPACE_END
