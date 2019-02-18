#include <MaterialXGenOsl/OslShaderGenerator.h>
#include <MaterialXGenOsl/OslSyntax.h>
#include <MaterialXGenShader/Nodes/SwizzleNode.h>
#include <MaterialXGenShader/Nodes/ConvertNode.h>
#include <MaterialXGenShader/Nodes/CombineNode.h>
#include <MaterialXGenShader/Nodes/SwitchNode.h>
#include <MaterialXGenShader/Nodes/CompareNode.h>
#include <MaterialXGenShader/Nodes/BlurNode.h>

namespace MaterialX
{
const string OslShaderGenerator::LANGUAGE = "genosl";
const string OslShaderGenerator::TARGET = "vanilla";

OslShaderGenerator::OslShaderGenerator()
    : ParentClass(OslSyntax::create())
{
    // Register build-in implementations

    // <!-- <compare> -->
    registerImplementation("IM_compare_float_" + OslShaderGenerator::LANGUAGE, CompareNode::create);
    registerImplementation("IM_compare_color2_" + OslShaderGenerator::LANGUAGE, CompareNode::create);
    registerImplementation("IM_compare_color3_" + OslShaderGenerator::LANGUAGE, CompareNode::create);
    registerImplementation("IM_compare_color4_" + OslShaderGenerator::LANGUAGE, CompareNode::create);
    registerImplementation("IM_compare_vector2_" + OslShaderGenerator::LANGUAGE, CompareNode::create);
    registerImplementation("IM_compare_vector3_" + OslShaderGenerator::LANGUAGE, CompareNode::create);
    registerImplementation("IM_compare_vector4_" + OslShaderGenerator::LANGUAGE, CompareNode::create);

    // <!-- <switch> -->
    // <!-- 'which' type : float -->
    registerImplementation("IM_switch_float_" + OslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_color2_" + OslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_color3_" + OslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_color4_" + OslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_vector2_" + OslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_vector3_" + OslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_vector4_" + OslShaderGenerator::LANGUAGE, SwitchNode::create);
    // <!-- 'which' type : integer -->
    registerImplementation("IM_switch_floatI_" + OslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_color2I_" + OslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_color3I_" + OslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_color4I_" + OslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_vector2I_" + OslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_vector3I_" + OslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_vector4I_" + OslShaderGenerator::LANGUAGE, SwitchNode::create);
    // <!-- 'which' type : boolean -->
    registerImplementation("IM_switch_floatB_" + OslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_color2B_" + OslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_color3B_" + OslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_color4B_" + OslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_vector2B_" + OslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_vector3B_" + OslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_vector4B_" + OslShaderGenerator::LANGUAGE, SwitchNode::create);

    // <!-- <swizzle> -->
    // <!-- from type : float -->
    registerImplementation("IM_swizzle_float_color2_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_float_color3_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_float_color4_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_float_vector2_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_float_vector3_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_float_vector4_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    // <!-- from type : color2 -->
    registerImplementation("IM_swizzle_color2_float_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color2_color2_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color2_color3_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color2_color4_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color2_vector2_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color2_vector3_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color2_vector4_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    // <!-- from type : color3 -->
    registerImplementation("IM_swizzle_color3_float_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_color2_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_color3_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_color4_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_vector2_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_vector3_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_vector4_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    // <!-- from type : color4 -->
    registerImplementation("IM_swizzle_color4_float_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_color2_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_color3_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_color4_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_vector2_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_vector3_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_vector4_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    // <!-- from type : vector2 -->
    registerImplementation("IM_swizzle_vector2_float_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_color2_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_color3_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_color4_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_vector2_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_vector3_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_vector4_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    // <!-- from type : vector3 -->
    registerImplementation("IM_swizzle_vector3_float_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_color2_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_color3_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_color4_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_vector2_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_vector3_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_vector4_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    // <!-- from type : vector4 -->
    registerImplementation("IM_swizzle_vector4_float_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_color2_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_color3_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_color4_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_vector2_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_vector3_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_vector4_" + OslShaderGenerator::LANGUAGE, SwizzleNode::create);

    // <!-- <convert> -->
    registerImplementation("IM_convert_float_color2_" + OslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_float_color3_" + OslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_float_color4_" + OslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_float_vector2_" + OslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_float_vector3_" + OslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_float_vector4_" + OslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_vector2_color2_" + OslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_vector2_vector3_" + OslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_vector3_vector2_" + OslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_vector3_color3_" + OslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_vector3_vector4_" + OslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_vector4_vector3_" + OslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_vector4_color4_" + OslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_color2_vector2_" + OslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_color3_vector3_" + OslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_color4_vector4_" + OslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_color3_color4_" + OslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_color4_color3_" + OslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_boolean_float_" + OslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_integer_float_" + OslShaderGenerator::LANGUAGE, ConvertNode::create);

    // <!-- <combine> -->
    registerImplementation("IM_combine_color2_" + OslShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine_vector2_" + OslShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine_color3_" + OslShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine_vector3_" + OslShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine_color4_" + OslShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine_vector4_" + OslShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine_color4CF_" + OslShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine_vector4VF_" + OslShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine_color4CC_" + OslShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine_vector4VV_" + OslShaderGenerator::LANGUAGE, CombineNode::create);

    // <!-- <blur> -->
    registerImplementation("IM_blur_float_" + OslShaderGenerator::LANGUAGE, BlurNode::create);
    registerImplementation("IM_blur_color2_" + OslShaderGenerator::LANGUAGE, BlurNode::create);
    registerImplementation("IM_blur_color3_" + OslShaderGenerator::LANGUAGE, BlurNode::create);
    registerImplementation("IM_blur_color4_" + OslShaderGenerator::LANGUAGE, BlurNode::create);
    registerImplementation("IM_blur_vector2_" + OslShaderGenerator::LANGUAGE, BlurNode::create);
    registerImplementation("IM_blur_vector3_" + OslShaderGenerator::LANGUAGE, BlurNode::create);
    registerImplementation("IM_blur_vector4_" + OslShaderGenerator::LANGUAGE, BlurNode::create);
}

ShaderPtr OslShaderGenerator::generate(const string& shaderName, ElementPtr element, const GenOptions& options)
{
    ShaderPtr shaderPtr = std::make_shared<Shader>(shaderName);
    shaderPtr->initialize(element, *this, options);

    Shader& shader = *shaderPtr;

    emitIncludes(shader);

    // Add global constants and type definitions
    shader.addLine("#define M_FLOAT_EPS 0.000001", false);
    emitTypeDefinitions(shader);

    // Emit sampling code if needed
    if (shader.hasClassification(ShaderNode::Classification::CONVOLUTION2D))
    {
        // Emit sampling functions
        shader.addInclude("stdlib/" + OslShaderGenerator::LANGUAGE + "/lib/mx_sampling.osl", *this);
        shader.newLine();
    }

    // Emit uv transform function
    if (options.fileTextureVerticalFlip)
    {
        shader.addInclude("stdlib/" + OslShaderGenerator::LANGUAGE + "/lib/mx_get_target_uv_vflip.osl", *this);
        shader.newLine();
    }
    else
    {
        shader.addInclude("stdlib/" + OslShaderGenerator::LANGUAGE + "/lib/mx_get_target_uv_noop.osl", *this);
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
    emitVariableBlock(publicUniforms, _syntax->getUniformQualifier(), COMMA, shader);

    // Emit shader output
    const TypeDesc* outputType = outputSocket->type;
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
        emitVariable(*constant, _syntax->getConstantQualifier(), shader);
        shader.endLine();
    }
    shader.newLine();

    emitFunctionCalls(*_defaultContext, shader);
    emitFinalOutput(shader);

    shader.endScope();

    return shaderPtr;
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
