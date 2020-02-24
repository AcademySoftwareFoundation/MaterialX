//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/Nodes/GeomPropValueNodeGlsl.h>

#include <MaterialXGenShader/Shader.h>

namespace MaterialX
{

ShaderNodeImplPtr GeomPropValueNodeGlsl::create()
{
    return std::make_shared<GeomPropValueNodeGlsl>();
}

void GeomPropValueNodeGlsl::createVariables(const ShaderNode& node, GenContext&, Shader& shader) const
{
    const ShaderInput* geomPropInput = node.getInput(GEOMPROP);
    if (!geomPropInput || !geomPropInput->getValue())
    {
        throw ExceptionShaderGenError("No 'geomprop' parameter found on geompropvalue node '" + node.getName() + "', don't know what property to bind");
    }
    const string geomProp = geomPropInput->getValue()->getValueString();
    ShaderStage& ps = shader.getStage(Stage::PIXEL);
    ShaderPort* uniform = addStageUniform(HW::PRIVATE_UNIFORMS, node.getOutput()->getType(), HW::T_GEOMPROP + "_" + geomProp, ps);
    uniform->setPath(geomPropInput->getPath());
}

void GeomPropValueNodeGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        const ShaderInput* geomPropInput = node.getInput(GEOMPROP);
        if (!geomPropInput)
        {
            throw ExceptionShaderGenError("No 'geomprop' parameter found on geompropvalue node '" + node.getName() + "', don't know what property to bind");
        }
        const string attrName = geomPropInput->getValue()->getValueString();
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        shadergen.emitString(" = " + HW::T_GEOMPROP + "_" + attrName, stage);
        shadergen.emitLineEnd(stage);
    END_SHADER_STAGE(shader, Stage::PIXEL)
}

} // namespace MaterialX
