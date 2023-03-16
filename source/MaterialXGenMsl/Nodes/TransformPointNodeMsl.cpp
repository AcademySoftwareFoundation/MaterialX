//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenMsl/Nodes/TransformPointNodeMsl.h>

#include <MaterialXGenShader/Shader.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr TransformPointNodeMsl::create()
{
    return std::make_shared<TransformPointNodeMsl>();
}

string TransformPointNodeMsl::getHomogeneousCoordinate(const ShaderInput* in, GenContext& context) const
{
    const ShaderGenerator& shadergen = context.getShaderGenerator();
    return "float4(" + shadergen.getUpstreamResult(in, context) + ", 1.0)";
}

MATERIALX_NAMESPACE_END
