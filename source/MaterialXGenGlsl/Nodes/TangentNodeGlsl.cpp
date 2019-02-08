#include <MaterialXGenGlsl/Nodes/TangentNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr TangentNodeGlsl::create()
{
    return std::make_shared<TangentNodeGlsl>();
}

void TangentNodeGlsl::createVariables(const ShaderNode& node, ShaderGenerator& /*shadergen*/, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    shader.createAppData(Type::VECTOR3, "i_tangent");

    const ShaderInput* spaceInput = node.getInput(SPACE);
    const string& space = spaceInput ? spaceInput->value->getValueString() : EMPTY_STRING;
    const string& path = spaceInput ? spaceInput->path : EMPTY_STRING;
    if (space == WORLD)
    {
        shader.createUniform(HwShader::VERTEX_STAGE, HwShader::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldInverseTransposeMatrix", path);
        shader.createVertexData(Type::VECTOR3, "tangentWorld");
    }
    else if (space == MODEL)
    {
        shader.createVertexData(Type::VECTOR3, "tangentModel");
    }
    else
    {
        shader.createVertexData(Type::VECTOR3, "tangentObject");
    }
}

void TangentNodeGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    const string& blockInstance = shader.getVertexDataBlock().instance;
    const string blockPrefix = blockInstance.length() ? blockInstance + "." : EMPTY_STRING;

    const ShaderInput* spaceInput = node.getInput(SPACE);
    const string& space = spaceInput ? spaceInput->value->getValueString() : EMPTY_STRING;

    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)
        if (space == WORLD)
        {
            if (!shader.isCalculated("tangentWorld"))
            {
                shader.setCalculated("tangentWorld");
                shader.addLine(blockPrefix + "tangentWorld = (u_worldInverseTransposeMatrix * vec4(i_tangent,0.0)).xyz");
            }
        }
        else if (space == MODEL)
        {
            if (!shader.isCalculated("tangentModel"))
            {
                shader.setCalculated("tangentModel");
                shader.addLine(blockPrefix + "tangentModel = i_tangent");
            }
        }
        else
        {
            if (!shader.isCalculated("tangentObject"))
            {
                shader.setCalculated("tangentObject");
                shader.addLine(blockPrefix + "tangentObject = i_tangent");
            }
        }
    END_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

        BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        shader.beginLine();
    shadergen.emitOutput(context, node.getOutput(), true, false, shader);
    if (space == WORLD)
    {
        shader.addStr(" = normalize(" + blockPrefix + "tangentWorld)");
    }
    else if (space == MODEL)
    {
        shader.addStr(" = normalize(" + blockPrefix + "tangentModel)");
    }
    else
    {
        shader.addStr(" = normalize(" + blockPrefix + "tangentObject)");
    }
    shader.endLine();
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
