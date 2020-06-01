//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenMdl/Nodes/SurfaceNodeMdl.h>
#include <MaterialXGenMdl/MdlShaderGenerator.h>

#include <MaterialXGenShader/GenContext.h>

namespace MaterialX
{

SurfaceNodeMdl::SurfaceNodeMdl()
{
}

ShaderNodeImplPtr SurfaceNodeMdl::create()
{
    return std::make_shared<SurfaceNodeMdl>();
}

void SurfaceNodeMdl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        const MdlShaderGenerator& shadergen = static_cast<const MdlShaderGenerator&>(context.getShaderGenerator());
        const ShaderGraph& graph = *node.getParent();

        const ShaderInput* ior = nullptr;
        for (const ShaderNode* candidate : graph.getNodes())
        {
            if (candidate->hasClassification(ShaderNode::Classification::BSDF_T) && node.isUsedClosure(candidate))
            {
                ior = candidate->getInput("ior");
                if (ior)
                {
                    break;
                }
            }
        }

        shadergen.emitLineBegin(stage);

        // Emit the output and funtion name.
        shadergen.emitOutput(node.getOutput(0), true, false, context, stage);
        shadergen.emitString(" = mx::pbrlib::mx_surface(", stage);

        // Emit all inputs on the node.
        string delim = "";
        for (ShaderInput* input : node.getInputs())
        {
            shadergen.emitString(delim, stage);
            shadergen.emitInput(input, context, stage);
            delim = ", ";
        }

        // Emit the ior input.
        if (ior)
        {
            shadergen.emitString(delim, stage);
            shadergen.emitInput(ior, context, stage);
        }

        // End function call
        shadergen.emitString(")", stage);
        shadergen.emitLineEnd(stage);

    END_SHADER_STAGE(stage, Stage::PIXEL)
}

} // namespace MaterialX
