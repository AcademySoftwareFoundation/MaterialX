#include <MaterialXShaderGen/Implementations/Tangent.h>
#include <MaterialXShaderGen/ShaderGenerator.h>
#include <MaterialXShaderGen/SgNode.h>
#include <MaterialXShaderGen/HwShader.h>

namespace MaterialX
{

SgImplementationPtr TangentOgsFx::creator()
{
    return std::make_shared<TangentOgsFx>();
}

void TangentOgsFx::registerInputs(const SgNode& node, ShaderGenerator& /*shadergen*/, Shader& shader)
{
    shader.registerAttribute(Shader::Variable("vec3", "inTangent", "TANGENT"));

    const SgInput* spaceInput = node.getInput(SPACE);
    string space = spaceInput ? spaceInput->value->getValueString() : "";
    if (space == WORLD)
    {
        shader.registerUniform(Shader::Variable("mat4", "gWorldITXf", "WorldInverseTranspose"));
        shader.registerVarying(Shader::Variable("vec3", "WorldTangent", "TANGENT"));
    }
    else if (space == MODEL)
    {
        shader.registerVarying(Shader::Variable("vec3", "ModelTangent", "TANGENT"));
    }
    else
    {
        shader.registerVarying(Shader::Variable("vec3", "ObjectTangent", "TANGENT"));
    }
}

void TangentOgsFx::emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)
        const SgInput* spaceInput = node.getInput(SPACE);
        string space = spaceInput ? spaceInput->value->getValueString() : "";
        if (space == WORLD)
        {
            if (!shader.isCalculated("WorldTangent"))
            {
                shader.setCalculated("WorldTangent");
                shader.addLine("VS_OUT.WorldTangent = normalize((gWorldITXf * vec4(inTangent,0)).xyz)");
            }
        }
        else if (space == MODEL)
        {
            if (!shader.isCalculated("ModelTangent"))
            {
                shader.setCalculated("ModelTangent");
                shader.addLine("VS_OUT.ModelTangent = normalize(inTangent)");
            }
        }
        else
        {
            if (!shader.isCalculated("ObjectTangent"))
            {
                shader.setCalculated("ObjectTangent");
                shader.addLine("VS_OUT.ObjectTangent = normalize(inTangent)");
            }
        }
    END_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        shader.beginLine();
        shadergen.emitOutput(node.getOutput(), true, shader);

        const SgInput* spaceInput = node.getInput(SPACE);
        string space = spaceInput ? spaceInput->value->getValueString() : "";
        if (space == WORLD)
        {
            shader.addStr(" = PS_IN.WorldTangent");
        }
        else if (space == MODEL)
        {
            shader.addStr(" = PS_IN.ModelTangent");
        }
        else
        {
            shader.addStr(" = PS_IN.ObjectTangent");
        }

        shader.endLine();
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
