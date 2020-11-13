//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/Nodes/HwSourceCodeNode.h>
#include <MaterialXGenShader/HwShaderGenerator.h>

namespace MaterialX
{

ShaderNodeImplPtr HwSourceCodeNode::create()
{
    return std::make_shared<HwSourceCodeNode>();
}

void HwSourceCodeNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        if (_inlined)
        {
            SourceCodeNode::emitFunctionCall(node, context, stage);
        }
        else
        {
            const ShaderGenerator& shadergen = context.getShaderGenerator();

            // Declare the output variables
            for (size_t i = 0; i < node.numOutputs(); ++i)
            {
                shadergen.emitLineBegin(stage);
                shadergen.emitOutput(node.getOutput(i), true, true, context, stage);
                shadergen.emitLineEnd(stage);
            }

            shadergen.emitLineBegin(stage);
            string delim = "";

            // Check if we have a closure context to modify the function call.
            HwClosureContextPtr ccx = context.getUserData<HwClosureContext>(HW::USER_DATA_CLOSURE_CONTEXT);

            if (ccx)
            {
                // Use the first output for classifying node type for the closure context.
                // This is only relevent for closures, and they only have a single output.
                const TypeDesc* nodeType = node.getOutput()->getType();

                // Emit function name.
                shadergen.emitString(_functionName + ccx->getSuffix(nodeType) + "(", stage);

                // Emit extra argument.
                for (const HwClosureContext::Argument& arg : ccx->getArguments(nodeType))
                {
                    shadergen.emitString(delim + arg.second, stage);
                    delim = ", ";
                }
            }
            else
            {
                // Emit function name.
                shadergen.emitString(_functionName + "(", stage);
            }

            // Emit all inputs.
            for (ShaderInput* input : node.getInputs())
            {
                shadergen.emitString(delim, stage);
                shadergen.emitInput(input, context, stage);
                delim = ", ";
            }

            // Emit all outputs.
            for (size_t i = 0; i < node.numOutputs(); ++i)
            {
                shadergen.emitString(delim, stage);
                shadergen.emitOutput(node.getOutput(i), false, false, context, stage);
                delim = ", ";
            }

            // End function call
            shadergen.emitString(")", stage);
            shadergen.emitLineEnd(stage);
        }
    END_SHADER_STAGE(stage, Stage::PIXEL)
}

} // namespace MaterialX
