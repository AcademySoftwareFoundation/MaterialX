//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/Nodes/ClosureLayerNodeGlsl.h>
#include <MaterialXGenGlsl/GlslShaderGenerator.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/TypeDesc.h>

namespace MaterialX
{

const string ClosureLayerNodeGlsl::TOP = "top";
const string ClosureLayerNodeGlsl::BASE = "base";

ShaderNodeImplPtr ClosureLayerNodeGlsl::create()
{
    return std::make_shared<ClosureLayerNodeGlsl>();
}

void ClosureLayerNodeGlsl::emitFunctionCall(const ShaderNode& _node, GenContext& context, ShaderStage& stage) const
{
BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
    const ShaderGenerator& shadergen = context.getShaderGenerator();

    ShaderNode& node = const_cast<ShaderNode&>(_node);

    ShaderInput* top = node.getInput(TOP);
    ShaderInput* base = node.getInput(BASE);
    ShaderOutput* output = node.getOutput();

    // Make sure the layer is fully connected.
    if (!(top->getConnection() && base->getConnection()))
    {
        // Just declare the output variable with default value.
        emitOutputVariables(node, context, stage);
        return;
    }

    ShaderNode* topBsdf = top->getConnection()->getNode();
    ShaderNode* baseBsdf = base->getConnection()->getNode();

    if (topBsdf->hasClassification(ShaderNode::Classification::THINFILM))
    {
        // This is a layer node with thin-film as top layer.
        // Call only the base BSDF but with thin-film parameters set.
        ClosureContext* cct = context.getClosureContext();
        if (!cct)
        {
            throw ExceptionShaderGenError("No closure context set to evaluate node '" + node.getName() + "'");
        }
        cct->setThinFilm(topBsdf);
        const string oldVariable = baseBsdf->getOutput()->getVariable();
        baseBsdf->getOutput()->setVariable(output->getVariable());
        shadergen.emitFunctionCall(*baseBsdf, context, stage);
        baseBsdf->getOutput()->setVariable(oldVariable);
    }
    else
    {
        // Evaluate top BSDF and base BSDF and combine their result
        // according to throughput from the top BSDF.
        //
        // TODO: Should we emit code to check the top throughput before
        //       calling the base BSDF? When the throughput is zero we could
        //       early out.
        //
        shadergen.emitComment("Evaluate top BSDF for " + node.getName(), stage);
        shadergen.emitFunctionCall(*topBsdf, context, stage);
        shadergen.emitComment("Evaluate base BSDF for " + node.getName(), stage);
        shadergen.emitFunctionCall(*baseBsdf, context, stage);

        // Declare the output variable.
        emitOutputVariables(node, context, stage);

        // Calculate the layering result.
        const string& topResult = top->getConnection()->getVariable();
        const string& baseResult = base->getConnection()->getVariable();
        shadergen.emitLine(output->getVariable() + ".result = " + topResult + ".result + " + topResult + ".throughput * " + baseResult + ".result", stage);
        shadergen.emitLine(output->getVariable() + ".throughput = " + topResult + ".throughput * " + baseResult + ".throughput", stage);
    }
END_SHADER_STAGE(stage, Stage::PIXEL)
}

} // namespace MaterialX
