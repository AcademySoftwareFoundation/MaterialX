//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/Nodes/LightShaderNodeGlsl.h>

#include <MaterialXGenShader/Shader.h>

namespace MaterialX
{

LightShaderNodeGlsl::LightShaderNodeGlsl() :
    _lightUniforms(HW::LIGHT_DATA, EMPTY_STRING)
{
}

ShaderNodeImplPtr LightShaderNodeGlsl::create()
{
    return std::make_shared<LightShaderNodeGlsl>();
}

const string& LightShaderNodeGlsl::getLanguage() const
{
    return GlslShaderGenerator::LANGUAGE;
}

const string& LightShaderNodeGlsl::getTarget() const
{
    return GlslShaderGenerator::TARGET;
}

void LightShaderNodeGlsl::initialize(const InterfaceElement& element, GenContext& context)
{
    SourceCodeNode::initialize(element, context);

    if (_inlined)
    {
        throw ExceptionShaderGenError("Light shaders doesn't support inlined implementations'");
    }

    if (!element.isA<Implementation>())
    {
        throw ExceptionShaderGenError("Element '" + element.getName() + "' is not an Implementation element");
    }

    const Implementation& impl = static_cast<const Implementation&>(element);

    // Store light uniforms for all inputs and parameters on the interface
    NodeDefPtr nodeDef = impl.getNodeDef();
    for (InputPtr input : nodeDef->getInputs())
    {
        _lightUniforms.add(TypeDesc::get(input->getType()), input->getName(), input->getValue());
    }
    for (ParameterPtr param : nodeDef->getParameters())
    {
        _lightUniforms.add(TypeDesc::get(param->getType()), param->getName(), param->getValue());
    }
}

void LightShaderNodeGlsl::createVariables(const ShaderNode&, GenContext&, Shader& shader) const
{
    ShaderStage& ps = shader.getStage(Stage::PIXEL);

    // Create all light uniforms
    VariableBlock& lightData = ps.getUniformBlock(HW::LIGHT_DATA);
    for (size_t i = 0; i < _lightUniforms.size(); ++i)
    {
        const ShaderPort* u = _lightUniforms[i];
        lightData.add(u->getType(), u->getName());
    }

    // Create uniform for number of active light sources
    ShaderPort* numActiveLights = addStageUniform(HW::PRIVATE_UNIFORMS, Type::INTEGER, "u_numActiveLightSources", ps);
    numActiveLights->setValue(Value::createValue<int>(0));
}

void LightShaderNodeGlsl::emitFunctionCall(const ShaderNode&, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        shadergen.emitLine(_functionName + "(light, position, result)", stage);
    END_SHADER_STAGE(shader, Stage::PIXEL)
}

} // namespace MaterialX
