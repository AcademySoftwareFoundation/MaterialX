#include <MaterialXGenShader/Nodes/HwSourceCodeNode.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/Shader.h>

#include <MaterialXCore/Library.h>
#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Document.h>

#include <cstdarg>

namespace MaterialX
{

ShaderNodeImplPtr HwSourceCodeNode::create()
{
    return std::make_shared<HwSourceCodeNode>();
}

void HwSourceCodeNode::emitFunctionCall(ShaderStage& stage, const ShaderNode& node, const ShaderGenerator& shadergen, GenContext& context) const
{
BEGIN_SHADER_STAGE(stage, MAIN_STAGE)
    if (_inlined)
    {
        SourceCodeNode::emitFunctionCall(stage, node, shadergen, context);
    }
    else
    {
        // Declare the output variables
        for (size_t i = 0; i < node.numOutputs(); ++i)
        {
            shadergen.emitLineBegin(stage);
            shadergen.emitOutput(stage, context, node.getOutput(i), true, true);
            shadergen.emitLineEnd(stage);
        }

        shadergen.emitLineBegin(stage);
        string delim = "";

        // Check if we have a closure context to modify the function call.
        HwClosureContextPtr ccx = context.getUserData<HwClosureContext>(HW::USER_DATA_CLOSURE_CONTEXT);

        if (ccx)
        {
            // Emit function name.
            shadergen.emitString(stage, _functionName + ccx->getSuffix() + "(");

            // Emit extra argument.
            for (const HwClosureContext::Argument& arg : ccx->getArguments())
            {
                shadergen.emitString(stage, delim + arg.second);
                delim = ", ";
            }
        }
        else
        {
            // Emit function name.
            shadergen.emitString(stage, _functionName + "(");
        }

        // Emit all inputs.
        for (ShaderInput* input : node.getInputs())
        {
            shadergen.emitString(stage, delim);
            shadergen.emitInput(stage, context, input);
            delim = ", ";
        }

        // Emit all outputs.
        for (size_t i = 0; i < node.numOutputs(); ++i)
        {
            shadergen.emitString(stage, delim);
            shadergen.emitOutput(stage, context, node.getOutput(i), false, false);
            delim = ", ";
        }

        // End function call
        shadergen.emitString(stage, ")");
        shadergen.emitLineEnd(stage);
    }
END_SHADER_STAGE(stage, MAIN_STAGE)
}

} // namespace MaterialX
