//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/Nodes/ClosureMixNodeGlsl.h>
#include <MaterialXGenGlsl/GlslShaderGenerator.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/TypeDesc.h>

namespace MaterialX
{

const string ClosureMixNodeGlsl::FG = "fg";
const string ClosureMixNodeGlsl::BG = "bg";
const string ClosureMixNodeGlsl::MIX = "mix";

ShaderNodeImplPtr ClosureMixNodeGlsl::create()
{
    return std::make_shared<ClosureMixNodeGlsl>();
}

void ClosureMixNodeGlsl::emitFunctionCall(const ShaderNode& _node, GenContext& context, ShaderStage& stage) const
{
BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
    const ShaderGenerator& shadergen = context.getShaderGenerator();
    ClosureContext* cct = context.getClosureContext();

    ShaderNode& node = const_cast<ShaderNode&>(_node);

    ShaderInput* fg = node.getInput(FG);
    ShaderInput* bg = node.getInput(BG);
    ShaderInput* mix = node.getInput(MIX);

    // If the add node has closure parameters set,
    // we pass this on to both components.

    if (fg->getConnection())
    {
        // Make sure it's a connection to a sibling and not the graph interface.
        ShaderNode* fgNode = fg->getConnection()->getNode();
        if (fgNode->getParent() == node.getParent())
        {
            ScopedAssignClosureParams assign(&node, fgNode, cct);
            shadergen.emitFunctionCall(*fgNode, context, stage);
        }
    }
    if (bg->getConnection())
    {
        // Make sure it's a connection to a sibling and not the graph interface.
        ShaderNode* bgNode = bg->getConnection()->getNode();
        if (bgNode->getParent() == node.getParent())
        {
            ScopedAssignClosureParams assign(&node, bgNode, cct);
            shadergen.emitFunctionCall(*bgNode, context, stage);
        }
    }

    const string fgResult = shadergen.getUpstreamResult(fg, context);
    const string bgResult = shadergen.getUpstreamResult(bg, context);
    const string mixResult = shadergen.getUpstreamResult(mix, context);

    ShaderOutput* output = node.getOutput();
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
