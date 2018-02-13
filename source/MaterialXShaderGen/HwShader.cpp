#include <MaterialXShaderGen/HwShader.h>

namespace MaterialX
{

HwShader::HwShader(const string& name) 
    : Shader(name)
    , _vertexData("VertexData", "vd")
{
}

void HwShader::initialize(ElementPtr element, ShaderGenerator& shadergen)
{
    Shader::initialize(element, shadergen);
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
