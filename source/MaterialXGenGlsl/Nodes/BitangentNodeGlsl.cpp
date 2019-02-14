#include <MaterialXGenGlsl/Nodes/BitangentNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr BitangentNodeGlsl::create()
{
    return std::make_shared<BitangentNodeGlsl>();
}

void BitangentNodeGlsl::createVariables(const ShaderNode& node, Shader& shader)
{
    ShaderStage& vs = shader.getStage(HwShader::VERTEX_STAGE);
    ShaderStage& ps = shader.getStage(HwShader::PIXEL_STAGE);

    addStageInput(vs, HwShader::APP_DATA, Type::VECTOR3, "i_bitangent");

    const ShaderInput* spaceInput = node.getInput(SPACE);
    const string& space = spaceInput ? spaceInput->value->getValueString() : EMPTY_STRING;
    if (space == WORLD)
    {
        const string& path = spaceInput ? spaceInput->path : EMPTY_STRING;
        addStageUniform(vs, HwShader::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldInverseTransposeMatrix", EMPTY_STRING, nullptr, path);
        addStageConnector(vs, ps, HwShader::VERTEX_DATA, Type::VECTOR3, "bitangentWorld");
    }
    else if (space == MODEL)
    {
        addStageConnector(vs, ps, HwShader::VERTEX_DATA, Type::VECTOR3, "bitangentModel");
    }
    else
    {
        addStageConnector(vs, ps, HwShader::VERTEX_DATA, Type::VECTOR3, "bitangentObject");
    }
}

void BitangentNodeGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, ShaderStage& stage)
{
    const ShaderInput* spaceInput = node.getInput(SPACE);
    const string& space = spaceInput ? spaceInput->value->getValueString() : EMPTY_STRING;

    BEGIN_SHADER_STAGE(stage, HwShader::VERTEX_STAGE)
        VariableBlock& vd = stage.getOutputBlock(HwShader::VERTEX_DATA);
        if (space == WORLD)
        {
            Variable& bitangent = vd["bitangentWorld"];
            if (!bitangent.isCalculated())
            {
                bitangent.setCalculated();
                stage.addLine(bitangent.getFullName() + " = (u_worldInverseTransposeMatrix * vec4(i_bitangent,0.0)).xyz");
            }
        }
        else if (space == MODEL)
        {
            Variable& bitangent = vd["bitangentModel"];
            if (!bitangent.isCalculated())
            {
                bitangent.setCalculated();
                stage.addLine(bitangent.getFullName() + " = i_bitangent");
            }
        }
        else
        {
            Variable& bitangent = vd["bitangentObject"];
            if (!bitangent.isCalculated())
            {
                bitangent.setCalculated();
                stage.addLine(bitangent.getFullName() + " = i_bitangent");
            }
        }
    END_SHADER_STAGE(stage, HwShader::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(stage, HwShader::PIXEL_STAGE)
        stage.beginLine();
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
