#include <MaterialXShaderGen/ShaderGenerators/Osl/OslShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/Osl/OslSyntax.h>
#include <MaterialXShaderGen/ShaderGenerators/Common/Swizzle.h>
#include <MaterialXShaderGen/ShaderGenerators/Common/Switch.h>
#include <MaterialXShaderGen/ShaderGenerators/Common/Compare.h>

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

    // Color2/4 and Vector2/4 must be remapped to Color3 and Vector3 when used
    // as shader outputs since in OSL a custom struct type is not supported as 
    // shader output.
    const std::unordered_map<string, std::pair<string,string>> shaderOutputTypeRemap =
    {
        { "color2", { "color3","rg0" } },
        { "color4", { "color3","rgb" } },
        { "vector2", { "vector3","xy0" } },
        { "vector4", { "vector3","xyz" } }
    };
}


const string OslShaderGenerator::LANGUAGE = "sx-osl";

OslShaderGenerator::OslShaderGenerator()
    : ParentClass(OslSyntax::create())
{
    // Register build-in implementations

    // <!-- <compare> -->
    registerImplementation("IM_compare__float__sx_osl", Compare::create);
    registerImplementation("IM_compare__color2__sx_osl", Compare::create);
    registerImplementation("IM_compare__color3__sx_osl", Compare::create);
    registerImplementation("IM_compare__color4__sx_osl", Compare::create);
    registerImplementation("IM_compare__vector2__sx_osl", Compare::create);
    registerImplementation("IM_compare__vector3__sx_osl", Compare::create);
    registerImplementation("IM_compare__vector4__sx_osl", Compare::create);

    // <!-- <switch> -->
    registerImplementation("IM_switch__float__sx_osl", Switch::create);
    registerImplementation("IM_switch__color2__sx_osl", Switch::create);
    registerImplementation("IM_switch__color3__sx_osl", Switch::create);
    registerImplementation("IM_switch__color4__sx_osl", Switch::create);
    registerImplementation("IM_switch__vector2__sx_osl", Switch::create);
    registerImplementation("IM_switch__vector3__sx_osl", Switch::create);
    registerImplementation("IM_switch__vector4__sx_osl", Switch::create);

    // <!-- <swizzle> -->
    // <!-- from type : float -->
    registerImplementation("IM_swizzle__float_color2__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__float_color3__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__float_color4__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__float_vector2__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__float_vector3__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__float_vector4__sx_osl", Swizzle::create);
    // <!-- from type : color2 -->
    registerImplementation("IM_swizzle__color2_float__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__color2_color2__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__color2_color3__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__color2_color4__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__color2_vector2__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__color2_vector3__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__color2_vector4__sx_osl", Swizzle::create);
    // <!-- from type : color3 -->
    registerImplementation("IM_swizzle__color3_float__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__color3_color2__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__color3_color3__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__color3_color4__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__color3_vector2__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__color3_vector3__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__color3_vector4__sx_osl", Swizzle::create);
    // <!-- from type : color4 -->
    registerImplementation("IM_swizzle__color4_float__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__color4_color2__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__color4_color3__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__color4_color4__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__color4_vector2__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__color4_vector3__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__color4_vector4__sx_osl", Swizzle::create);
    // <!-- from type : vector2 -->
    registerImplementation("IM_swizzle__vector2_float__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__vector2_color2__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__vector2_color3__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__vector2_color4__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__vector2_vector2__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__vector2_vector3__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__vector2_vector4__sx_osl", Swizzle::create);
    // <!-- from type : vector3 -->
    registerImplementation("IM_swizzle__vector3_float__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__vector3_color2__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__vector3_color3__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__vector3_color4__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__vector3_vector2__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__vector3_vector3__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__vector3_vector4__sx_osl", Swizzle::create);
    // <!-- from type : vector4 -->
    registerImplementation("IM_swizzle__vector4_float__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__vector4_color2__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__vector4_color3__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__vector4_color4__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__vector4_vector2__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__vector4_vector3__sx_osl", Swizzle::create);
    registerImplementation("IM_swizzle__vector4_vector4__sx_osl", Swizzle::create);
}

ShaderPtr OslShaderGenerator::generate(const string& shaderName, ElementPtr element)
{
    ShaderPtr shaderPtr = std::make_shared<Shader>(shaderName);
    shaderPtr->initialize(element, *this);

    Shader& shader = *shaderPtr;

    emitIncludes(shader);
    emitTypeDefs(shader);
    emitFunctionDefinitions(shader);

    // Emit shader type
    const SgOutputSocket* outputSocket = shader.getNodeGraph()->getOutputSocket();
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

    // Emit all app data inputs
    const Shader::VariableBlock& appDataBlock = shader.getAppDataBlock();
    for (const Shader::Variable* input : appDataBlock.variableOrder)
    {
        const string& type = _syntax->getTypeName(input->type);
        const string value = _syntax->getTypeDefault(input->type, true);
        shader.addLine(type + " " + input->name + " = " + value + " [[ int lockgeom=0 ]],");
    }

    // Emit all public inputs
    const Shader::VariableBlock& publicUniforms = shader.getUniformBlock(Shader::PIXEL_STAGE, Shader::PUBLIC_UNIFORMS);
    for (const Shader::Variable* uniform : publicUniforms.variableOrder)
    {
        shader.beginLine();
        emitUniform(*uniform, shader);
        shader.addStr(",");
        shader.endLine(false);
    }

    // Emit shader output
    string outputType = outputSocket->type;
    auto it = shaderOutputTypeRemap.find(outputType);
    if (it != shaderOutputTypeRemap.end())
    {
        outputType = it->second.first;
    }
    const string type = _syntax->getOutputTypeName(outputType);
    const string value = _syntax->getTypeDefault(outputType, true);
    shader.addLine(type + " " + outputSocket->name + " = " + value, false);

    shader.endScope();

    // Emit shader body
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
    shader.addBlock(shader.getRequestedVDirection() != getTargetVDirection() ? VDIRECTION_FLIP : VDIRECTION_NOOP, *this);

    // Call parent to emit all other functions
    ParentClass::emitFunctionDefinitions(shader);
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
    ParentClass::emitFunctionCalls(shader);
}

void OslShaderGenerator::emitFinalOutput(Shader& shader) const
{
    SgNodeGraph* graph = shader.getNodeGraph();
    const SgOutputSocket* outputSocket = graph->getOutputSocket();

    if (!outputSocket->connection)
    {
        // Early out for the rare case where the whole graph is just a single value
        shader.addLine(outputSocket->name + " = " + (outputSocket->value ?
            _syntax->getValue(*outputSocket->value, outputSocket->type) : 
            _syntax->getTypeDefault(outputSocket->type)));
        return;
    }

    string finalResult = outputSocket->connection->name;

    // Handle channel swizzling
    if (outputSocket->channels != EMPTY_STRING)
    {
        string finalResultSwizz = finalResult + "_swizzled";
        finalResult = _syntax->getSwizzledVariable(finalResult, outputSocket->type, outputSocket->connection->type, outputSocket->channels);
        const string type = _syntax->getTypeName(outputSocket->type);
        shader.addLine(type + " " + finalResultSwizz + " = " + finalResult);
        finalResult = finalResultSwizz;
    }

    // Handle output type remapping
    auto it = shaderOutputTypeRemap.find(outputSocket->type);
    if (it != shaderOutputTypeRemap.end())
    {
        finalResult = _syntax->getSwizzledVariable(finalResult, it->second.first, outputSocket->type, it->second.second);
    }

    shader.addLine(outputSocket->name + " = " + finalResult);
}

} // namespace MaterialX
