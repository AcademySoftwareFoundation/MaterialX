//
// TM & (c) 2022 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenOsl/Nodes/MaterialNodeOsl.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/GenContext.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr MaterialNodeOsl::create()
{
    return std::make_shared<MaterialNodeOsl>();
}

void MaterialNodeOsl::emitFunctionDefinition(const ShaderNode&, GenContext& context, ShaderStage& stage) const
{
    const static string FUNCTION_DEFINITION =
        "MATERIAL mx_surfacematerial(surfaceshader surface, displacementshader disp)\n"
        "{\n"
        "    float opacity_weight = clamp(surface.opacity, 0.0, 1.0);\n"
        "    return (surface.bsdf + surface.edf) * opacity_weight + transparent() * (1.0 - opacity_weight);\n"
        "}\n\n";

    const ShaderGenerator& shadergen = context.getShaderGenerator();
    shadergen.emitBlock(FUNCTION_DEFINITION, FilePath(), context, stage);
}

void MaterialNodeOsl::emitFunctionCall(const ShaderNode& _node, GenContext& context, ShaderStage& stage) const
{
BEGIN_SHADER_STAGE(stage, Stage::PIXEL)

    ShaderNode& node = const_cast<ShaderNode&>(_node);
    ShaderInput* surfaceshaderInput = node.getInput(ShaderNode::SURFACESHADER);

    if (!surfaceshaderInput->getConnection())
    {
        // Just declare the output variable with default value.
        emitOutputVariables(node, context, stage);
        return;
    }

    const ShaderGenerator& shadergen = context.getShaderGenerator();

    // Emit the function call for upstream surface shader.
    const ShaderNode* surfaceshaderNode = surfaceshaderInput->getConnection()->getNode();
    shadergen.emitFunctionCall(*surfaceshaderNode, context, stage);

    shadergen.emitLineBegin(stage);

    // Emit the output and funtion name.
    shadergen.emitOutput(node.getOutput(), true, false, context, stage);
    shadergen.emitString(" = mx_surfacematerial(", stage);

    // Emit all inputs on the node.
    string delim = "";
    for (ShaderInput* input : node.getInputs())
    {
        shadergen.emitString(delim, stage);
        shadergen.emitInput(input, context, stage);
        delim = ", ";
    }

    // End function call
    shadergen.emitString(")", stage);
    shadergen.emitLineEnd(stage);

END_SHADER_STAGE(stage, Stage::PIXEL)
}

MATERIALX_NAMESPACE_END
