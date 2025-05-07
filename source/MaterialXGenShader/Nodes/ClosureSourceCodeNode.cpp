//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/Nodes/ClosureSourceCodeNode.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr ClosureSourceCodeNode::create()
{
    return std::make_shared<ClosureSourceCodeNode>();
}

void ClosureSourceCodeNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const ShaderGenerator& shadergen = context.getShaderGenerator();

        // Emit calls for any closure dependencies upstream from this node.
        shadergen.emitDependentFunctionCalls(node, context, stage, ShaderNode::Classification::CLOSURE);

        if (_inlined)
        {
            SourceCodeNode::emitFunctionCall(node, context, stage);
        }
        else
        {
            string delim = "";

            // Declare the output variable.
            emitOutputVariables(node, context, stage);

            // Emit function name.
            shadergen.emitLineBegin(stage);
            shadergen.emitString(_functionName + "(", stage);

            if (context.getShaderGenerator().nodeNeedsClosureData(node))
            {
                shadergen.emitString(delim + HW::CLOSURE_DATA_ARG + ", ", stage);
            }

            // Emit all inputs.
            for (ShaderInput* input : node.getInputs())
            {
                shadergen.emitString(delim, stage);
                shadergen.emitInput(input, context, stage);
                delim = ", ";
            }

            // Emit the output.
            shadergen.emitString(delim + node.getOutput()->getVariable() + ")", stage);

            // End function call
            shadergen.emitLineEnd(stage);
        }
    }
}

MATERIALX_NAMESPACE_END
