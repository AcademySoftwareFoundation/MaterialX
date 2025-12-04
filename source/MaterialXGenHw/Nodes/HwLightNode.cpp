//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenHw/Nodes/HwLightNode.h>

#include <MaterialXGenHw/HwConstants.h>
#include <MaterialXGenHw/HwShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>

MATERIALX_NAMESPACE_BEGIN

HwLightNode::HwLightNode()
{
}

ShaderNodeImplPtr HwLightNode::create()
{
    return std::make_shared<HwLightNode>();
}

void HwLightNode::createVariables(const ShaderNode&, GenContext& context, Shader& shader) const
{
    ShaderStage& ps = shader.getStage(Stage::PIXEL);

    // Create uniform for intensity, exposure and direction
    VariableBlock& lightUniforms = ps.getUniformBlock(HW::LIGHT_DATA);
    lightUniforms.add(Type::FLOAT, "intensity", Value::createValue<float>(1.0f));
    lightUniforms.add(Type::FLOAT, "exposure", Value::createValue<float>(0.0f));
    lightUniforms.add(Type::VECTOR3, "direction", Value::createValue<Vector3>(Vector3(0.0f, 1.0f, 0.0f)));

    const HwShaderGenerator& shadergen = static_cast<const HwShaderGenerator&>(context.getShaderGenerator());
    shadergen.addStageLightingUniforms(context, ps);
}

void HwLightNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const HwShaderGenerator& shadergen = static_cast<const HwShaderGenerator&>(context.getShaderGenerator());
        const Syntax& syntax = shadergen.getSyntax();

        const string& vec3 = syntax.getTypeName(Type::VECTOR3);
        const string vec3_zero = syntax.getValue(Type::VECTOR3, HW::VEC3_ZERO);

        shadergen.emitLine(vec3+" L = light.position - position", stage);
        shadergen.emitLine("float distance = length(L)", stage);
        shadergen.emitLine("L /= distance", stage);
        shadergen.emitLine("result.direction = L", stage);
        shadergen.emitLineBreak(stage);

        const ShaderInput* edfInput = node.getInput("edf");
        const ShaderNode* edf = edfInput->getConnectedSibling();
        if (edf)
        {

            shadergen.emitScopeBegin(stage);
            shadergen.emitLine("ClosureData closureData = makeClosureData(CLOSURE_TYPE_EMISSION, "+vec3_zero+", -L, light.direction, "+vec3_zero+", 0)", stage);
            shadergen.emitFunctionCall(*edf, context, stage);
            shadergen.emitScopeEnd(stage);
            shadergen.emitLineBreak(stage);

            shadergen.emitComment("Apply quadratic falloff and adjust intensity", stage);
            shadergen.emitLine("result.intensity = " + edf->getOutput()->getVariable() + " / (distance * distance)", stage);

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
        }
        else
        {
            shadergen.emitLine("result.intensity = "+vec3_zero, stage);
        }
    }
}

MATERIALX_NAMESPACE_END
