#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenGlsl/GlslSyntax.h>
#include <MaterialXGenGlsl/Nodes/PositionNodeGlsl.h>
#include <MaterialXGenGlsl/Nodes/NormalNodeGlsl.h>
#include <MaterialXGenGlsl/Nodes/TangentNodeGlsl.h>
#include <MaterialXGenGlsl/Nodes/BitangentNodeGlsl.h>
#include <MaterialXGenGlsl/Nodes/TexCoordNodeGlsl.h>
#include <MaterialXGenGlsl/Nodes/GeomColorNodeGlsl.h>
#include <MaterialXGenGlsl/Nodes/GeomAttrValueNodeGlsl.h>
#include <MaterialXGenGlsl/Nodes/FrameNodeGlsl.h>
#include <MaterialXGenGlsl/Nodes/TimeNodeGlsl.h>
#include <MaterialXGenGlsl/Nodes/ViewDirectionNodeGlsl.h>
#include <MaterialXGenGlsl/Nodes/SurfaceNodeGlsl.h>
#include <MaterialXGenGlsl/Nodes/SurfaceShaderNodeGlsl.h>
#include <MaterialXGenGlsl/Nodes/LightNodeGlsl.h>
#include <MaterialXGenGlsl/Nodes/LightCompoundNodeGlsl.h>
#include <MaterialXGenGlsl/Nodes/LightShaderNodeGlsl.h>
#include <MaterialXGenGlsl/Nodes/HeightToNormalNodeGlsl.h>
#include <MaterialXGenGlsl/Nodes/LightSamplerNodeGlsl.h>
#include <MaterialXGenGlsl/Nodes/NumLightsNodeGlsl.h>

#include <MaterialXGenShader/Nodes/SourceCodeNode.h>
#include <MaterialXGenShader/Nodes/SwizzleNode.h>
#include <MaterialXGenShader/Nodes/ConvertNode.h>
#include <MaterialXGenShader/Nodes/CombineNode.h>
#include <MaterialXGenShader/Nodes/SwitchNode.h>
#include <MaterialXGenShader/Nodes/CompareNode.h>
#include <MaterialXGenShader/Nodes/BlurNode.h>

