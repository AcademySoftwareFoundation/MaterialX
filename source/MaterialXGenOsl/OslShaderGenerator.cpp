#include <MaterialXGenOsl/OslShaderGenerator.h>
#include <MaterialXGenOsl/OslSyntax.h>
#include <MaterialXGenShader/Nodes/SwizzleNode.h>
#include <MaterialXGenShader/Nodes/ConvertNode.h>
#include <MaterialXGenShader/Nodes/SwitchNode.h>
#include <MaterialXGenShader/Nodes/CompareNode.h>
#include <MaterialXGenShader/Nodes/BlurNode.h>

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


const string OslShaderGenerator::LANGUAGE = "sx-osl";

OslShaderGenerator::OslShaderGenerator()
    : ParentClass(OslSyntax::create())
    , _remapShaderOutput(false)
{
    // Register build-in implementations

    // <!-- <compare> -->
    registerImplementation("IM_compare_float_sx_osl", CompareNode::create);
    registerImplementation("IM_compare_color2_sx_osl", CompareNode::create);
    registerImplementation("IM_compare_color3_sx_osl", CompareNode::create);
    registerImplementation("IM_compare_color4_sx_osl", CompareNode::create);
    registerImplementation("IM_compare_vector2_sx_osl", CompareNode::create);
    registerImplementation("IM_compare_vector3_sx_osl", CompareNode::create);
    registerImplementation("IM_compare_vector4_sx_osl", CompareNode::create);

    // <!-- <switch> -->
    // <!-- 'which' type : float -->
    registerImplementation("IM_switch_float_sx_osl", SwitchNode::create);
    registerImplementation("IM_switch_color2_sx_osl", SwitchNode::create);
    registerImplementation("IM_switch_color3_sx_osl", SwitchNode::create);
    registerImplementation("IM_switch_color4_sx_osl", SwitchNode::create);
    registerImplementation("IM_switch_vector2_sx_osl", SwitchNode::create);
    registerImplementation("IM_switch_vector3_sx_osl", SwitchNode::create);
    registerImplementation("IM_switch_vector4_sx_osl", SwitchNode::create);
    // <!-- 'which' type : integer -->
    registerImplementation("IM_switch_floatI_sx_osl", SwitchNode::create);
    registerImplementation("IM_switch_color2I_sx_osl", SwitchNode::create);
    registerImplementation("IM_switch_color3I_sx_osl", SwitchNode::create);
    registerImplementation("IM_switch_color4I_sx_osl", SwitchNode::create);
    registerImplementation("IM_switch_vector2I_sx_osl", SwitchNode::create);
    registerImplementation("IM_switch_vector3I_sx_osl", SwitchNode::create);
    registerImplementation("IM_switch_vector4I_sx_osl", SwitchNode::create);
    // <!-- 'which' type : boolean -->
    registerImplementation("IM_switch_floatB_sx_osl", SwitchNode::create);
    registerImplementation("IM_switch_color2B_sx_osl", SwitchNode::create);
    registerImplementation("IM_switch_color3B_sx_osl", SwitchNode::create);
    registerImplementation("IM_switch_color4B_sx_osl", SwitchNode::create);
    registerImplementation("IM_switch_vector2B_sx_osl", SwitchNode::create);
    registerImplementation("IM_switch_vector3B_sx_osl", SwitchNode::create);
    registerImplementation("IM_switch_vector4B_sx_osl", SwitchNode::create);

    // <!-- <swizzle> -->
    // <!-- from type : float -->
    registerImplementation("IM_swizzle_float_color2_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_float_color3_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_float_color4_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_float_vector2_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_float_vector3_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_float_vector4_sx_osl", SwizzleNode::create);
    // <!-- from type : color2 -->
    registerImplementation("IM_swizzle_color2_float_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_color2_color2_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_color2_color3_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_color2_color4_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_color2_vector2_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_color2_vector3_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_color2_vector4_sx_osl", SwizzleNode::create);
    // <!-- from type : color3 -->
    registerImplementation("IM_swizzle_color3_float_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_color2_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_color3_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_color4_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_vector2_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_vector3_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_vector4_sx_osl", SwizzleNode::create);
    // <!-- from type : color4 -->
    registerImplementation("IM_swizzle_color4_float_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_color2_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_color3_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_color4_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_vector2_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_vector3_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_vector4_sx_osl", SwizzleNode::create);
    // <!-- from type : vector2 -->
    registerImplementation("IM_swizzle_vector2_float_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_color2_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_color3_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_color4_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_vector2_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_vector3_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_vector4_sx_osl", SwizzleNode::create);
    // <!-- from type : vector3 -->
    registerImplementation("IM_swizzle_vector3_float_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_color2_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_color3_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_color4_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_vector2_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_vector3_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_vector4_sx_osl", SwizzleNode::create);
    // <!-- from type : vector4 -->
    registerImplementation("IM_swizzle_vector4_float_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_color2_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_color3_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_color4_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_vector2_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_vector3_sx_osl", SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_vector4_sx_osl", SwizzleNode::create);

    // <!-- <convert> -->
    registerImplementation("IM_convert_float_color2_sx_osl", ConvertNode::create);
    registerImplementation("IM_convert_float_color3_sx_osl", ConvertNode::create);
    registerImplementation("IM_convert_float_color4_sx_osl", ConvertNode::create);
    registerImplementation("IM_convert_float_vector2_sx_osl", ConvertNode::create);
    registerImplementation("IM_convert_float_vector3_sx_osl", ConvertNode::create);
    registerImplementation("IM_convert_float_vector4_sx_osl", ConvertNode::create);
    registerImplementation("IM_convert_vector2_color2_sx_osl", ConvertNode::create);
    registerImplementation("IM_convert_vector3_color3_sx_osl", ConvertNode::create);
    registerImplementation("IM_convert_vector4_color4_sx_osl", ConvertNode::create);
    registerImplementation("IM_convert_color2_vector2_sx_osl", ConvertNode::create);
    registerImplementation("IM_convert_color3_vector3_sx_osl", ConvertNode::create);
    registerImplementation("IM_convert_color4_vector4_sx_osl", ConvertNode::create);
    registerImplementation("IM_convert_color3_color4_sx_osl", ConvertNode::create);
    registerImplementation("IM_convert_color4_color3_sx_osl", ConvertNode::create);
    registerImplementation("IM_convert_boolean_float_sx_osl", ConvertNode::create);
    registerImplementation("IM_convert_integer_float_sx_osl", ConvertNode::create);

    // <!-- <blur> -->
    registerImplementation("IM_blur_float_sx_osl", BlurNode::create);
    registerImplementation("IM_blur_color2_sx_osl", BlurNode::create);
    registerImplementation("IM_blur_color3_sx_osl", BlurNode::create);
    registerImplementation("IM_blur_color4_sx_osl", BlurNode::create);
    registerImplementation("IM_blur_vector2_sx_osl", BlurNode::create);
    registerImplementation("IM_blur_vector3_sx_osl", BlurNode::create);
    registerImplementation("IM_blur_vector4_sx_osl", BlurNode::create);

    // Color2/4 and Vector2/4 must be remapped to Color3 and Vector3 when used
    // as shader outputs since in OSL a custom struct type is not supported as
    // shader output.
    //
    // Note: this mapping is directly impacted by code that lives in TypeDesc::getChannelIndex(),
    // so if it changes also change this. (Or vice-versa).
    _shaderOutputTypeRemap =
    {
        { Type::COLOR2,  { Type::COLOR3, "rg0" } },
        { Type::COLOR4,  { Type::COLOR3, "rgb" } },
        { Type::VECTOR2, { Type::COLOR3, "xy0" } },
        { Type::VECTOR4, { Type::COLOR3, "xyz" } }
    };
}

ShaderPtr OslShaderGenerator::generate(const string& shaderName, ElementPtr element, const GenOptions& options)
{
    ShaderPtr shaderPtr = std::make_shared<Shader>(shaderName);
    shaderPtr->initialize(element, *this, options);

    Shader& shader = *shaderPtr;

    emitIncludes(shader);

    shader.addLine("#define M_FLOAT_EPS 0.000001", false);

    emitTypeDefs(shader);

    // Emit sampling code if needed
    if (shader.hasClassification(ShaderNode::Classification::CONVOLUTION2D))
    {
        // Emit sampling functions
        shader.addInclude("stdlib/sx-osl/lib/sx_sampling.osl", *this);
        shader.newLine();
    }

    emitFunctionDefinitions(shader);

    // Emit shader type
    const ShaderGraphOutputSocket* outputSocket = shader.getGraph()->getOutputSocket();
    if (outputSocket->type == Type::SURFACESHADER)
    {
        shader.addStr("surface ");
    }
    else if (outputSocket->type == Type::VOLUMESHADER)
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

    shader.addLine("float dummy = 0.0,", false);

    // Emit all app data inputs
    const Shader::VariableBlock& appDataBlock = shader.getAppDataBlock();
    for (const Shader::Variable* input : appDataBlock.variableOrder)
    {
        const string& type = _syntax->getTypeName(input->type);
        const string value = _syntax->getDefaultValue(input->type, true);
        shader.addLine(type + " " + input->name + " = " + value + " [[ int lockgeom=0 ]],", false);
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
    const TypeDesc* outputType = outputSocket->type;

    // Remap shader output as needed
    if (_remapShaderOutput)
    {
        auto it = _shaderOutputTypeRemap.find(outputType);
        if (it != _shaderOutputTypeRemap.end())
        {
            outputType = it->second.first;
        }
    }
    const string type = _syntax->getOutputTypeName(outputType);
    const string value = _syntax->getDefaultValue(outputType, true);
    shader.addLine(type + " " + outputSocket->variable + " = " + value, false);

    shader.endScope();

    // Emit shader body
    shader.beginScope(Shader::Brackets::BRACES);

    // Emit private constants. Must be within the shader body.
    const Shader::VariableBlock& psConstants = shader.getConstantBlock(Shader::PIXEL_STAGE);
    shader.addComment("Private Constants: ");
    for (const Shader::Variable* constant : psConstants.variableOrder)
    {
        shader.beginLine();
        emitConstant(*constant, shader);
        shader.endLine();
    }
    shader.newLine();

    emitFunctionCalls(*_defaultContext, shader);
    emitFinalOutput(shader);

    shader.endScope();

    return shaderPtr;
}

void OslShaderGenerator::emitFunctionDefinitions(Shader& shader)
{
    // Emit function for handling texture coords v-flip
    // as needed relative to the default v-direction 
    shader.addBlock(Shader::getDefaultVDirection() != getTargetVDirection() ? VDIRECTION_FLIP : VDIRECTION_NOOP, *this);

    // Call parent to emit all other functions
    ParentClass::emitFunctionDefinitions(shader);
}

void OslShaderGenerator::emitIncludes(Shader& shader)
{
    static const vector<string> includeFiles =
    {
        "color2.h",
        "color4.h",
        "matrix33.h",
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

void OslShaderGenerator::emitFunctionCalls(const GenContext& context, Shader &shader)
{
    // Emit needed globals
    if (!shader.getGraph()->hasClassification(ShaderNode::Classification::TEXTURE))
    {
        shader.addLine("closure color null_closure = 0");
    }

    // Call parent
    ParentClass::emitFunctionCalls(context, shader);
}

void OslShaderGenerator::emitFinalOutput(Shader& shader) const
{
    ShaderGraph* graph = shader.getGraph();
    const ShaderGraphOutputSocket* outputSocket = graph->getOutputSocket();

    if (!outputSocket->connection)
    {
        // Early out for the rare case where the whole graph is just a single value
        shader.addLine(outputSocket->variable + " = " + (outputSocket->value ?
            _syntax->getValue(outputSocket->type, *outputSocket->value) :
            _syntax->getDefaultValue(outputSocket->type)));
        return;
    }

    string finalResult = outputSocket->connection->variable;

    // Handle output type remapping as needed
    if (_remapShaderOutput)
    {
        auto it = _shaderOutputTypeRemap.find(outputSocket->type);
        if (it != _shaderOutputTypeRemap.end())
        {
            finalResult = _syntax->getSwizzledVariable(finalResult, outputSocket->type, it->second.second, it->second.first);
        }
    }

    shader.addLine(outputSocket->variable + " = " + finalResult);
}

void OslShaderGenerator::emitVariable(const Shader::Variable& uniform, const string& /*qualifier*/, Shader& shader)
{
    const string initStr = (uniform.value ? _syntax->getValue(uniform.type, *uniform.value, true) : _syntax->getDefaultValue(uniform.type, true));
    string line = _syntax->getTypeName(uniform.type) + " " + uniform.name;

    // If an arrays we need an array qualifier (suffix) for the variable name
    string arraySuffix;
    uniform.getArraySuffix(arraySuffix);
    line += arraySuffix;

    line += initStr.empty() ? "" : " = " + initStr;
    shader.addStr(line);
}

} // namespace MaterialX
