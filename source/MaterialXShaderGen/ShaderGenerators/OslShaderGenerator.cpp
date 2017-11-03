#include <MaterialXShaderGen/ShaderGenerators/OslShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/OslSyntax.h>

#include <MaterialXShaderGen/Implementations/Swizzle.h>
#include <MaterialXShaderGen/Implementations/Switch.h>
#include <MaterialXShaderGen/Implementations/Compare.h>

namespace
{
    const char* kVDirectionFlip =
        "void vdirection(vector2 texcoord, output vector2 result)\n"
        "{\n"
        "   result.x = texcoord.x;\n"
        "   result.y = 1.0 - texcoord.y;\n"
        "}\n\n";

    const char* kVDirectionNoop =
        "void vdirection(vector2 texcoord, output vector2 result)\n"
        "{\n"
        "   result = texcoord;\n"
        "}\n\n";
}

namespace MaterialX
{

OslShaderGenerator::OslShaderGenerator()
    : ShaderGenerator(std::make_shared<OslSyntax>())
{
    // Register build-in node implementations

    // <!-- <compare> -->
    registerNodeImplementation("IM_compare__float__osl", Compare::creator);
    registerNodeImplementation("IM_compare__color2__osl", Compare::creator);
    registerNodeImplementation("IM_compare__color3__osl", Compare::creator);
    registerNodeImplementation("IM_compare__color4__osl", Compare::creator);
    registerNodeImplementation("IM_compare__vector2__osl", Compare::creator);
    registerNodeImplementation("IM_compare__vector3__osl", Compare::creator);
    registerNodeImplementation("IM_compare__vector4__osl", Compare::creator);

    // <!-- <switch> -->
    registerNodeImplementation("IM_switch__float__osl", Switch::creator);
    registerNodeImplementation("IM_switch__color2__osl", Switch::creator);
    registerNodeImplementation("IM_switch__color3__osl", Switch::creator);
    registerNodeImplementation("IM_switch__color4__osl", Switch::creator);
    registerNodeImplementation("IM_switch__vector2__osl", Switch::creator);
    registerNodeImplementation("IM_switch__vector3__osl", Switch::creator);
    registerNodeImplementation("IM_switch__vector4__osl", Switch::creator);

    // <!-- <swizzle> -->
    // <!-- from type : float -->
    registerNodeImplementation("IM_swizzle__float_color2__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__float_color3__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__float_color4__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__float_vector2__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__float_vector3__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__float_vector4__osl", Swizzle::creator);
    // <!-- from type : color2 -->
    registerNodeImplementation("IM_swizzle__color2_float__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__color2_color2__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__color2_color3__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__color2_color4__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__color2_vector2__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__color2_vector3__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__color2_vector4__osl", Swizzle::creator);
    // <!-- from type : color3 -->
    registerNodeImplementation("IM_swizzle__color3_float__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__color3_color2__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__color3_color3__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__color3_color4__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__color3_vector2__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__color3_vector3__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__color3_vector4__osl", Swizzle::creator);
    // <!-- from type : color4 -->
    registerNodeImplementation("IM_swizzle__color4_float__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__color4_color2__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__color4_color3__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__color4_color4__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__color4_vector2__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__color4_vector3__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__color4_vector4__osl", Swizzle::creator);
    // <!-- from type : vector2 -->
    registerNodeImplementation("IM_swizzle__vector2_float__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__vector2_color2__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__vector2_color3__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__vector2_color4__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__vector2_vector2__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__vector2_vector3__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__vector2_vector4__osl", Swizzle::creator);
    // <!-- from type : vector3 -->
    registerNodeImplementation("IM_swizzle__vector3_float__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__vector3_color2__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__vector3_color3__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__vector3_color4__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__vector3_vector2__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__vector3_vector3__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__vector3_vector4__osl", Swizzle::creator);
    // <!-- from type : vector4 -->
    registerNodeImplementation("IM_swizzle__vector4_float__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__vector4_color2__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__vector4_color3__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__vector4_color4__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__vector4_vector2__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__vector4_vector3__osl", Swizzle::creator);
    registerNodeImplementation("IM_swizzle__vector4_vector4__osl", Swizzle::creator);
}

ShaderPtr OslShaderGenerator::generate(const string& shaderName, ElementPtr element)
{
    ShaderPtr shaderPtr = std::make_shared<Shader>(shaderName);
    shaderPtr->initialize(element, *this);

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

void OslShaderGenerator::emitFunctions(Shader& shader)
{
    // Emit function for handling texture coords v-flip 
    // as needed by the v-direction set by the user
    shader.addBlock(shader.getRequestedVDirection() != getTargetVDirection() ? kVDirectionFlip : kVDirectionNoop);

    // Call parent to emit all other functions
    ShaderGenerator::emitFunctions(shader);
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
        FilePath path = ShaderGenerator::findSourceCode(file);
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
