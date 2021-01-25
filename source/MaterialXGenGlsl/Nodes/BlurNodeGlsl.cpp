//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/Nodes/BlurNodeGlsl.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

ShaderNodeImplPtr BlurNodeGlsl::create()
{
    return std::shared_ptr<BlurNodeGlsl>(new BlurNodeGlsl());
}

void BlurNodeGlsl::emitSamplingFunctionDefinition(const ShaderNode& /*node*/, GenContext& context, ShaderStage& stage) const
{
    const ShaderGenerator& shadergen = context.getShaderGenerator();
    shadergen.emitInclude("stdlib/genglsl/lib/mx_sampling" + shadergen.getSyntax().getSourceFileExtension(), context, stage);
    shadergen.emitLineBreak(stage);
}

} // namespace MaterialX
