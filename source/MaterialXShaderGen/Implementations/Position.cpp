#include <MaterialXShaderGen/Implementations/Position.h>
#include <MaterialXShaderGen/ShaderGenerator.h>
#include <MaterialXShaderGen/SgNode.h>
#include <MaterialXShaderGen/HwShader.h>

namespace MaterialX
{

namespace
{
    static const string kLanguage = "glsl";
    static const string kTarget = "ogsfx";
    static const string kSpace  = "space";
    static const string kWorld  = "world";
    static const string kObject = "object";
    static const string kModel  = "model";
}

SgImplementationPtr PositionOgsFx::creator()
{
    return std::make_shared<PositionOgsFx>();
}

const string& PositionOgsFx::getLanguage() const
{
    return kLanguage;
}

const string& PositionOgsFx::getTarget() const
{
    return kTarget;
}

void PositionOgsFx::registerInputs(const SgNode& node, ShaderGenerator& /*shadergen*/, Shader& shader)
{
    shader.registerAttribute(Shader::Variable("vec3", "inPosition", "POSITION"));

    const SgInput* spaceInput = node.getInput(kSpace);
    string space = spaceInput ? spaceInput->value->getValueString() : "";
    if (space == kWorld)
    {
        shader.registerVarying(Shader::Variable("vec3", "WorldPosition", "POSITION"));
    }
    else if (space == kModel)
    {
        shader.registerVarying(Shader::Variable("vec3", "ModelPosition", "POSITION"));
    }
    else
    {
        shader.registerVarying(Shader::Variable("vec3", "ObjectPosition", "POSITION"));
    }
}

void PositionOgsFx::emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)
        const SgInput* spaceInput = node.getInput(kSpace);
        string space = spaceInput ? spaceInput->value->getValueString() : "";
        if (space == kWorld)
        {
            if (!shader.isCalculated("WorldPosition"))
            {
                shader.setCalculated("WorldPosition");
                shader.addLine("VS_OUT.WorldPosition = Pw.xyz");
            }
        }
        else if (space == kModel)
        {
            if (!shader.isCalculated("ModelPosition"))
            {
                shader.setCalculated("ModelPosition");
                shader.addLine("VS_OUT.ModelPosition = Po.xyz");
            }
        }
        else
        {
            if (!shader.isCalculated("ObjectPosition"))
            {
                shader.setCalculated("ObjectPosition");
                shader.addLine("VS_OUT.ObjectPosition = Po.xyz");
            }
        }
    END_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        shader.beginLine();
        shadergen.emitOutput(node.getOutput(), true, shader);

        const SgInput* spaceInput = node.getInput(kSpace);
        string space = spaceInput ? spaceInput->value->getValueString() : "";
        if (space == kWorld)
        {
            shader.addStr(" = PS_IN.WorldPosition");
        }
        else if (space == kModel)
        {
            shader.addStr(" = PS_IN.ModelPosition");
        }
        else
        {
            shader.addStr(" = PS_IN.ObjectPosition");
        }

        shader.endLine();
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
