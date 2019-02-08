#include <MaterialXGenGlsl/Nodes/BitangentNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr BitangentNodeGlsl::create()
{
    return std::make_shared<BitangentNodeGlsl>();
}

void BitangentNodeGlsl::createVariables(const ShaderNode& node, ShaderGenerator& /*shadergen*/, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    shader.createAppData(Type::VECTOR3, "i_bitangent");

    const ShaderInput* spaceInput = node.getInput(SPACE);
    const string& space = spaceInput ? spaceInput->value->getValueString() : EMPTY_STRING;
    const string& path = spaceInput ? spaceInput->path : EMPTY_STRING;
    if (space == WORLD)
    {
        shader.createUniform(HwShader::VERTEX_STAGE, HwShader::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldInverseTransposeMatrix", path);
        shader.createVertexData(Type::VECTOR3, "bitangentWorld");
    }
    else if (space == MODEL)
    {
        shader.createVertexData(Type::VECTOR3, "bitangentModel");
    }
    else
    {
        shader.createVertexData(Type::VECTOR3, "bitangentObject");
    }
}

void BitangentNodeGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    const string& blockInstance = shader.getVertexDataBlock().instance;
    const string blockPrefix = blockInstance.length() ? blockInstance + "." : EMPTY_STRING;

    const ShaderInput* spaceInput = node.getInput(SPACE);
    const string& space = spaceInput ? spaceInput->value->getValueString() : EMPTY_STRING;

    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)
        if (space == WORLD)
        {
            if (!shader.isCalculated("bitangentWorld"))
            {
                shader.setCalculated("bitangentWorld");
                shader.addLine(blockPrefix + "bitangentWorld = (u_worldInverseTransposeMatrix * vec4(i_bitangent,0.0)).xyz");
            }
        }
        else if (space == MODEL)
        {
            if (!shader.isCalculated("bitangentModel"))
            {
                shader.setCalculated("bitangentModel");
                shader.addLine(blockPrefix + "bitangentModel = i_bitangent");
            }
        }
        else
        {
            if (!shader.isCalculated("bitangentObject"))
            {
                shader.setCalculated("bitangentObject");
                shader.addLine(blockPrefix + "bitangentObject = i_bitangent");
            }
        }
    END_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        shader.beginLine();
        shadergen.emitOutput(context, node.getOutput(), true, false, shader);
        if (space == WORLD)
        {
            shader.addStr(" = normalize(" + blockPrefix + "bitangentWorld)");
        }
        else if (space == MODEL)
        {
            shader.addStr(" = normalize(" + blockPrefix + "bitangentModel)");
        }
        else
        {
            shader.addStr(" = normalize(" + blockPrefix + "bitangentObject)");
        }
        shader.endLine();
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
