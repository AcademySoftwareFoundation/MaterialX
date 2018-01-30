#include <MaterialXShaderGen/ShaderGenerators/Glsl/OgsFx/BitangentOgsFx.h>
#include <MaterialXShaderGen/ShaderGenerator.h>
#include <MaterialXShaderGen/SgNode.h>
#include <MaterialXShaderGen/HwShader.h>

namespace MaterialX
{

SgImplementationPtr BitangentOgsFx::creator()
{
    return std::make_shared<BitangentOgsFx>();
}

void BitangentOgsFx::registerInputs(const SgNode& node, ShaderGenerator& /*shadergen*/, Shader& shader)
{
    shader.registerAttribute(Shader::Variable("vec3", "inBitangent", "BITANGENT"));

    const SgInput* spaceInput = node.getInput(SPACE);
    string space = spaceInput ? spaceInput->value->getValueString() : "";
    if (space == WORLD)
    {
        shader.registerUniform(Shader::Variable("mat4", "gWorldITXf", "WorldInverseTranspose"));
        shader.registerVarying(Shader::Variable("vec3", "WorldBitangent", "BITANGENT"));
    }
    else if (space == MODEL)
    {
        shader.registerVarying(Shader::Variable("vec3", "ModelBitangent", "BITANGENT"));
    }
    else
    {
        shader.registerVarying(Shader::Variable("vec3", "ObjectBitangent", "BITANGENT"));
    }
}

void BitangentOgsFx::emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)
        const SgInput* spaceInput = node.getInput(SPACE);
        string space = spaceInput ? spaceInput->value->getValueString() : "";
        if (space == WORLD)
        {
            if (!shader.isCalculated("WorldBitangent"))
            {
                shader.setCalculated("WorldBitangent");
                shader.addLine("VS_OUT.WorldBitangent = normalize((gWorldITXf * vec4(inBitangent,0)).xyz)");
            }
        }
        else if (space == MODEL)
        {
            if (!shader.isCalculated("ModelBitangent"))
            {
                shader.setCalculated("ModelBitangent");
                shader.addLine("VS_OUT.ModelBitangent = normalize(inBitangent)");
            }
        }
        else
        {
            if (!shader.isCalculated("ObjectBitangent"))
            {
                shader.setCalculated("ObjectBitangent");
                shader.addLine("VS_OUT.ObjectBitangent = normalize(inBitangent)");
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
            shader.addStr(" = PS_IN.WorldBitangent");
        }
        else if (space == MODEL)
        {
            shader.addStr(" = PS_IN.ModelBitangent");
        }
        else
        {
            shader.addStr(" = PS_IN.ObjectBitangent");
        }
        shader.endLine();
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
