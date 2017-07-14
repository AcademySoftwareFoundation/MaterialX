#include <MaterialXShaderGen/ShaderGenerators/OslShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/OslSyntax.h>

namespace MaterialX
{

OslShaderGenerator::OslShaderGenerator()
    : ShaderGenerator(std::make_shared<OslSyntax>())
{
}

ShaderPtr OslShaderGenerator::generate(NodePtr node, OutputPtr downstreamConnection)
{
    ShaderPtr shader = std::make_shared<Shader>();
    shader->initialize(node, downstreamConnection, getLanguage(), getTarget());

    Shader& shaderRef = *shader;

    emitTypeDefs(shaderRef);
    emitFunctions(shaderRef);

    emitShaderSignature(shaderRef);

    shaderRef.beginScope(Shader::Brackets::BRACES);
    emitShaderBody(shaderRef);
    shaderRef.endScope();

    return shader;
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
    const NodeGraph& graph = shader.getNodeGraph();

    // Emit shader type
    const string& outputType = shader.getOutput().getType();
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
    shader.addStr(graph.getName() + "\n");

    shader.beginScope(Shader::Brackets::PARENTHESES);

    // Emit varying variables used by the shader
    for (const Shader::Varying& varyings : shader.getVaryings())
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
    for (const Shader::Uniform& uniform : shader.getUniforms())
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
    const string variable = _syntax->getVariableName(shader.getOutput());
    const string value = _syntax->getTypeDefault(outputType, true);
    shader.addLine(type + " " + variable + " = " + value, false);

    shader.endScope();
}

} // namespace MaterialX
