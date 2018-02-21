#include <MaterialXShaderGen/HwShader.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslShaderGenerator.h>

namespace MaterialX
{

const string HwShader::LIGHT_DATA_BLOCK = "LightData";

HwShader::HwShader(const string& name) 
    : Shader(name)
    , _vertexData("VertexData", "vd")
{
    _stages.push_back(Stage("Vertex"));

    // Create default uniform blocks for vertex stage
    createUniformBlock(VERTEX_STAGE, PRIVATE_UNIFORMS, "prv");
    createUniformBlock(VERTEX_STAGE, PUBLIC_UNIFORMS, "pub");

    // Create light data uniform block with required field 'type'
    createUniformBlock(PIXEL_STAGE, LIGHT_DATA_BLOCK, "u_lightData[MAX_LIGHT_SOURCES]");
    createUniform(PIXEL_STAGE, LIGHT_DATA_BLOCK, DataType::INTEGER, "type");
}

void HwShader::initialize(ElementPtr element, ShaderGenerator& shadergen)
{
    Shader::initialize(element, shadergen);

    // For surface shaders we need light shaders
    if (_rootGraph->hasClassification(SgNode::Classification::SHADER | SgNode::Classification::SURFACE))
    {
        // Create variables for all bound light shaders
        HwShaderGenerator& sg = static_cast<HwShaderGenerator&>(shadergen);
        for (auto lightShader : sg.getBoundLightShaders())
        {
            lightShader.second->createVariables(SgNode::NONE, shadergen, *this);
        }
    }
}

void HwShader::createVertexData(const string& type, const string& name, const string& semantic)
{
    if (_vertexData.variableMap.find(name) == _vertexData.variableMap.end())
    {
        VariablePtr variable = std::make_shared<Variable>(type, name, semantic);
        _vertexData.variableMap[name] = variable;
        _vertexData.variableOrder.push_back(variable.get());
    }
}

}
