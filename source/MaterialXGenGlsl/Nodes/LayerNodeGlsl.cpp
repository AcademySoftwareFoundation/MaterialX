//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/Nodes/LayerNodeGlsl.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/TypeDesc.h>

namespace MaterialX
{

const string LayerNodeGlsl::TOP = "top";
const string LayerNodeGlsl::BASE = "base";

ShaderNodeImplPtr LayerNodeGlsl::create()
{
    return std::make_shared<LayerNodeGlsl>();
}

void LayerNodeGlsl::emitFunctionCall(const ShaderNode& _node, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        const ShaderGenerator& shadergen = context.getShaderGenerator();

        ShaderNode& node = const_cast<ShaderNode&>(_node);

        ShaderInput* top = node.getInput(TOP);
        ShaderInput* base = node.getInput(BASE);
        ShaderOutput* output = node.getOutput();
        if (!(top && base && output))
        {
            throw ExceptionShaderGenError("Node '" + node.getName() + "' is not a valid layer node");
        }

        // Declare the output variable.
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(output, true, true, context, stage);
        shadergen.emitLineEnd(stage);

        if (!(top->getConnection() && base->getConnection()))
        {
            return;
        }



        const string& topResult = top->getConnection()->getVariable();
        const string& baseResult = base->getConnection()->getVariable();

        shadergen.emitLine(output->getVariable() + ".eval = " + topResult + ".eval + " + topResult + ".throughput * " + baseResult + ".eval", stage);
        shadergen.emitLine(output->getVariable() + ".throughput = " + topResult + ".throughput * " + baseResult + ".throughput", stage);

    END_SHADER_STAGE(stage, Stage::PIXEL)
}

} // namespace MaterialX
