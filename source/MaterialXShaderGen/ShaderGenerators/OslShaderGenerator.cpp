#include <MaterialXShaderGen/ShaderGenerators/OslShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/OslSyntax.h>

#include <MaterialXShaderGen/Implementations/Swizzle.h>
#include <MaterialXShaderGen/Implementations/Switch.h>
#include <MaterialXShaderGen/Implementations/Compare.h>

namespace MaterialX
{

namespace
{
    const char* VDIRECTION_FLIP =
        "void vdirection(vector2 texcoord, output vector2 result)\n"
        "{\n"
        "   result.x = texcoord.x;\n"
        "   result.y = 1.0 - texcoord.y;\n"
        "}\n\n";

    const char* VDIRECTION_NOOP =
        "void vdirection(vector2 texcoord, output vector2 result)\n"
        "{\n"
        "   result = texcoord;\n"
        "}\n\n";
}

const string OslShaderGenerator::LANGUAGE = "osl";

OslShaderGenerator::OslShaderGenerator()
    : ShaderGenerator(std::make_shared<OslSyntax>())
{
    // Register build-in implementations

    // <!-- <compare> -->
    registerImplementation("IM_compare__float__osl", Compare::creator);
    registerImplementation("IM_compare__color2__osl", Compare::creator);
    registerImplementation("IM_compare__color3__osl", Compare::creator);
    registerImplementation("IM_compare__color4__osl", Compare::creator);
    registerImplementation("IM_compare__vector2__osl", Compare::creator);
    registerImplementation("IM_compare__vector3__osl", Compare::creator);
    registerImplementation("IM_compare__vector4__osl", Compare::creator);

    // <!-- <switch> -->
    registerImplementation("IM_switch__float__osl", Switch::creator);
    registerImplementation("IM_switch__color2__osl", Switch::creator);
    registerImplementation("IM_switch__color3__osl", Switch::creator);
    registerImplementation("IM_switch__color4__osl", Switch::creator);
    registerImplementation("IM_switch__vector2__osl", Switch::creator);
    registerImplementation("IM_switch__vector3__osl", Switch::creator);
    registerImplementation("IM_switch__vector4__osl", Switch::creator);

    // <!-- <swizzle> -->
    // <!-- from type : float -->
    registerImplementation("IM_swizzle__float_color2__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__float_color3__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__float_color4__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__float_vector2__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__float_vector3__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__float_vector4__osl", Swizzle::creator);
    // <!-- from type : color2 -->
    registerImplementation("IM_swizzle__color2_float__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__color2_color2__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__color2_color3__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__color2_color4__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__color2_vector2__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__color2_vector3__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__color2_vector4__osl", Swizzle::creator);
    // <!-- from type : color3 -->
    registerImplementation("IM_swizzle__color3_float__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__color3_color2__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__color3_color3__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__color3_color4__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__color3_vector2__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__color3_vector3__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__color3_vector4__osl", Swizzle::creator);
    // <!-- from type : color4 -->
    registerImplementation("IM_swizzle__color4_float__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__color4_color2__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__color4_color3__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__color4_color4__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__color4_vector2__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__color4_vector3__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__color4_vector4__osl", Swizzle::creator);
    // <!-- from type : vector2 -->
    registerImplementation("IM_swizzle__vector2_float__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector2_color2__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector2_color3__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector2_color4__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector2_vector2__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector2_vector3__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector2_vector4__osl", Swizzle::creator);
    // <!-- from type : vector3 -->
    registerImplementation("IM_swizzle__vector3_float__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector3_color2__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector3_color3__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector3_color4__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector3_vector2__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector3_vector3__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector3_vector4__osl", Swizzle::creator);
    // <!-- from type : vector4 -->
    registerImplementation("IM_swizzle__vector4_float__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector4_color2__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector4_color3__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector4_color4__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector4_vector2__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector4_vector3__osl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector4_vector4__osl", Swizzle::creator);
}

ShaderPtr OslShaderGenerator::generate(const string& shaderName, ElementPtr element)
{
    ShaderPtr shaderPtr = std::make_shared<Shader>(shaderName);
    shaderPtr->initialize(element, *this);

    Shader& shader = *shaderPtr;

    emitIncludes(shader);
    emitTypeDefs(shader);
    emitFunctionDefinitions(shader);

    emitShaderSignature(shader);

    shader.beginScope(Shader::Brackets::BRACES);
    emitFunctionCalls(shader);
    emitFinalOutput(shader);
    shader.endScope();

    return shaderPtr;
}

void OslShaderGenerator::emitFunctionDefinitions(Shader& shader)
{
    // Emit function for handling texture coords v-flip 
    // as needed by the v-direction set by the user
    shader.addBlock(shader.getRequestedVDirection() != getTargetVDirection() ? VDIRECTION_FLIP : VDIRECTION_NOOP);

    // Call parent to emit all other functions
    ShaderGenerator::emitFunctionDefinitions(shader);
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
        FilePath path = findSourceCode(file);
        shader.addLine("#include \"" + path.asString() + "\"", false);
    }

    shader.newLine();
}

void OslShaderGenerator::emitFunctionCalls(Shader &shader)
{
    // Emit needed globals
    if (!shader.getNodeGraph()->hasClassification(SgNode::Classification::TEXTURE))
    {
        shader.addLine("closure color null_closure = 0");
    }

    // Call parent
    ShaderGenerator::emitFunctionCalls(shader);
}

void OslShaderGenerator::emitShaderSignature(Shader &shader)
{
    // Emit shader type
    SgNodeGraph* graph = shader.getNodeGraph();
    const SgOutputSocket* outputSocket = graph->getOutputSocket();

    if (outputSocket->type == "surfaceshader")
    {
        shader.addStr("surface ");
    }
    else if (outputSocket->type == "volumeshader")
    {
        shader.addStr("volume ");
    }
    else
    {
        shader.addStr("shader ");
    }

    // Emit shader name
    shader.addStr(shader.getName() + "\n");

    shader.beginScope(Shader::Brackets::PARENTHESES);

    // Emit all shader uniforms
    for (const Shader::Variable& uniform : shader.getUniforms())
    {
        shader.beginLine();
        emitUniform(uniform, shader);
        shader.addStr(",");
        shader.endLine(false);
    }

    // Emit shader output
    const string type = _syntax->getOutputTypeName(outputSocket->type);
    const string variable = _syntax->getVariableName(outputSocket);
    const string value = _syntax->getTypeDefault(outputSocket->type, true);
    shader.addLine(type + " " + variable + " = " + value, false);

    shader.endScope();
}

} // namespace MaterialX
