//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/Nodes/SourceCodeClosureNodeGlsl.h>
#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

ShaderNodeImplPtr SourceCodeClosureNodeGlsl::create()
{
    return std::make_shared<SourceCodeClosureNodeGlsl>();
}

void SourceCodeClosureNodeGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
    const GlslShaderGenerator& shadergen = static_cast<const GlslShaderGenerator&>(context.getShaderGenerator());

    // Emit calls for any closure dependencies upstream from this node.
    shadergen.emitDependentNodes(node, context, stage, ShaderNode::Classification::CLOSURE);

    if (_inlined)
    {
        SourceCodeNode::emitFunctionCall(node, context, stage);
    }
    else
    {
        const ShaderOutput* output = node.getOutput();
        string delim = "";

        // Declare the output variable.
        emitOutputVariables(node, context, stage);

        // Check if we have a closure context to modify the function call.
        HwClosureContextPtr ccx = context.getUserData<HwClosureContext>(HW::USER_DATA_CLOSURE_CONTEXT);
        if (ccx)
        {
            // Check if thin-film has been added to the BSDF context.
            const TypeDesc* closureType = output->getType();
            const ShaderNode* thinfilm = ccx->getThinFilm();
            if (closureType == Type::BSDF && thinfilm)
            {
                // Set thin-film parameters on the BSDF.
                const ShaderInput* thickness = thinfilm->getInput("thickness");
                const ShaderInput* ior = thinfilm->getInput("ior");
                if (!(thickness && ior))
                {
                    throw ExceptionShaderGenError("Node '" + thinfilm->getName() + "' is not a valid thin_film_bsdf node");
                }
                shadergen.emitLine(output->getVariable() + ".tf_thickness = " + shadergen.getUpstreamResult(thickness, context), stage);
                shadergen.emitLine(output->getVariable() + ".tf_ior = " + shadergen.getUpstreamResult(ior, context), stage);

                // Once the thinfilm has been applied we reset it on the context
                // since we don't want any upstream BSDF to also pick this up.
                ccx->setThinFilm(nullptr);
            }

            // Emit function name.
            shadergen.emitLineBegin(stage);
            shadergen.emitString(_functionName + ccx->getSuffix(closureType) + "(", stage);

            // Emit extra argument.
            for (const HwClosureContext::Argument& arg : ccx->getArguments(closureType))
            {
                shadergen.emitString(delim + arg.second, stage);
                delim = ", ";
            }
        }
        else
        {
            // Emit function name.
            shadergen.emitLineBegin(stage);
            shadergen.emitString(_functionName + "(", stage);
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
END_SHADER_STAGE(stage, Stage::PIXEL)
}

} // namespace MaterialX
