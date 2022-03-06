//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/Nodes/UnlitSurfaceNodeGlsl.h>

#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/GenContext.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr UnlitSurfaceNodeGlsl::create()
{
    return std::make_shared<UnlitSurfaceNodeGlsl>();
}

void UnlitSurfaceNodeGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const GlslShaderGenerator& shadergen = static_cast<const GlslShaderGenerator&>(context.getShaderGenerator());

    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)

        // Declare the output variable
        const ShaderOutput* output = node.getOutput();
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(output, true, true, context, stage);
        shadergen.emitLineEnd(stage);

        const string outColor = output->getVariable() + ".color";
        const string outTransparency = output->getVariable() + ".transparency";

        if (context.getOptions().hwTransparency)
        {
            shadergen.emitLineBegin(stage);
            shadergen.emitString("float surfaceOpacity = ", stage);
            shadergen.emitInput(node.getInput("opacity"), context, stage);
            shadergen.emitLineEnd(stage);
            shadergen.emitLineBreak(stage);
        }

        const ShaderInput* emission = node.getInput("emission");
        const ShaderInput* emissionColor = node.getInput("emission_color");
        shadergen.emitLine(outColor + " = " + shadergen.getUpstreamResult(emission, context) + " * " + shadergen.getUpstreamResult(emissionColor, context), stage);

        if (context.getOptions().hwTransparency)
        {
            const ShaderInput* transmission = node.getInput("transmission");
            const ShaderInput* transmissionColor = node.getInput("transmission_color");
            shadergen.emitLine(outTransparency + " = " + shadergen.getUpstreamResult(transmission, context) + " * " + shadergen.getUpstreamResult(transmissionColor, context), stage);

            shadergen.emitComment("Mix in opacity which affect the total result", stage);
            shadergen.emitLine(outColor + " *= surfaceOpacity", stage);
            shadergen.emitLine(outTransparency + " = mix(vec3(1.0), " + outTransparency + ", surfaceOpacity)", stage);
        }
        else
        {
            shadergen.emitLine(outTransparency + " = vec3(0.0)", stage);
        }

    END_SHADER_STAGE(stage, Stage::PIXEL)
}

MATERIALX_NAMESPACE_END
