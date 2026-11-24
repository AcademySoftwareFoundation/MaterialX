//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenWgsl/Nodes/WgslTransformNormalNode.h>

#include <MaterialXGenShader/GenContext.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr WgslTransformNormalNode::create()
{
    return std::make_shared<WgslTransformNormalNode>();
}

void WgslTransformNormalNode::emitNormalizeCall(const ShaderOutput* output, GenContext& context, ShaderStage& stage) const
{
    const ShaderGenerator& shadergen = context.getShaderGenerator();
    shadergen.emitLine(output->getVariable() + " = normalize(" + output->getVariable() + ")", stage);
}

MATERIALX_NAMESPACE_END
