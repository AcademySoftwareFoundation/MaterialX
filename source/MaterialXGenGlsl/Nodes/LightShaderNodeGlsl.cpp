#include <MaterialXGenGlsl/Nodes/LightShaderNodeGlsl.h>

#include <MaterialXGenShader/Util.h>

namespace MaterialX
{

LightShaderNodeGlsl::LightShaderNodeGlsl()
    : _lightUniforms(HW::LIGHT_DATA, EMPTY_STRING)
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

void LightShaderNodeGlsl::initialize(ElementPtr implementation, const ShaderGenerator& shadergen, GenContext& context)
{
    SourceCodeNode::initialize(implementation, shadergen, context);

    if (_inlined)
    {
        throw ExceptionShaderGenError("Light shaders doesn't support inlined implementations'");
    }

    ImplementationPtr impl = implementation->asA<Implementation>();
    if (!impl)
    {
        throw ExceptionShaderGenError("Element '" + implementation->getName() + "' is not an Implementation element");
    }

    // Store light uniforms for all inputs and parameters on the interface
    NodeDefPtr nodeDef = impl->getNodeDef();
    for (InputPtr input : nodeDef->getInputs())
    {
        _lightUniforms.add(TypeDesc::get(input->getType()), input->getName(), EMPTY_STRING, input->getValue(), input->getNamePath());
    }
    for (ParameterPtr param : nodeDef->getParameters())
    {
        _lightUniforms.add(TypeDesc::get(param->getType()), param->getName(), EMPTY_STRING, param->getValue(), param->getNamePath());
    }
}

void LightShaderNodeGlsl::createVariables(Shader& shader, const ShaderNode&, const ShaderGenerator&, GenContext&) const
{
    ShaderStage& ps = shader.getStage(HW::PIXEL_STAGE);

    // Create all light uniforms
    for (size_t i = 0; i < _lightUniforms.size(); ++i)
    {
        const Variable& u = _lightUniforms[i];
        addStageUniform(ps, HW::LIGHT_DATA, u.getType(), u.getName());
    }

    // Create uniform for number of active light sources
    addStageUniform(ps, HW::PRIVATE_UNIFORMS, Type::INTEGER, "u_numActiveLightSources",
        EMPTY_STRING, Value::createValue<int>(0));
}

void LightShaderNodeGlsl::emitFunctionCall(ShaderStage& stage, const ShaderNode&, const ShaderGenerator& shadergen, GenContext&) const
{
BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
    shadergen.emitLine(stage, _functionName + "(light, position, result)");
END_SHADER_STAGE(shader, HW::PIXEL_STAGE)
}

} // namespace MaterialX
