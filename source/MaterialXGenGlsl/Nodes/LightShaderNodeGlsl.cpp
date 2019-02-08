#include <MaterialXGenGlsl/Nodes/LightShaderNodeGlsl.h>

#include <MaterialXGenShader/Util.h>

namespace MaterialX
{

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

void LightShaderNodeGlsl::initialize(ElementPtr implementation, ShaderGenerator& shadergen, const GenOptions& options)
{
    SourceCodeNode::initialize(implementation, shadergen, options);

    if (_inlined)
    {
        throw ExceptionShaderGenError("Light shaders doesn't support inlined implementations'");
    }

    ImplementationPtr impl = implementation->asA<Implementation>();
    if (!impl)
    {
        throw ExceptionShaderGenError("Element '" + implementation->getName() + "' is not an Implementation element");
    }

    // Create light uniforms for all inputs and parameters on the nodedef
    NodeDefPtr nodeDef = impl->getNodeDef();
    _lightUniforms.resize(nodeDef->getInputCount() + nodeDef->getParameterCount());
    size_t index = 0;
    for (InputPtr input : nodeDef->getInputs())
    {
        _lightUniforms[index++] = Shader::Variable(TypeDesc::get(input->getType()), input->getName(), input->getNamePath(), EMPTY_STRING, input->getValue());
    }
    for (ParameterPtr param : nodeDef->getParameters())
    {
        _lightUniforms[index++] = Shader::Variable(TypeDesc::get(param->getType()), param->getName(), param->getNamePath(), EMPTY_STRING, param->getValue());
    }
}

void LightShaderNodeGlsl::createVariables(const ShaderNode& /*node*/, ShaderGenerator& /*shadergen*/, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    // Create variables used by this shader
    for (const Shader::Variable& uniform : _lightUniforms)
    {
        shader.createUniform(HwShader::PIXEL_STAGE, HwShader::LIGHT_DATA_BLOCK, uniform.type, uniform.name, EMPTY_STRING, uniform.semantic, uniform.value);
    }

    // Create uniform for number of active light sources
    shader.createUniform(HwShader::PIXEL_STAGE, HwShader::PRIVATE_UNIFORMS, Type::INTEGER, "u_numActiveLightSources",
        EMPTY_STRING, EMPTY_STRING, Value::createValue<int>(0));
}

void LightShaderNodeGlsl::emitFunctionCall(const ShaderNode& /*node*/, GenContext& /*context*/, ShaderGenerator& /*shadergen*/, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

        shader.addLine(_functionName + "(light, position, result)");

    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
