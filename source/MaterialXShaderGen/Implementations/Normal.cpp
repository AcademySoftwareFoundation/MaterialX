#include <MaterialXShaderGen/Implementations/Normal.h>
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

SgImplementationPtr NormalOgsFx::creator()
{
    return std::make_shared<NormalOgsFx>();
}

const string& NormalOgsFx::getLanguage() const
{
    return kLanguage;
}

const string& NormalOgsFx::getTarget() const
{
    return kTarget;
}

void NormalOgsFx::registerInputs(const SgNode& node, ShaderGenerator& /*shadergen*/, Shader& shader)
{
    shader.registerAttribute(Shader::Variable("vec3", "inNormal", "NORMAL"));

    const SgInput* spaceInput = node.getInput(kSpace);
    string space = spaceInput ? spaceInput->value->getValueString() : "";
    if (space == kWorld)
    {
        shader.registerUniform(Shader::Variable("mat4", "gWorldITXf", "WorldInverseTranspose"));
        shader.registerVarying(Shader::Variable("vec3", "WorldNormal", "NORMAL"));
    }
    else if (space == kModel)
    {
        // TODO: add uniform for model space transformation matrix
        shader.registerVarying(Shader::Variable("vec3", "ModelNormal", "NORMAL"));
    }
    else
    {
        shader.registerVarying(Shader::Variable("vec3", "ObjectNormal", "NORMAL"));
    }
}

void NormalOgsFx::emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)
        const SgInput* spaceInput = node.getInput(kSpace);
        string space = spaceInput ? spaceInput->value->getValueString() : "";
        if (space == kWorld)
        {
            if (!shader.isCalculated("WorldNormal"))
            {
                shader.setCalculated("WorldNormal");
                shader.addLine("VS_OUT.WorldNormal = normalize((gWorldITXf * vec4(inNormal,0)).xyz)");
                shader.addLine("float frontFacing = dot(VS_OUT.WorldNormal, VS_OUT.WorldView)");
                shader.addLine("if (frontFacing < 0.0)", false);
                shader.beginScope();
                shader.addLine("VS_OUT.WorldNormal = -VS_OUT.WorldNormal");
                shader.endScope();
            }
        }
        else if (space == kModel)
        {
            if (!shader.isCalculated("ModelNormal"))
            {
                shader.setCalculated("ModelNormal");
                shader.addLine("VS_OUT.ModelNormal = normalize(inNormal)");
            }
        }
        else
        {
            if (!shader.isCalculated("ObjectNormal"))
            {
                shader.setCalculated("ObjectNormal");
                shader.addLine("VS_OUT.ObjectNormal = normalize(inNormal)");
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
            shader.addStr(" = PS_IN.WorldNormal");
        }
        else if (space == kModel)
        {
            shader.addStr(" = PS_IN.ModelNormal");
        }
        else
        {
            shader.addStr(" = PS_IN.ObjectNormal");
        }

        shader.endLine();
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
