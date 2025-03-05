//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/Nodes/HwBlurNode.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/ShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr HwBlurNode::create(const string& samplingIncludeFilename)
{
    return std::make_shared<HwBlurNode>(samplingIncludeFilename);
}

void HwBlurNode::emitSamplingFunctionDefinition(const ShaderNode& /*node*/, GenContext& context, ShaderStage& stage) const
{
    const ShaderGenerator& shadergen = context.getShaderGenerator();
    shadergen.emitLibraryInclude(_samplingIncludeFilename, context, stage);
    shadergen.emitLineBreak(stage);
}

MATERIALX_NAMESPACE_END
