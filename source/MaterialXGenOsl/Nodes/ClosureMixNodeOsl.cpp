//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenOsl/Nodes/ClosureMixNodeOsl.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/TypeDesc.h>

namespace MaterialX
{

const string ClosureMixNodeOsl::FG = "fg";
const string ClosureMixNodeOsl::BG = "bg";
const string ClosureMixNodeOsl::MIX = "mix";

ShaderNodeImplPtr ClosureMixNodeOsl::create()
{
    return std::make_shared<ClosureMixNodeOsl>();
}

void ClosureMixNodeOsl::emitFunctionCall(const ShaderNode& _node, GenContext& context, ShaderStage& stage) const
{
BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
    const ShaderGenerator& shadergen = context.getShaderGenerator();

    ShaderNode& node = const_cast<ShaderNode&>(_node);

    // Emit calls for the closure dependencies upstream from this node.
    shadergen.emitDependentFunctionCalls(node, context, stage, ShaderNode::Classification::CLOSURE);

    ShaderOutput* output = node.getOutput();
    const string fgResult = shadergen.getUpstreamResult(node.getInput(FG), context);
    const string bgResult = shadergen.getUpstreamResult(node.getInput(BG), context);
    const string mixResult = shadergen.getUpstreamResult(node.getInput(MIX), context);

    if (output->getType() == Type::BSDF)
    {
        emitOutputVariables(node, context, stage);
        shadergen.emitLine(output->getVariable() + ".result = mix(" + bgResult + ".result, " + fgResult + ".result, " + mixResult + ")", stage);
        shadergen.emitLine(output->getVariable() + ".throughput = mix(" + bgResult + ".throughput, " + fgResult + ".throughput, " + mixResult + ")", stage);
    }
    else if (output->getType() == Type::EDF)
    {
        shadergen.emitLine(shadergen.getSyntax().getTypeName(Type::EDF) + " " + output->getVariable() + " = mix(" + bgResult + ", " + fgResult + ", " + mixResult + ")", stage);
    }
END_SHADER_STAGE(stage, Stage::PIXEL)
}

} // namespace MaterialX
