#include <MaterialXGenGlsl/Nodes/NormalGlsl.h>

namespace MaterialX
{

GenImplementationPtr NormalGlsl::create()
{
    return std::make_shared<NormalGlsl>();
}

void NormalGlsl::createVariables(const DagNode& node, ShaderGenerator& /*shadergen*/, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    shader.createAppData(Type::VECTOR3, "i_normal");

    const DagInput* spaceInput = node.getInput(SPACE);
    string space = spaceInput ? spaceInput->value->getValueString() : EMPTY_STRING;
    if (space == WORLD)
    {
        shader.createUniform(HwShader::VERTEX_STAGE, HwShader::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldInverseTransposeMatrix");
        shader.createVertexData(Type::VECTOR3, "normalWorld");
    }
    else if (space == MODEL)
    {
        shader.createVertexData(Type::VECTOR3, "normalModel");
    }
    else
    {
        shader.createVertexData(Type::VECTOR3, "normalObject");
    }
}

void NormalGlsl::emitFunctionCall(const DagNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    const string& blockInstance = shader.getVertexDataBlock().instance;
    const string blockPrefix = blockInstance.length() ? blockInstance + "." : EMPTY_STRING;

    const DagInput* spaceInput = node.getInput(SPACE);
    string space = spaceInput ? spaceInput->value->getValueString() : EMPTY_STRING;

    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)
        if (space == WORLD)
        {
            if (!shader.isCalculated("normalWorld"))
            {
                shader.setCalculated("normalWorld");
                shader.addLine(blockPrefix + "normalWorld = (u_worldInverseTransposeMatrix * vec4(i_normal,0.0)).xyz");
            }
        }
        else if (space == MODEL)
        {
            if (!shader.isCalculated("normalModel"))
            {
                shader.setCalculated("normalModel");
                shader.addLine(blockPrefix + "normalModel = i_normal");
            }
        }
        else
        {
            if (!shader.isCalculated("normalObject"))
            {
                shader.setCalculated("normalObject");
                shader.addLine(blockPrefix + "normalObject = i_normal");
            }
        }
    END_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        shader.beginLine();
        shadergen.emitOutput(context, node.getOutput(), true, false, shader);
        if (space == WORLD)
        {
            shader.addStr(" = normalize(" + blockPrefix + "normalWorld)");
        }
        else if (space == MODEL)
        {
            shader.addStr(" = normalize(" + blockPrefix + "normalModel)");
        }
        else
        {
            shader.addStr(" = normalize(" + blockPrefix + "normalObject)");
        }

        shader.endLine();
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