namespace MaterialX
{
const string GlslShaderGenerator::LANGUAGE = "genglsl";
const string GlslShaderGenerator::TARGET = "glsl400";
const string GlslShaderGenerator::VERSION = "400";

GlslShaderGenerator::GlslShaderGenerator()
    : HwShaderGenerator(GlslSyntax::create())
{
    //
    // Register all custom node implementation classes
    //

    // <!-- <compare> -->
    registerImplementation("IM_compare_float_" + GlslShaderGenerator::LANGUAGE, CompareNode::create);
    registerImplementation("IM_compare_color2_" + GlslShaderGenerator::LANGUAGE, CompareNode::create);
    registerImplementation("IM_compare_color3_" + GlslShaderGenerator::LANGUAGE, CompareNode::create);
    registerImplementation("IM_compare_color4_" + GlslShaderGenerator::LANGUAGE, CompareNode::create);
    registerImplementation("IM_compare_vector2_" + GlslShaderGenerator::LANGUAGE, CompareNode::create);
    registerImplementation("IM_compare_vector3_" + GlslShaderGenerator::LANGUAGE, CompareNode::create);
    registerImplementation("IM_compare_vector4_" + GlslShaderGenerator::LANGUAGE, CompareNode::create);

    // <!-- <switch> -->
    // <!-- 'which' type : float -->
    registerImplementation("IM_switch_float_" + GlslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_color2_" + GlslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_color3_" + GlslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_color4_" + GlslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_vector2_" + GlslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_vector3_" + GlslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_vector4_" + GlslShaderGenerator::LANGUAGE, SwitchNode::create);
    // <!-- 'which' type : integer -->
    registerImplementation("IM_switch_floatI_" + GlslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_color2I_" + GlslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_color3I_" + GlslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_color4I_" + GlslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_vector2I_" + GlslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_vector3I_" + GlslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_vector4I_" + GlslShaderGenerator::LANGUAGE, SwitchNode::create);
    // <!-- 'which' type : boolean -->
    registerImplementation("IM_switch_floatB_" + GlslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_color2B_" + GlslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_color3B_" + GlslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_color4B_" + GlslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_vector2B_" + GlslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_vector3B_" + GlslShaderGenerator::LANGUAGE, SwitchNode::create);
    registerImplementation("IM_switch_vector4B_" + GlslShaderGenerator::LANGUAGE, SwitchNode::create);

    // <!-- <swizzle> -->
    // <!-- from type : float -->
    registerImplementation("IM_swizzle_float_color2_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_float_color3_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_float_color4_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_float_vector2_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_float_vector3_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_float_vector4_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    // <!-- from type : color2 -->
    registerImplementation("IM_swizzle_color2_float_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color2_color2_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color2_color3_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color2_color4_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color2_vector2_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color2_vector3_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color2_vector4_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    // <!-- from type : color3 -->
    registerImplementation("IM_swizzle_color3_float_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_color2_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_color3_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_color4_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_vector2_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_vector3_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_vector4_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    // <!-- from type : color4 -->
    registerImplementation("IM_swizzle_color4_float_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_color2_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_color3_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_color4_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_vector2_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_vector3_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_vector4_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    // <!-- from type : vector2 -->
    registerImplementation("IM_swizzle_vector2_float_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_color2_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_color3_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_color4_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_vector2_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_vector3_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_vector4_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    // <!-- from type : vector3 -->
    registerImplementation("IM_swizzle_vector3_float_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_color2_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_color3_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_color4_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_vector2_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_vector3_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_vector4_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    // <!-- from type : vector4 -->
    registerImplementation("IM_swizzle_vector4_float_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_color2_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_color3_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_color4_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_vector2_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_vector3_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_vector4_" + GlslShaderGenerator::LANGUAGE, SwizzleNode::create);

    // <!-- <convert> -->
    registerImplementation("IM_convert_float_color2_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_float_color3_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_float_color4_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_float_vector2_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_float_vector3_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_float_vector4_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_vector2_color2_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_vector3_color3_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_vector4_color4_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_color2_vector2_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_color3_vector3_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_color4_vector4_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_color3_color4_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_color4_color3_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_boolean_float_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_integer_float_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);

    // <!-- <combine> -->
    registerImplementation("IM_combine_color2_" + GlslShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine_vector2_" + GlslShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine_color3_" + GlslShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine_vector3_" + GlslShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine_color4_" + GlslShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine_vector4_" + GlslShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine_color4CF_" + GlslShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine_vector4VF_" + GlslShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine_color4CC_" + GlslShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine_vector4VV_" + GlslShaderGenerator::LANGUAGE, CombineNode::create);

    // <!-- <position> -->
    registerImplementation("IM_position_vector3_" + GlslShaderGenerator::LANGUAGE, PositionNodeGlsl::create);
    // <!-- <normal> -->
    registerImplementation("IM_normal_vector3_" + GlslShaderGenerator::LANGUAGE, NormalNodeGlsl::create);
    // <!-- <tangent> -->
    registerImplementation("IM_tangent_vector3_" + GlslShaderGenerator::LANGUAGE, TangentNodeGlsl::create);
    // <!-- <bitangent> -->
    registerImplementation("IM_bitangent_vector3_" + GlslShaderGenerator::LANGUAGE, BitangentNodeGlsl::create);
    // <!-- <texcoord> -->
    registerImplementation("IM_texcoord_vector2_" + GlslShaderGenerator::LANGUAGE, TexCoordNodeGlsl::create);
    registerImplementation("IM_texcoord_vector3_" + GlslShaderGenerator::LANGUAGE, TexCoordNodeGlsl::create);
    // <!-- <geomcolor> -->
    registerImplementation("IM_geomcolor_float_" + GlslShaderGenerator::LANGUAGE, GeomColorNodeGlsl::create);
    registerImplementation("IM_geomcolor_color2_" + GlslShaderGenerator::LANGUAGE, GeomColorNodeGlsl::create);
    registerImplementation("IM_geomcolor_color3_" + GlslShaderGenerator::LANGUAGE, GeomColorNodeGlsl::create);
    registerImplementation("IM_geomcolor_color4_" + GlslShaderGenerator::LANGUAGE, GeomColorNodeGlsl::create);
    // <!-- <geomattrvalue> -->
    registerImplementation("IM_geomattrvalue_integer_" + GlslShaderGenerator::LANGUAGE, GeomAttrValueNodeGlsl::create);
    registerImplementation("IM_geomattrvalue_boolean_" + GlslShaderGenerator::LANGUAGE, GeomAttrValueNodeGlsl::create);
    registerImplementation("IM_geomattrvalue_string_" + GlslShaderGenerator::LANGUAGE, GeomAttrValueNodeGlsl::create);
    registerImplementation("IM_geomattrvalue_float_" + GlslShaderGenerator::LANGUAGE, GeomAttrValueNodeGlsl::create);
    registerImplementation("IM_geomattrvalue_color2_" + GlslShaderGenerator::LANGUAGE, GeomAttrValueNodeGlsl::create);
    registerImplementation("IM_geomattrvalue_color3_" + GlslShaderGenerator::LANGUAGE, GeomAttrValueNodeGlsl::create);
    registerImplementation("IM_geomattrvalue_color4_" + GlslShaderGenerator::LANGUAGE, GeomAttrValueNodeGlsl::create);
    registerImplementation("IM_geomattrvalue_vector2_" + GlslShaderGenerator::LANGUAGE, GeomAttrValueNodeGlsl::create);
    registerImplementation("IM_geomattrvalue_vector3_" + GlslShaderGenerator::LANGUAGE, GeomAttrValueNodeGlsl::create);
    registerImplementation("IM_geomattrvalue_vector4_" + GlslShaderGenerator::LANGUAGE, GeomAttrValueNodeGlsl::create);

    // <!-- <frame> -->
    registerImplementation("IM_frame_float_" + GlslShaderGenerator::LANGUAGE, FrameNodeGlsl::create);
    // <!-- <time> -->
    registerImplementation("IM_time_float_" + GlslShaderGenerator::LANGUAGE, TimeNodeGlsl::create);
    // <!-- <viewdirection> -->
    registerImplementation("IM_viewdirection_vector3_" + GlslShaderGenerator::LANGUAGE, ViewDirectionNodeGlsl::create);

    // <!-- <surface> -->
    registerImplementation("IM_surface_" + GlslShaderGenerator::LANGUAGE, SurfaceNodeGlsl::create);
    // <!-- <light> -->
    registerImplementation("IM_light_" + GlslShaderGenerator::LANGUAGE, LightNodeGlsl::create);

    // <!-- <pointlight> -->
    registerImplementation("IM_pointlight_" + GlslShaderGenerator::LANGUAGE, LightShaderNodeGlsl::create);
    // <!-- <directionallight> -->
    registerImplementation("IM_directionallight_" + GlslShaderGenerator::LANGUAGE, LightShaderNodeGlsl::create);
    // <!-- <spotlight> -->
    registerImplementation("IM_spotlight_" + GlslShaderGenerator::LANGUAGE, LightShaderNodeGlsl::create);

    // <!-- <heighttonormal> -->
    registerImplementation("IM_heighttonormal_vector3_" + GlslShaderGenerator::LANGUAGE, HeightToNormalNodeGlsl::create);

    // <!-- <blur> -->
    registerImplementation("IM_blur_float_" + GlslShaderGenerator::LANGUAGE, BlurNode::create);
    registerImplementation("IM_blur_color2_" + GlslShaderGenerator::LANGUAGE, BlurNode::create);
    registerImplementation("IM_blur_color3_" + GlslShaderGenerator::LANGUAGE, BlurNode::create);
    registerImplementation("IM_blur_color4_" + GlslShaderGenerator::LANGUAGE, BlurNode::create);
    registerImplementation("IM_blur_vector2_" + GlslShaderGenerator::LANGUAGE, BlurNode::create);
    registerImplementation("IM_blur_vector3_" + GlslShaderGenerator::LANGUAGE, BlurNode::create);
    registerImplementation("IM_blur_vector4_" + GlslShaderGenerator::LANGUAGE, BlurNode::create);

    _lightSamplingNodes.push_back(ShaderNode::create(nullptr, "numActiveLightSources", NumLightsNodeGlsl::create()));
    _lightSamplingNodes.push_back(ShaderNode::create(nullptr, "sampleLightSource", LightSamplerNodeGlsl::create()));
}

ShaderPtr GlslShaderGenerator::generate(const string& name, ElementPtr element, GenContext& context)
{
    ShaderPtr shader = createShader(name, element, context);

    // Turn on fixed float formatting to make sure float values are
    // emitted with a decimal point and not as integers, and to avoid
    // any scientific notation which isn't supported by all OpenGL targets.
    Value::ScopedFloatFormatting fmt(Value::FloatFormatFixed);

    // Emit code for vertex shader stage
    ShaderStage& vs = shader->getStage(HW::VERTEX_STAGE);
    emitVertexStage(vs, shader->getGraph(), context);

    // Emit code for pixel shader stage
    ShaderStage& ps = shader->getStage(HW::PIXEL_STAGE);
    emitPixelStage(ps, shader->getGraph(), context);

    return shader;
}

void GlslShaderGenerator::emitVertexStage(ShaderStage& stage, const ShaderGraph& graph, GenContext& context)
{
    // Add version directive
    emitLine(stage, "#version " + getVersion(), false);
    emitLineBreak(stage);

    // Add all constants
    const VariableBlock& constants = stage.getConstantBlock();
    if (!constants.empty())
    {
        emitVariableBlock(stage, constants, _syntax->getConstantQualifier(), SEMICOLON);
        emitLineBreak(stage);
    }

    // Add all uniforms
    for (auto it : stage.getUniformBlocks())
    {
        const VariableBlock& uniforms = *it.second;
        if (!uniforms.empty())
        {
            emitComment(stage, "Uniform block: " + uniforms.getName());
            emitVariableBlock(stage, uniforms, _syntax->getUniformQualifier(), SEMICOLON);
            emitLineBreak(stage);
        }
    }

    // Add vertex inputs
    const VariableBlock& vertexInputs = stage.getInputBlock(HW::VERTEX_INPUTS);
    if (!vertexInputs.empty())
    {
        emitComment(stage, "Inputs block: " + vertexInputs.getName());
        emitVariableBlock(stage, vertexInputs, _syntax->getInputQualifier(), SEMICOLON, false);
        emitLineBreak(stage);
    }

    // Add vertex data outputs block
    const VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
    if (!vertexData.empty())
    {
        emitLine(stage, "out " + vertexData.getName(), false);
        emitScopeBegin(stage, ShaderStage::Brackets::BRACES);
        emitVariableBlock(stage, vertexData, EMPTY_STRING, SEMICOLON, false);
        emitScopeEnd(stage, false, false);
        emitString(stage, " " + vertexData.getInstance() + SEMICOLON);
        emitLineBreak(stage);
        emitLineBreak(stage);
    }

    emitFunctionDefinitions(stage, graph, context);

    // Add main function
    emitLine(stage, "void main()", false);
    emitScopeBegin(stage, ShaderStage::Brackets::BRACES);
    emitLine(stage, "vec4 hPositionWorld = u_worldMatrix * vec4(i_position, 1.0)");
    emitLine(stage, "gl_Position = u_viewProjectionMatrix * hPositionWorld");
    emitFunctionCalls(stage, graph, context);
    emitScopeEnd(stage);
    emitLineBreak(stage);
}

void GlslShaderGenerator::emitPixelStage(ShaderStage& stage, const ShaderGraph& graph, GenContext& context)
{
    // Add version directive
    emitLine(stage, "#version " + getVersion(), false);
    emitLineBreak(stage);

    // Add global constants and type definitions
    emitInclude(stage, "pbrlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_defines.glsl");
    emitLine(stage, "#define MAX_LIGHT_SOURCES " + std::to_string(getMaxActiveLightSources()), false);
    emitLineBreak(stage);
    emitTypeDefinitions(stage);

    // Add all constants
    const VariableBlock& constants = stage.getConstantBlock();
    if (!constants.empty())
    {
        emitVariableBlock(stage, constants, _syntax->getConstantQualifier(), SEMICOLON);
        emitLineBreak(stage);
    }

    // Add all uniforms
    for (auto it : stage.getUniformBlocks())
    {
        const VariableBlock& uniforms = *it.second;

        // Skip light uniforms as they are handled separately
        if (!uniforms.empty() && uniforms.getName() != HW::LIGHT_DATA)
        {
            emitComment(stage, "Uniform block: " + uniforms.getName());
            emitVariableBlock(stage, uniforms, _syntax->getUniformQualifier(), SEMICOLON);
            emitLineBreak(stage);
        }
    }

    bool lighting = graph.hasClassification(ShaderNode::Classification::SHADER|ShaderNode::Classification::SURFACE) ||
                    graph.hasClassification(ShaderNode::Classification::BSDF);

    // Add light data block if needed
    if (lighting)
    {
        const VariableBlock& lightData = stage.getUniformBlock(HW::LIGHT_DATA);
        emitLine(stage, "struct " + lightData.getName(), false);
        emitScopeBegin(stage, ShaderStage::Brackets::BRACES);
        emitVariableBlock(stage, lightData, EMPTY_STRING, SEMICOLON, false);
        emitScopeEnd(stage, true);
        emitLineBreak(stage);
        emitLine(stage, "uniform " + lightData.getName() + " " + lightData.getInstance() + "[MAX_LIGHT_SOURCES]");
        emitLineBreak(stage);
    }

    // Add vertex data inputs block
    const VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
    if (!vertexData.empty())
    {
        emitLine(stage, "in " + vertexData.getName(), false);
        emitScopeBegin(stage, ShaderStage::Brackets::BRACES);
        emitVariableBlock(stage, vertexData, EMPTY_STRING, SEMICOLON, false);
        emitScopeEnd(stage, false, false);
        emitString(stage, " " + vertexData.getInstance() + SEMICOLON);
        emitLineBreak(stage);
        emitLineBreak(stage);
    }

    // Add the pixel shader output. This needs to be a vec4 for rendering
    // and upstream connection will be converted to vec4 if needed in emitFinalOutput()
    emitComment(stage, "Pixel shader outputs");
    const VariableBlock& outputs = stage.getOutputBlock(HW::PIXEL_OUTPUTS);
    emitVariableBlock(stage, outputs, _syntax->getOutputQualifier(), SEMICOLON, false);
    emitLineBreak(stage);

    // Emit common math functions
    emitInclude(stage, "pbrlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_math.glsl");
    emitLineBreak(stage);

    // Emit lighting functions
    if (lighting)
    {
        if (context.getOptions().hwSpecularEnvironmentMethod == SPECULAR_ENVIRONMENT_FIS)
        {
            emitInclude(stage, "pbrlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_environment_fis.glsl");
        }
        else
        {
            emitInclude(stage, "pbrlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_environment_prefilter.glsl");
        }
        emitLineBreak(stage);
    }

    // Emit sampling code if needed
    if (graph.hasClassification(ShaderNode::Classification::CONVOLUTION2D))
    {
        // Emit sampling functions
        emitInclude(stage, "stdlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_sampling.glsl");
        emitLineBreak(stage);
    }

    // Emit uv transform function
    if (context.getOptions().fileTextureVerticalFlip)
    {
        emitInclude(stage, "stdlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_get_target_uv_vflip.glsl");
        emitLineBreak(stage);
    }
    else
    {
        emitInclude(stage, "stdlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_get_target_uv_noop.glsl");
        emitLineBreak(stage);
    }

    // Add all functions for node implementations
    emitFunctionDefinitions(stage, graph, context);

    const ShaderGraphOutputSocket* outputSocket = graph.getOutputSocket();

    // Add main function
    emitLine(stage, "void main()", false);
    emitScopeBegin(stage, ShaderStage::Brackets::BRACES);

    if (graph.hasClassification(ShaderNode::Classification::CLOSURE))
    {
        // Handle the case where the graph is a direct closure.
        // We don't support rendering closures without attaching 
        // to a surface shader, so just output black.
        emitLine(stage, outputSocket->variable + " = vec4(0.0, 0.0, 0.0, 1.0)");
    }
    else
    {
        // Add all function calls
        emitFunctionCalls(stage, graph, context);

        // Emit final output
        if (outputSocket->connection)
        {
            string finalOutput = outputSocket->connection->variable;

            if (graph.hasClassification(ShaderNode::Classification::SURFACE))
            {
                if (context.getOptions().hwTransparency)
                {
                    emitLine(stage, "float outAlpha = clamp(1.0 - dot(" + finalOutput + ".transparency, vec3(0.3333)), 0.0, 1.0)");
                    emitLine(stage, outputSocket->variable + " = vec4(" + finalOutput + ".color, outAlpha)");
                }
                else
                {
                    emitLine(stage, outputSocket->variable + " = vec4(" + finalOutput + ".color, 1.0)");
                }
            }
            else
            {
                if (!outputSocket->type->isFloat4())
                {
                    toVec4(outputSocket->type, finalOutput);
                }
                emitLine(stage, outputSocket->variable + " = " + finalOutput);
            }
        }
        else
        {
            string outputValue = outputSocket->value ? _syntax->getValue(outputSocket->type, *outputSocket->value) : _syntax->getDefaultValue(outputSocket->type);
            if (!outputSocket->type->isFloat4())
            {
                string finalOutput = outputSocket->variable + "_tmp";
                emitLine(stage, _syntax->getTypeName(outputSocket->type) + " " + finalOutput + " = " + outputValue);
                toVec4(outputSocket->type, finalOutput);
                emitLine(stage, outputSocket->variable + " = " + finalOutput);
            }
            else
            {
                emitLine(stage, outputSocket->variable + " = " + outputValue);
            }
        }
    }

    // End main function
    emitScopeEnd(stage);
    emitLineBreak(stage);
}

void GlslShaderGenerator::emitFunctionDefinitions(ShaderStage& stage, const ShaderGraph& graph, GenContext& context)
{
BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)

    // For surface shaders we need light shaders
    if (graph.hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE))
    {
        // Emit functions for all bound light shaders
        for (auto it : _boundLightShaders)
        {
            emitFunctionDefinition(stage, *it.second, context);
        }
        // Emit functions for light sampling
        for (auto it : _lightSamplingNodes)
        {
            emitFunctionDefinition(stage, *it, context);
        }
    }
END_SHADER_STAGE(stage, HW::PIXEL_STAGE)

    // Call parent to emit all other functions
    HwShaderGenerator::emitFunctionDefinitions(stage, graph, context);
}

void GlslShaderGenerator::emitFunctionCalls(ShaderStage& stage, const ShaderGraph& graph, GenContext& context)
{
BEGIN_SHADER_STAGE(stage, HW::VERTEX_STAGE)
    // For vertex stage just emit all function calls in order
    // and ignore conditional scope.
    for (const ShaderNode* node : graph.getNodes())
    {
        emitFunctionCall(stage, *node, context, true);
    }
END_SHADER_STAGE(stage, HW::VERTEX_STAGE)

BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
    // For pixel stage surface shaders need special handling
    if (graph.hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE))
    {
        // Handle all texturing nodes. These are inputs to any
        // closure/shader nodes and need to be emitted first.
        emitTextureNodes(stage, graph, context);

        // Emit function calls for all surface shader nodes
        for (const ShaderNode* node : graph.getNodes())
        {
            if (node->hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE))
            {
                emitFunctionCall(stage, *node, context, false);
            }
        }
    }
    else
    {
        // No surface shader or closure graph,
        // so generate a normel function call.
        HwShaderGenerator::emitFunctionCalls(stage, graph, context);
    }
END_SHADER_STAGE(stage, HW::PIXEL_STAGE)
}

void GlslShaderGenerator::emitTextureNodes(ShaderStage& stage, const ShaderGraph& graph, GenContext& context)
{
    // Emit function calls for all texturing nodes
    bool found = false;
    for (const ShaderNode* node : graph.getNodes())
    {
        if (node->hasClassification(ShaderNode::Classification::TEXTURE) && !node->referencedConditionally())
        {
            emitFunctionCall(stage, *node, context, false);
            found = true;
        }
    }

    if (found)
    {
        emitLineBreak(stage);
    }
}

void GlslShaderGenerator::emitBsdfNodes(ShaderStage& stage, const ShaderGraph& graph, GenContext& context, 
                                        const ShaderNode& surfaceShader, int closureType, 
                                        const string& incident, const string& outgoing, 
                                        string& bsdf)
{
    HwClosureContext ccx(closureType);

    switch (closureType)
    {
    case HwClosureContext::REFLECTION:
        ccx.addArgument("vec3", incident);
        ccx.addArgument("vec3", outgoing);
        ccx.setSuffix("_reflection");
        break;
    case HwClosureContext::TRANSMISSION:
        ccx.addArgument("vec3", outgoing);
        ccx.setSuffix("_transmission");
        break;
    case HwClosureContext::INDIRECT:
        ccx.addArgument("vec3", outgoing);
        ccx.setSuffix("_indirect");
        break;
    default:
        throw ExceptionShaderGenError("Unknown closure context given when generating bsdf node function calls");
    }

    bsdf = "BSDF(0.0)";

    context.pushUserData(HW::CLOSURE_CONTEXT, &ccx);

    // Emit function calls for all BSDF nodes used by this surface shader.
    // The last node will hold the final result.
    const ShaderNode* last = nullptr;
    for (const ShaderNode* node : graph.getNodes())
    {
        if (node->hasClassification(ShaderNode::Classification::BSDF) && surfaceShader.isUsedClosure(node))
        {
            emitFunctionCall(stage, *node, context, false);
            last = node;
        }
    }
    if (last)
    {
        bsdf = last->getOutput()->variable;
    }

    context.popUserData(HW::CLOSURE_CONTEXT);
}

void GlslShaderGenerator::emitEdfNodes(ShaderStage& stage, const ShaderGraph& graph, GenContext& context, 
                                       const ShaderNode& lightShader, const string& normalDir, const string& evalDir, 
                                       string& edf)
{
    // Set extra arguments according to the given directions
    HwClosureContext ccx(HwClosureContext::EMISSION);
    ccx.addArgument("vec3", normalDir);
    ccx.addArgument("vec3", evalDir);

    edf = "EDF(0.0)";

    context.pushUserData(HW::CLOSURE_CONTEXT, &ccx);

    // Emit function calls for all EDF nodes used by this shader
    // The last node will hold the final result
    const ShaderNode* last = nullptr;
    for (const ShaderNode* node : graph.getNodes())
    {
        if (node->hasClassification(ShaderNode::Classification::EDF) && lightShader.isUsedClosure(node))
        {
            emitFunctionCall(stage, *node, context, false);
            last = node;
        }
    }
    if (last)
    {
        edf = last->getOutput()->variable;
    }

    context.popUserData(HW::CLOSURE_CONTEXT);
}

void GlslShaderGenerator::toVec4(const TypeDesc* type, string& variable)
{
    if (type->isFloat3())
    {
        variable = "vec4(" + variable + ", 1.0)";
    }
    else if (type->isFloat2())
    {
        variable = "vec4(" + variable + ", 0.0, 1.0)";
    }
    else if (type == Type::FLOAT || type == Type::INTEGER)
    {
        variable = "vec4(" + variable + ", " + variable + ", " + variable + ", 1.0)";
    }
    else if (type == Type::BSDF || type == Type::EDF)
    {
        variable = "vec4(" + variable + ", 1.0)";
    }
    else
    {
        // Can't understand other types. Just return black.
        variable = "vec4(0.0, 0.0, 0.0, 1.0)";
    }
}

void GlslShaderGenerator::emitVariable(ShaderStage& stage, const Variable& variable, const string& qualifier, bool assingValue)
{
    // A file texture input needs special handling on GLSL
    if (variable.getType() == Type::FILENAME)
    {
        // Samplers must always be uniforms
        emitString(stage, "uniform sampler2D " + variable.getName());
    }
    else
    {
        // If an array we need an array qualifier (suffix) for the variable name
        string arraySuffix;
        variable.getArraySuffix(arraySuffix);

        string str = qualifier.empty() ? EMPTY_STRING : qualifier + " ";
        str += _syntax->getTypeName(variable.getType()) + " " + variable.getName();
        str += arraySuffix;

        if (!variable.getSemantic().empty())
        {
            str += " : " + variable.getSemantic();
        }

        if (assingValue)
        {
            const string valueStr = (variable.getValue() ?
                _syntax->getValue(variable.getType(), *variable.getValue(), true) :
                _syntax->getDefaultValue(variable.getType(), true));
            str += valueStr.empty() ? EMPTY_STRING : " = " + valueStr;
        }

        emitString(stage, str);
    }
}

ShaderNodeImplPtr GlslShaderGenerator::createCompoundImplementation(NodeGraphPtr impl)
{
    NodeDefPtr nodeDef = impl->getNodeDef();
    if (!nodeDef)
    {
        throw ExceptionShaderGenError("Error creating compound implementation. Given nodegraph '" + impl->getName() + "' has no nodedef set");
    }
    if (TypeDesc::get(nodeDef->getType()) == Type::LIGHTSHADER)
    {
        return LightCompoundNodeGlsl::create();
    }
    return HwShaderGenerator::createCompoundImplementation(impl);
}

ValuePtr GlslShaderGenerator::remapEnumeration(const ValueElementPtr& input, const InterfaceElement& mappingElement, const TypeDesc*& enumerationType)
{
    const string& inputName = input->getName();
    const string& inputValue = input->getValueString();
    const string& inputType = input->getType();

    return remapEnumeration(inputName, inputValue, inputType, mappingElement, enumerationType);
}

ValuePtr GlslShaderGenerator::remapEnumeration(const string& inputName, const string& inputValue, const string& inputType, const InterfaceElement& mappingElement, const TypeDesc*& enumerationType)
{
    enumerationType = nullptr;

    ValueElementPtr valueElem = mappingElement.getChildOfType<ValueElement>(inputName);
    if (!valueElem)
    {
        return nullptr;
    }

    // Don't convert file names and arrays to integers
    const TypeDesc* inputTypeDesc = TypeDesc::get(inputType);
    if (inputTypeDesc->isArray() || inputTypeDesc == Type::FILENAME)
    {
        return nullptr;
    }
    // Don't convert supported types
    if (getSyntax()->typeSupported(inputTypeDesc))
    {
        return nullptr;
    }

    // Skip any elements which have no enumerations
    const string& valueElemEnums = valueElem->getAttribute(ValueElement::ENUM_ATTRIBUTE);
    if (valueElemEnums.empty())
    {
        return nullptr;
    }

    // Always update the type. For GLSL we always convert to integers,
    // with the integer value being an index into the enumeration.
    enumerationType = TypeDesc::get(TypedValue<int>::TYPE);

    // Update the return value if any was specified. If the value
    // cannot be found always return a default value of 0 to provide some mapping.
    ValuePtr returnValue = nullptr;
    if (inputValue.size())
    {
        int integerValue = 0;
        StringVec valueElemEnumsVec = splitString(valueElemEnums, ",");
        auto pos = std::find(valueElemEnumsVec.begin(), valueElemEnumsVec.end(), inputValue);
        if (pos != valueElemEnumsVec.end())
        {
            integerValue = static_cast<int>(std::distance(valueElemEnumsVec.begin(), pos));
        }
        returnValue = Value::createValue<int>(integerValue);
    }
    return returnValue;
}

const string GlslImplementation::SPACE = "space";
const string GlslImplementation::WORLD = "world";
const string GlslImplementation::OBJECT = "object";
const string GlslImplementation::MODEL = "model";
const string GlslImplementation::INDEX = "index";
const string GlslImplementation::ATTRNAME = "attrname";

const string& GlslImplementation::getLanguage() const
{
    return GlslShaderGenerator::LANGUAGE;
}

const string& GlslImplementation::getTarget() const
{
    return GlslShaderGenerator::TARGET;
}

}
