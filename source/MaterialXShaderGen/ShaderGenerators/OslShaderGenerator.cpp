#include <MaterialXShaderGen/ShaderGenerators/OslShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/OslSyntax.h>
#include <MaterialXShaderGen/ShaderGenRegistry.h>

namespace MaterialX
{

OslShaderGenerator::OslShaderGenerator()
    : ShaderGenerator(std::make_shared<OslSyntax>())
{
}

ShaderPtr OslShaderGenerator::generate(const string& shaderName, ElementPtr element)
{
    ShaderPtr shaderPtr = std::make_shared<Shader>(shaderName);
    shaderPtr->initialize(element, getLanguage(), getTarget());

    Shader& shader = *shaderPtr;

    emitIncludes(shader);
    emitTypeDefs(shader);
    emitFunctions(shader);

    emitShaderSignature(shader);

    shader.beginScope(Shader::Brackets::BRACES);
    emitShaderBody(shader);
    shader.endScope();

    // Release resources used by shader gen
    shaderPtr->finalize();

    return shaderPtr;
}

void OslShaderGenerator::emitIncludes(Shader& shader)
{
    static const vector<string> includeFiles =
    {
        "color2.h",
        "color4.h",
        "vector2.h",
        "vector4.h",
        "mx_funcs.h"
    };

    for (const string& file : includeFiles)
    {
        FilePath path = ShaderGenRegistry::findSourceCode(file);
        shader.addLine("#include \"" + path.asString() + "\"", false);
    }

    shader.newLine();
}

void OslShaderGenerator::emitShaderBody(Shader &shader)
{
    // Emit needed globals
    shader.addLine("closure color null_closure = 0");

    // Call parent
    ShaderGenerator::emitShaderBody(shader);
}

void OslShaderGenerator::emitShaderSignature(Shader &shader)
{
    const NodeGraphPtr& graph = shader.getNodeGraph();

    // Emit shader type
    const string& outputType = shader.getOutput()->getType();
    if (outputType == "surfaceshader")
    {
        shader.addStr("surface ");
    }
    else if (outputType == "displacementshader")
    {
        shader.addStr("volume ");
    }
    else
    {
        shader.addStr("shader ");
    }

    // Emit shader name
    shader.addStr(graph->getName() + "\n");

    shader.beginScope(Shader::Brackets::PARENTHESES);

    // Emit varying variables used by the shader
    for (const auto& varyings : shader.getVaryings())
    {
        shader.beginLine();
        emitUniform(
            varyings.first,
            varyings.second->getType(),
            varyings.second->getValue(),
            shader
        );
        shader.addStr(",");
        shader.endLine(false);
    }

    // Emit uniforms variables used by the shader
    for (const auto& uniform : shader.getUniforms())
    {
        shader.beginLine();
        emitUniform(
            uniform.first,
            uniform.second->getType(),
            uniform.second->getValue(),
            shader
        );
        shader.addStr(",");
        shader.endLine(false);
    }

    // Emit shader output
    const string type = _syntax->getOutputTypeName(outputType);
    const string variable = _syntax->getVariableName(*shader.getOutput());
    const string value = _syntax->getTypeDefault(outputType, true);
    shader.addLine(type + " " + variable + " = " + value, false);

    shader.endScope();
}

} // namespace MaterialX
