#include <MaterialXShaderGen/HwShader.h>

namespace MaterialX
{

const string HwShader::VERTEX_DATA_BLOCK = "";

HwShader::HwShader(const string& name) 
    : Shader(name)
    , _vertexData("VertexData", "vd")
{
}

void HwShader::initialize(ElementPtr element, ShaderGenerator& shadergen)
{
    Shader::initialize(element, shadergen);
}

void HwShader::createVertexData(const string& type, const string& name, const string& sementic)
{
    if (_vertexData.variableMap.find(name) == _vertexData.variableMap.end())
    {
        VariablePtr variable = std::make_shared<Variable>(type, name, sementic);
        _vertexData.variableMap[name] = variable;
        _vertexData.variableOrder.push_back(variable.get());
    }
}

}
