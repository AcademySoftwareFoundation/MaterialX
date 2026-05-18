//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenGlsl/Nodes/DisplacementNodeGlsl.h>

#include <MaterialXGenHw/HwConstants.h>
#include <MaterialXGenHw/HwShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr DisplacementNodeGlsl::create()
{
    return std::make_shared<DisplacementNodeGlsl>();
}

void DisplacementNodeGlsl::createVariables(const ShaderNode&, GenContext&, Shader& shader) const
{
    ShaderStage& vs = shader.getStage(Stage::VERTEX);
    ShaderStage& ps = shader.getStage(Stage::PIXEL);

    // Add a simple float marker to signal displacement to the pixel stage.
    // We use a float instead of the displacementshader struct because
    // ESSL 300 (WebGL 2) does not support struct varyings.
    addStageConnector(HW::VERTEX_DATA, Type::FLOAT, HW::T_DISPLACEMENT_ACTIVE, vs, ps);
}

void DisplacementNodeGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const HwShaderGenerator& shadergen = static_cast<const HwShaderGenerator&>(context.getShaderGenerator());

    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        // Only emit displacement in vertex stage when actively evaluating
        // displacement dependencies. Default displacement nodes from
        // surfacematerial's unconnected inputs should be skipped.
        if (!context.getEmitVertexDisplacement())
            return;

        // Emit all dependent nodes first (fractal3d, position, multiply, etc.)
        // so their outputs are available for the displacement calculation.
        shadergen.emitDependentFunctionCalls(node, context, stage);

        // Construct the displacementshader struct from inputs.
        const ShaderOutput* output = node.getOutput();
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(output, true, false, context, stage);
        shadergen.emitString(" = displacementshader(", stage);

        // Displacement input (float → vec3 conversion, or vec3 directly)
        const ShaderInput* dispInput = node.getInput("displacement");
        if (dispInput)
        {
            if (dispInput->getType() == Type::FLOAT)
            {
                // Float displacement along normal direction (Z in tangent space)
                shadergen.emitString("vec3(0.0, 0.0, ", stage);
                shadergen.emitInput(dispInput, context, stage);
                shadergen.emitString(")", stage);
            }
            else
            {
                shadergen.emitInput(dispInput, context, stage);
            }
        }
        else
        {
            shadergen.emitString("vec3(0.0)", stage);
        }

        shadergen.emitString(", ", stage);

        // Scale input
        const ShaderInput* scaleInput = node.getInput("scale");
        if (scaleInput)
        {
            shadergen.emitInput(scaleInput, context, stage);
        }
        else
        {
            shadergen.emitString("1.0", stage);
        }

        shadergen.emitString(")", stage);
        shadergen.emitLineEnd(stage);

        // Signal displacement to the pixel stage via a float marker varying.
        // We use a float instead of the displacementshader struct because
        // ESSL 300 (WebGL 2) does not support struct varyings.
        VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        const string prefix = shadergen.getVertexDataPrefix(vertexData);
        ShaderPort* marker = vertexData[HW::T_DISPLACEMENT_ACTIVE];
        if (marker && !marker->isEmitted())
        {
            marker->setEmitted();
            shadergen.emitLine(prefix + marker->getVariable() + " = 1.0", stage);
        }
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        // Displacement is applied in the vertex stage, but the pixel stage
        // still needs the output variable declared for compound function
        // definitions that reference all outputs.
        const ShaderOutput* output = node.getOutput();
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(output, true, false, context, stage);
        shadergen.emitString(" = displacementshader(vec3(0.0), 1.0)", stage);
        shadergen.emitLineEnd(stage);
    }
}

MATERIALX_NAMESPACE_END
