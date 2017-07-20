#include <MaterialXShaderGen/ShaderGenerators/OslShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/OslSyntax.h>

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

void OslShaderGenerator::emitShaderBody(Shader &shader)
{
    // Emit needed globals
    shader.addLine("closure color null_closure = 0");

    // Call parent
    ShaderGenerator::emitShaderBody(shader);
}

void OslShaderGenerator::emitFinalOutput(Shader& shader) const
{
    const OutputPtr& output = shader.getOutput();
    const NodePtr connectedNode = output->getConnectedNode();

    string finalResult = _syntax->getVariableName(*connectedNode);

    const string& outputType = output->getType();
    if (outputType == kSURFACE)
    {
        finalResult = finalResult + ".bsdf + " + finalResult + ".edf";
    }
    else if (output->getChannels() != EMPTY_STRING)
    {
        finalResult = _syntax->getSwizzledVariable(finalResult, output->getType(), connectedNode->getType(), output->getChannels());
    }

    shader.addLine(_syntax->getVariableName(*output) + " = " + finalResult);
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
    const string variable = _syntax->getVariableName(*shader.getOutput());
    const string value = _syntax->getTypeDefault(outputType, true);
    shader.addLine(type + " " + variable + " = " + value, false);

    shader.endScope();
}

} // namespace MaterialX
