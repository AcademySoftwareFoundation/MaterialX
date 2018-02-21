#include <MaterialXShaderGen/ShaderGenerators/Glsl/LightGlsl.h>
#include <MaterialXShaderGen/Util.h>

namespace MaterialX
{

SgImplementationPtr LightGlsl::creator()
{
    return std::make_shared<LightGlsl>();
}

void LightGlsl::initialize(ElementPtr implementation, ShaderGenerator& shadergen)
{
    GlslImplementation::initialize(implementation, shadergen);

    ImplementationPtr impl = implementation->asA<Implementation>();
    if (!impl)
    {
        throw ExceptionShaderGenError("Element '" + implementation->getName() + "' is not an Implementation element");
    }

    const string& file = impl->getAttribute("file");
    if (file.empty())
    {
        throw ExceptionShaderGenError("No source file specified for implementation '" + impl->getName() + "'");
    }

    // Find the function name to use
    _functionName = impl->getAttribute("function");
    if (_functionName.empty())
    {
        // No function given so use nodedef name
        _functionName = impl->getNodeDefString();
    }

    _functionSource = "";
    if (!readFile(shadergen.findSourceCode(file), _functionSource))
    {
        throw ExceptionShaderGenError("Can't find source file '" + file + "' used by implementation '" + impl->getName() + "'");
    }

    NodeDefPtr nodeDef = impl->getNodeDef();
    _lightUniforms.resize(nodeDef->getInputCount() + nodeDef->getParameterCount());

    size_t index = 0;
    for (InputPtr input : nodeDef->getInputs())
    {
        _lightUniforms[index++] = Shader::Variable(input->getType(), input->getName());
    }
    for (ParameterPtr param : nodeDef->getParameters())
    {
        _lightUniforms[index++] = Shader::Variable(param->getType(), param->getName());
    }
}

void LightGlsl::createVariables(const SgNode& /*node*/, ShaderGenerator& /*shadergen*/, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    // Create variables used by this shader
    for (const Shader::Variable& uniform : _lightUniforms)
    {
        shader.createUniform(HwShader::PIXEL_STAGE, HwShader::LIGHT_DATA_BLOCK, uniform.type, uniform.name);
    }

    // Create uniform for number of active light sources
    shader.createUniform(HwShader::PIXEL_STAGE, HwShader::PRIVATE_UNIFORMS, DataType::INTEGER, "u_numActiveLightSources",
        EMPTY_STRING, Value::createValue<int>(1));
}

void LightGlsl::emitFunctionDefinition(const SgNode& /*node*/, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

        static const string INCLUDE_PATTERN = "#include ";

        std::stringstream stream(_functionSource);
        for (string line; std::getline(stream, line); )
        {
            size_t pos = line.find(INCLUDE_PATTERN);
            if (pos != string::npos)
            {
                const size_t start = pos + INCLUDE_PATTERN.size() + 1;
                const size_t count = line.size() - start - 1;
                const string filename = line.substr(start, count);
                shader.addInclude(filename, shadergen);
            }
            else
            {
                shader.addLine(line, false);
            }
        }

        shader.newLine();

    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

void LightGlsl::emitFunctionCall(const SgNode& /*node*/, ShaderGenerator& /*shadergen*/, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

        shader.addLine(_functionName + "(light, position, result)");

    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
