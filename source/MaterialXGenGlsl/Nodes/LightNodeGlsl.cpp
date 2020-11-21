//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/Nodes/LightNodeGlsl.h>

#include <MaterialXGenShader/Shader.h>

namespace MaterialX
{

namespace
{
    const string LIGHT_DIRECTION_CALCULATION =
        "vec3 L = light.position - position;\n"
        "float distance = length(L);\n"
        "L /= distance;\n"
        "result.direction = L;\n";
}

LightNodeGlsl::LightNodeGlsl()
{
    // Emission context
    _callEmission = HwClosureContext::create(HwClosureContext::EMISSION);
    _callEmission->addArgument(Type::EDF, HwClosureContext::Argument(Type::VECTOR3, "light.direction"));
    _callEmission->addArgument(Type::EDF, HwClosureContext::Argument(Type::VECTOR3, "-L"));
}

ShaderNodeImplPtr LightNodeGlsl::create()
{
    return std::make_shared<LightNodeGlsl>();
}

void LightNodeGlsl::createVariables(const ShaderNode&, GenContext& context, Shader& shader) const
{
    ShaderStage& ps = shader.getStage(Stage::PIXEL);

    // Create uniform for intensity, exposure and direction
    VariableBlock& lightUniforms = ps.getUniformBlock(HW::LIGHT_DATA);
    lightUniforms.add(Type::FLOAT, "intensity", Value::createValue<float>(1.0f));
    lightUniforms.add(Type::FLOAT, "exposure", Value::createValue<float>(0.0f));
    lightUniforms.add(Type::VECTOR3, "direction", Value::createValue<Vector3>(Vector3(0.0f,1.0f,0.0f)));

    const GlslShaderGenerator& shadergen = static_cast<const GlslShaderGenerator&>(context.getShaderGenerator());
    shadergen.addStageLightingUniforms(context, ps);
}

void LightNodeGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        const GlslShaderGenerator& shadergen = static_cast<const GlslShaderGenerator&>(context.getShaderGenerator());
        const ShaderGraph& graph = *node.getParent();

        shadergen.emitBlock(LIGHT_DIRECTION_CALCULATION, context, stage);
        shadergen.emitLineBreak(stage);

        string emission;
        shadergen.emitEdfNodes(graph, node, _callEmission, context, stage, emission);
        shadergen.emitLineBreak(stage);

        shadergen.emitComment("Apply quadratic falloff and adjust intensity", stage);
        shadergen.emitLine("result.intensity = " + emission + " / (distance * distance)", stage);

        const ShaderInput* intensity = node.getInput("intensity");
        const ShaderInput* exposure = node.getInput("exposure");

        shadergen.emitLineBegin(stage);
        shadergen.emitString("result.intensity *= ", stage);
        shadergen.emitInput(intensity, context, stage);
        shadergen.emitLineEnd(stage);

        // Emit exposure adjustment only if it matters
        if (exposure->getConnection() || (exposure->getValue() && exposure->getValue()->asA<float>() != 0.0f))
        {
            shadergen.emitLineBegin(stage);
            shadergen.emitString("result.intensity *= pow(2, ", stage);
            shadergen.emitInput(exposure, context, stage);
            shadergen.emitString(")", stage);
            shadergen.emitLineEnd(stage);
        }
    END_SHADER_STAGE(shader, Stage::PIXEL)
}

} // namespace MaterialX
