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
    registerImplementation("IM_convert_vector3_color3_" + OslShaderGenerator::LANGUAGE, ConvertNode::create);
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

ShaderPtr OslShaderGenerator::generate(const string& name, ElementPtr element, GenContext& context)
{
    ShaderPtr shader = createShader(name, element, context);

    const ShaderGraph& graph = shader->getGraph();
    ShaderStage& stage = shader->getStage(OSL::STAGE);

    emitIncludes(stage);

    // Add global constants and type definitions
    emitLine(stage, "#define M_FLOAT_EPS 0.000001", false);
    emitTypeDefinitions(stage);

    // Emit sampling code if needed
    if (graph.hasClassification(ShaderNode::Classification::CONVOLUTION2D))
    {
        // Emit sampling functions
        emitInclude(stage, "stdlib/" + OslShaderGenerator::LANGUAGE + "/lib/mx_sampling.osl");
        emitLineBreak(stage);
    }

    // Emit uv transform function
    if (context.getOptions().fileTextureVerticalFlip)
    {
        emitInclude(stage, "stdlib/" + OslShaderGenerator::LANGUAGE + "/lib/mx_get_target_uv_vflip.osl");
        emitLineBreak(stage);
    }
    else
    {
        emitInclude(stage, "stdlib/" + OslShaderGenerator::LANGUAGE + "/lib/mx_get_target_uv_noop.osl");
        emitLineBreak(stage);
    }

    // Emit function definitions for all nodes
    emitFunctionDefinitions(stage, graph, context);

    // Emit shader type
    const ShaderGraphOutputSocket* outputSocket = graph.getOutputSocket();
    if (outputSocket->type == Type::SURFACESHADER)
    {
        emitString(stage, "surface ");
    }
    else if (outputSocket->type == Type::VOLUMESHADER)
    {
        emitString(stage, "volume ");
    }
    else
    {
        emitString(stage, "shader ");
    }

    // Begin shader signature
    emitLine(stage, shader->getName(), false);
    emitScopeBegin(stage, ShaderStage::Brackets::PARENTHESES);
    emitLine(stage, "float dummy = 0.0,", false);

    // Emit all varying inputs
    const VariableBlock& inputs = stage.getInputBlock(OSL::INPUTS);
    for (size_t i=0; inputs.size(); ++i)
    {
        const Variable& input = inputs[i];
        const string& type = _syntax->getTypeName(input.getType());
        const string& value = _syntax->getDefaultValue(input.getType(), true);
        emitLine(stage, type + " " + input.getName() + " = " + value + " [[ int lockgeom=0 ]],", false);
    }

    // Emit all uniform inputs
    const VariableBlock& uniforms = stage.getUniformBlock(OSL::UNIFORMS);
    emitVariableBlock(stage, uniforms, _syntax->getUniformQualifier(), COMMA);

    // Emit shader output
    // TODO: Support multiple outputs
    const TypeDesc* outputType = outputSocket->type;
    const string type = _syntax->getOutputTypeName(outputType);
    const string value = _syntax->getDefaultValue(outputType, true);
    emitLine(stage, type + " " + outputSocket->variable + " = " + value, false);

    // End shader signature
    emitScopeEnd(stage);

    // Begin shader body
    emitScopeBegin(stage, ShaderStage::Brackets::BRACES);

    // Emit constants
    const VariableBlock& constants = stage.getConstantBlock();
    if (constants.size())
    {
        emitVariableBlock(stage, constants, _syntax->getConstantQualifier(), SEMICOLON);
        emitLineBreak(stage);
    }

    // Emit function calls for all nodes
    emitFunctionCalls(stage, graph, context);

    // Emit final output
    if (outputSocket->connection)
    {
        string finalResult = outputSocket->connection->variable;
        emitLine(stage, outputSocket->variable + " = " + finalResult);
    }
    else
    {
        emitLine(stage, outputSocket->variable + " = " + (outputSocket->value ?
            _syntax->getValue(outputSocket->type, *outputSocket->value) :
            _syntax->getDefaultValue(outputSocket->type)));
    }

    // End shader body
    emitScopeEnd(stage);

    return shader;
}

ShaderPtr OslShaderGenerator::createShader(const string& name, ElementPtr element, GenContext& context)
{
    // Create the root shader graph
    ShaderGraphPtr graph = ShaderGraph::create(nullptr, name, element, *this, context);
    ShaderPtr shader = std::make_shared<Shader>(name, graph);

    // Create our stage.
    ShaderStagePtr stage = createStage(*shader, OSL::STAGE);
    stage->createUniformBlock(OSL::UNIFORMS);
    stage->createInputBlock(OSL::INPUTS);
    stage->createOutputBlock(OSL::OUTPUTS);

    // Create shader variables for all nodes that need this.
    for (ShaderNode* node : graph->getNodes())
    {
        node->getImplementation().createVariables(*shader, *node, *this, context);
    }

    // Create uniforms for the published graph interface.
    VariableBlock& uniforms = stage->getUniformBlock(OSL::UNIFORMS);
    for (ShaderGraphInputSocket* inputSocket : graph->getInputSockets())
    {
        // Only for inputs that are connected/used internally,
        // and are editable by users.
        if (inputSocket->connections.size() && graph->isEditable(*inputSocket))
        {
            uniforms.add(inputSocket->type, inputSocket->variable, EMPTY_STRING, inputSocket->value, inputSocket->path);
        }
    }

    // Create outputs from the graph interface.
    VariableBlock& outputs = stage->getOutputBlock(OSL::OUTPUTS);
    for (ShaderGraphOutputSocket* outputSocket : graph->getOutputSockets())
    {
        outputs.add(outputSocket->type, outputSocket->name);
    }

    return shader;
}

void OslShaderGenerator::emitFunctionCalls(ShaderStage& stage, const ShaderGraph& graph, GenContext& context)
{
    if (!graph.hasClassification(ShaderNode::Classification::TEXTURE))
    {
        emitLine(stage, "closure color null_closure = 0");
    }
    ShaderGenerator::emitFunctionCalls(stage, graph, context);
}

void OslShaderGenerator::emitIncludes(ShaderStage& stage)
{
    static const string INCLUDE_PREFIX = "#include \"";
    static const string INCLUDE_SUFFIX = "\"";
    static const vector<string> INCLUDE_FILES =
    {
        "color2.h",
        "color4.h",
        "matrix33.h",
        "vector2.h",
        "vector4.h",
        "mx_funcs.h"
    };

    for (const string& file : INCLUDE_FILES)
    {
        FilePath path = findSourceCode(file);
        emitLine(stage, INCLUDE_PREFIX + path.asString() + INCLUDE_SUFFIX, false);
    }

    emitLineBreak(stage);
}

namespace OSL
{
    // Identifiers for OSL stage and variable blocks
    const string STAGE    = MAIN_STAGE;
    const string UNIFORMS = "u";
    const string INPUTS   = "i";
    const string OUTPUTS  = "o";
}

} // namespace MaterialX
