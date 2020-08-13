//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

#include <MaterialXGenGlsl/GlslSyntax.h>
#include <MaterialXGenGlsl/Nodes/PositionNodeGlsl.h>
#include <MaterialXGenGlsl/Nodes/NormalNodeGlsl.h>
#include <MaterialXGenGlsl/Nodes/TangentNodeGlsl.h>
#include <MaterialXGenGlsl/Nodes/BitangentNodeGlsl.h>
#include <MaterialXGenGlsl/Nodes/TexCoordNodeGlsl.h>
#include <MaterialXGenGlsl/Nodes/GeomColorNodeGlsl.h>
#include <MaterialXGenGlsl/Nodes/GeomPropValueNodeGlsl.h>
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
#include <MaterialXGenGlsl/Nodes/TransformVectorNodeGlsl.h>
#include <MaterialXGenGlsl/Nodes/TransformPointNodeGlsl.h>
#include <MaterialXGenGlsl/Nodes/TransformNormalNodeGlsl.h>

#include <MaterialXGenShader/Nodes/HwSourceCodeNode.h>
#include <MaterialXGenShader/Nodes/SwizzleNode.h>
#include <MaterialXGenShader/Nodes/ConvertNode.h>
#include <MaterialXGenShader/Nodes/CombineNode.h>
#include <MaterialXGenShader/Nodes/SwitchNode.h>
#include <MaterialXGenShader/Nodes/IfNode.h>
#include <MaterialXGenShader/Nodes/BlurNode.h>
#include <MaterialXGenShader/Nodes/HwImageNode.h>
#include <MaterialXGenShader/Nodes/LayerNode.h>
#include <MaterialXGenShader/Nodes/ThinFilmNode.h>

namespace MaterialX
{

const string GlslShaderGenerator::LANGUAGE = "genglsl";
const string GlslShaderGenerator::TARGET = "glsl400";
const string GlslShaderGenerator::VERSION = "400";

//
// GlslShaderGenerator methods
//

GlslShaderGenerator::GlslShaderGenerator() :
    HwShaderGenerator(GlslSyntax::create())
{
    //
    // Register all custom node implementation classes
    //

    // <!-- <if*> -->
    static const string SEPARATOR = "_";
    static const string INT_SEPARATOR = "I_";
    static const string BOOL_SEPARATOR = "B_";
    static const StringVec IMPL_PREFIXES = { "IM_ifgreater_", "IM_ifgreatereq_", "IM_ifequal_" };
    static const vector<CreatorFunction<ShaderNodeImpl>> IMPL_CREATE_FUNCTIONS =
            { IfGreaterNode::create,  IfGreaterEqNode::create, IfEqualNode::create };
    static const vector<bool> IMPL_HAS_INTVERSION = { true, true, true };
    static const vector<bool> IMPL_HAS_BOOLVERSION = { false, false, true };
    static const StringVec IMPL_TYPES = { "float", "color2", "color3", "color4", "vector2", "vector3", "vector4" };
    for (size_t i=0; i<IMPL_PREFIXES.size(); i++)
    {
        const string& implPrefix = IMPL_PREFIXES[i];
        for (const string& implType : IMPL_TYPES)
        {
            const string implRoot = implPrefix + implType;
            registerImplementation(implRoot + SEPARATOR + GlslShaderGenerator::LANGUAGE, IMPL_CREATE_FUNCTIONS[i]);
            if (IMPL_HAS_INTVERSION[i])
            {
                registerImplementation(implRoot + INT_SEPARATOR + GlslShaderGenerator::LANGUAGE, IMPL_CREATE_FUNCTIONS[i]);
            }
            if (IMPL_HAS_BOOLVERSION[i])
            {
                registerImplementation(implRoot + BOOL_SEPARATOR + GlslShaderGenerator::LANGUAGE, IMPL_CREATE_FUNCTIONS[i]);
            }
        }
    }
    
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
    registerImplementation("IM_convert_vector2_vector3_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_vector3_vector2_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_vector3_color3_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_vector3_vector4_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_vector4_vector3_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_vector4_color4_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_color2_vector2_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_color3_vector3_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_color4_vector4_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_color3_color4_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_color4_color3_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_boolean_float_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);
    registerImplementation("IM_convert_integer_float_" + GlslShaderGenerator::LANGUAGE, ConvertNode::create);

    // <!-- <combine> -->
    registerImplementation("IM_combine2_color2_" + GlslShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine2_vector2_" + GlslShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine2_color4CF_" + GlslShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine2_vector4VF_" + GlslShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine2_color4CC_" + GlslShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine2_vector4VV_" + GlslShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine3_color3_" + GlslShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine3_vector3_" + GlslShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine4_color4_" + GlslShaderGenerator::LANGUAGE, CombineNode::create);
    registerImplementation("IM_combine4_vector4_" + GlslShaderGenerator::LANGUAGE, CombineNode::create);

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
    // <!-- <geompropvalue> -->
    registerImplementation("IM_geompropvalue_integer_" + GlslShaderGenerator::LANGUAGE, GeomPropValueNodeGlsl::create);
    registerImplementation("IM_geompropvalue_boolean_" + GlslShaderGenerator::LANGUAGE, GeomPropValueNodeGlsl::create);
    registerImplementation("IM_geompropvalue_string_" + GlslShaderGenerator::LANGUAGE, GeomPropValueNodeGlsl::create);
    registerImplementation("IM_geompropvalue_float_" + GlslShaderGenerator::LANGUAGE, GeomPropValueNodeGlsl::create);
    registerImplementation("IM_geompropvalue_color2_" + GlslShaderGenerator::LANGUAGE, GeomPropValueNodeGlsl::create);
    registerImplementation("IM_geompropvalue_color3_" + GlslShaderGenerator::LANGUAGE, GeomPropValueNodeGlsl::create);
    registerImplementation("IM_geompropvalue_color4_" + GlslShaderGenerator::LANGUAGE, GeomPropValueNodeGlsl::create);
    registerImplementation("IM_geompropvalue_vector2_" + GlslShaderGenerator::LANGUAGE, GeomPropValueNodeGlsl::create);
    registerImplementation("IM_geompropvalue_vector3_" + GlslShaderGenerator::LANGUAGE, GeomPropValueNodeGlsl::create);
    registerImplementation("IM_geompropvalue_vector4_" + GlslShaderGenerator::LANGUAGE, GeomPropValueNodeGlsl::create);

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

    // <!-- <point_light> -->
    registerImplementation("IM_point_light_" + GlslShaderGenerator::LANGUAGE, LightShaderNodeGlsl::create);
    // <!-- <directional_light> -->
    registerImplementation("IM_directional_light_" + GlslShaderGenerator::LANGUAGE, LightShaderNodeGlsl::create);
    // <!-- <spot_light> -->
    registerImplementation("IM_spot_light_" + GlslShaderGenerator::LANGUAGE, LightShaderNodeGlsl::create);

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

    // <!-- <ND_transformpoint> ->
    registerImplementation("IM_transformpoint_vector3_" + GlslShaderGenerator::LANGUAGE, TransformPointNodeGlsl::create);

    // <!-- <ND_transformvector> ->
    registerImplementation("IM_transformvector_vector3_" + GlslShaderGenerator::LANGUAGE, TransformVectorNodeGlsl::create);

    // <!-- <ND_transformnormal> ->
    registerImplementation("IM_transformnormal_vector3_" + GlslShaderGenerator::LANGUAGE, TransformNormalNodeGlsl::create);

    // <!-- <image> -->
    registerImplementation("IM_image_float_" + GlslShaderGenerator::LANGUAGE, HwImageNode::create);
    registerImplementation("IM_image_color2_" + GlslShaderGenerator::LANGUAGE, HwImageNode::create);
    registerImplementation("IM_image_color3_" + GlslShaderGenerator::LANGUAGE, HwImageNode::create);
    registerImplementation("IM_image_color4_" + GlslShaderGenerator::LANGUAGE, HwImageNode::create);
    registerImplementation("IM_image_vector2_" + GlslShaderGenerator::LANGUAGE, HwImageNode::create);
    registerImplementation("IM_image_vector3_" + GlslShaderGenerator::LANGUAGE, HwImageNode::create);
    registerImplementation("IM_image_vector4_" + GlslShaderGenerator::LANGUAGE, HwImageNode::create);

    // <!-- <layer> -->
    registerImplementation("IM_layer_bsdf_" + GlslShaderGenerator::LANGUAGE, LayerNode::create);

    // <!-- <thin_film_brdf> -->
    registerImplementation("IM_thin_film_brdf_" + GlslShaderGenerator::LANGUAGE, ThinFilmNode::create);
    // <!-- <dielectric_brdf> -->
    registerImplementation("IM_dielectric_brdf_" + GlslShaderGenerator::LANGUAGE, ThinFilmSupport::create);

    _lightSamplingNodes.push_back(ShaderNode::create(nullptr, "numActiveLightSources", NumLightsNodeGlsl::create()));
    _lightSamplingNodes.push_back(ShaderNode::create(nullptr, "sampleLightSource", LightSamplerNodeGlsl::create()));
}

ShaderPtr GlslShaderGenerator::generate(const string& name, ElementPtr element, GenContext& context) const
{
    ShaderPtr shader = createShader(name, element, context);

    // Turn on fixed float formatting to make sure float values are
    // emitted with a decimal point and not as integers, and to avoid
    // any scientific notation which isn't supported by all OpenGL targets.
    ScopedFloatFormatting fmt(Value::FloatFormatFixed);

    // Make sure we initialize/reset the binding context before generation.
    HwResourceBindingContextPtr resourceBindingCtx = context.getUserData<HwResourceBindingContext>(HW::USER_DATA_BINDING_CONTEXT);
    if (resourceBindingCtx)
    {
        resourceBindingCtx->initialize();
    }

    // Emit code for vertex shader stage
    ShaderStage& vs = shader->getStage(Stage::VERTEX);
    emitVertexStage(shader->getGraph(), context, vs);
    replaceTokens(_tokenSubstitutions, vs);

    // Emit code for pixel shader stage
    ShaderStage& ps = shader->getStage(Stage::PIXEL);
    emitPixelStage(shader->getGraph(), context, ps);
    replaceTokens(_tokenSubstitutions, ps);

    return shader;
}

void GlslShaderGenerator::emitVertexStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    HwResourceBindingContextPtr resourceBindingCtx = context.getUserData<HwResourceBindingContext>(HW::USER_DATA_BINDING_CONTEXT);

    // Add directives
    emitLine("#version " + getVersion(), stage, false);
    if (resourceBindingCtx)
    {
        resourceBindingCtx->emitDirectives(context, stage);
    }
    emitLineBreak(stage);

    // Add all constants
    const VariableBlock& constants = stage.getConstantBlock();
    if (!constants.empty())
    {
        emitVariableDeclarations(constants, _syntax->getConstantQualifier(), Syntax::SEMICOLON, context, stage);
        emitLineBreak(stage);
    }

    // Add all uniforms
    for (const auto& it : stage.getUniformBlocks())
    {
        const VariableBlock& uniforms = *it.second;
        if (!uniforms.empty())
        {
            emitComment("Uniform block: " + uniforms.getName(), stage);
            if (resourceBindingCtx)
            {
                resourceBindingCtx->emitResourceBindings(context, uniforms, stage);
            }
            else
            {
                emitVariableDeclarations(uniforms, _syntax->getUniformQualifier(), Syntax::SEMICOLON, context, stage);
                emitLineBreak(stage);
            }
        }
    }

    // Add vertex inputs
    const VariableBlock& vertexInputs = stage.getInputBlock(HW::VERTEX_INPUTS);
    if (!vertexInputs.empty())
    {
        emitComment("Inputs block: " + vertexInputs.getName(), stage);
        emitVariableDeclarations(vertexInputs, _syntax->getInputQualifier(), Syntax::SEMICOLON, context, stage, false);
        emitLineBreak(stage);
    }

    // Add vertex data outputs block
    const VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
    if (!vertexData.empty())
    {
        emitLine("out " + vertexData.getName(), stage, false);
        emitScopeBegin(stage);
        emitVariableDeclarations(vertexData, EMPTY_STRING, Syntax::SEMICOLON, context, stage, false);
        emitScopeEnd(stage, false, false);
        emitString(" " + vertexData.getInstance() + Syntax::SEMICOLON, stage);
        emitLineBreak(stage);
        emitLineBreak(stage);
    }

    emitFunctionDefinitions(graph, context, stage);

    // Add main function
    setFunctionName("main", stage);
    emitLine("void main()", stage, false);
    emitScopeBegin(stage);
    emitLine("vec4 hPositionWorld = " + HW::T_WORLD_MATRIX + " * vec4(" + HW::T_IN_POSITION + ", 1.0)", stage);
    emitLine("gl_Position = " + HW::T_VIEW_PROJECTION_MATRIX + " * hPositionWorld", stage);
    emitFunctionCalls(graph, context, stage);
    emitScopeEnd(stage);
    emitLineBreak(stage);
}

void GlslShaderGenerator::emitSpecularEnvironment(GenContext& context, ShaderStage& stage) const
{
    int specularMethod = context.getOptions().hwSpecularEnvironmentMethod;
    if (specularMethod == SPECULAR_ENVIRONMENT_FIS)
    {
        emitInclude("pbrlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_environment_fis.glsl", context, stage);
    }
    else if (specularMethod == SPECULAR_ENVIRONMENT_PREFILTER)
    {
        emitInclude("pbrlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_environment_prefilter.glsl", context, stage);
    }
    else if (specularMethod == SPECULAR_ENVIRONMENT_NONE)
    {
        emitInclude("pbrlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_environment_none.glsl", context, stage);
    }
    else
    {
        throw ExceptionShaderGenError("Invalid hardware specular environment method specified: '" + std::to_string(specularMethod) + "'");
    }
    emitLineBreak(stage);
}

void GlslShaderGenerator::emitPixelStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    HwResourceBindingContextPtr resourceBindingCtx = context.getUserData<HwResourceBindingContext>(HW::USER_DATA_BINDING_CONTEXT);

    // Add directives
    emitLine("#version " + getVersion(), stage, false);
    if (resourceBindingCtx)
    {
        resourceBindingCtx->emitDirectives(context, stage);
    }
    emitLineBreak(stage);

    // Add global constants and type definitions
    emitInclude("pbrlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_defines.glsl", context, stage);

    if (context.getOptions().hwMaxActiveLightSources > 0)
    {
        const unsigned int maxLights = std::max(1u, context.getOptions().hwMaxActiveLightSources);
        emitLine("#define " + HW::LIGHT_DATA_MAX_LIGHT_SOURCES + " " + std::to_string(maxLights), stage, false);
    }
    emitLine("#define DIRECTIONAL_ALBEDO_METHOD " + std::to_string(int(context.getOptions().hwDirectionalAlbedoMethod)), stage, false);
    emitLine("#define " + HW::ENV_RADIANCE_MAX_SAMPLES + " " + std::to_string(context.getOptions().hwMaxRadianceSamples), stage, false);
    emitLineBreak(stage);
    emitTypeDefinitions(context, stage);

    // Add all constants
    const VariableBlock& constants = stage.getConstantBlock();
    if (!constants.empty())
    {
        emitVariableDeclarations(constants, _syntax->getConstantQualifier(), Syntax::SEMICOLON, context, stage);
        emitLineBreak(stage);
    }

    // Add all uniforms
    for (const auto& it : stage.getUniformBlocks())
    {
        const VariableBlock& uniforms = *it.second;

        // Skip light uniforms as they are handled separately
        if (!uniforms.empty() && uniforms.getName() != HW::LIGHT_DATA)
        {
            emitComment("Uniform block: " + uniforms.getName(), stage);
            if (resourceBindingCtx)
            {
                resourceBindingCtx->emitResourceBindings(context, uniforms, stage);
            }
            else
            {
                emitVariableDeclarations(uniforms, _syntax->getUniformQualifier(), Syntax::SEMICOLON, context, stage);
                emitLineBreak(stage);
            }
        }
    }

    bool lighting = graph.hasClassification(ShaderNode::Classification::SHADER|ShaderNode::Classification::SURFACE) ||
                    graph.hasClassification(ShaderNode::Classification::BSDF);
    bool shadowing = (lighting && context.getOptions().hwShadowMap) ||
                     context.getOptions().hwWriteDepthMoments;

    // Add light data block if needed
    if (lighting && context.getOptions().hwMaxActiveLightSources > 0)
    {
        const VariableBlock& lightData = stage.getUniformBlock(HW::LIGHT_DATA);
        const string structArraySuffix = "[" + HW::LIGHT_DATA_MAX_LIGHT_SOURCES + "]";
        const string structName        = lightData.getInstance();
        if (resourceBindingCtx)
        {
            resourceBindingCtx->emitStructuredResourceBindings(
                context, lightData, stage, structName, structArraySuffix);
        }
        else
        {
            emitLine("struct " + lightData.getName(), stage, false);
            emitScopeBegin(stage);
            emitVariableDeclarations(lightData, EMPTY_STRING, Syntax::SEMICOLON, context, stage, false);
            emitScopeEnd(stage, true);
            emitLineBreak(stage);
            emitLine("uniform " + lightData.getName() + " " + structName + structArraySuffix, stage);
        }
        emitLineBreak(stage);
    }

    // Add vertex data inputs block
    const VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
    if (!vertexData.empty())
    {
        emitLine("in " + vertexData.getName(), stage, false);
        emitScopeBegin(stage);
        emitVariableDeclarations(vertexData, EMPTY_STRING, Syntax::SEMICOLON, context, stage, false);
        emitScopeEnd(stage, false, false);
        emitString(" " + vertexData.getInstance() + Syntax::SEMICOLON, stage);
        emitLineBreak(stage);
        emitLineBreak(stage);
    }

    // Add the pixel shader output. This needs to be a vec4 for rendering
    // and upstream connection will be converted to vec4 if needed in emitFinalOutput()
    emitComment("Pixel shader outputs", stage);
    const VariableBlock& outputs = stage.getOutputBlock(HW::PIXEL_OUTPUTS);
    emitVariableDeclarations(outputs, _syntax->getOutputQualifier(), Syntax::SEMICOLON, context, stage, false);
    emitLineBreak(stage);

    // Emit common math functions
    emitInclude("pbrlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_math.glsl", context, stage);
    emitLineBreak(stage);

    // Emit lighting and shadowing code
    if (lighting)
    {
        emitSpecularEnvironment(context, stage);
    }
    if (shadowing)
    {
        emitInclude("pbrlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_shadow.glsl", context, stage);
    }

    // Emit directional albedo table code.
    if (context.getOptions().hwDirectionalAlbedoMethod == DIRECTIONAL_ALBEDO_TABLE ||
        context.getOptions().hwWriteAlbedoTable)
    {
        emitInclude("pbrlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_table.glsl", context, stage);
        emitLineBreak(stage);
    }

    // Set the include file to use for uv transformations,
    // depending on the vertical flip flag.
    if (context.getOptions().fileTextureVerticalFlip)
    {
        _tokenSubstitutions[ShaderGenerator::T_FILE_TRANSFORM_UV] = "stdlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_transform_uv_vflip.glsl";
    }
    else
    {
        _tokenSubstitutions[ShaderGenerator::T_FILE_TRANSFORM_UV] = "stdlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_transform_uv.glsl";
    }

    // Emit uv transform code globally if needed.
    if (context.getOptions().hwAmbientOcclusion)
    {
        emitInclude(ShaderGenerator::T_FILE_TRANSFORM_UV, context, stage);
    }

    // Add all functions for node implementations
    emitFunctionDefinitions(graph, context, stage);

    const ShaderGraphOutputSocket* outputSocket = graph.getOutputSocket();

    // Add main function
    setFunctionName("main", stage);
    emitLine("void main()", stage, false);
    emitScopeBegin(stage);

    if (graph.hasClassification(ShaderNode::Classification::CLOSURE))
    {
        // Handle the case where the graph is a direct closure.
        // We don't support rendering closures without attaching 
        // to a surface shader, so just output black.
        emitLine(outputSocket->getVariable() + " = vec4(0.0, 0.0, 0.0, 1.0)", stage);
    }
    else if (context.getOptions().hwWriteDepthMoments)
    {
        emitLine(outputSocket->getVariable() + " = vec4(mx_compute_depth_moments(), 0.0, 1.0)", stage);
    }
    else if (context.getOptions().hwWriteAlbedoTable)
    {
        emitLine(outputSocket->getVariable() + " = vec4(mx_ggx_directional_albedo_generate_table(), 0.0, 1.0)", stage);
    }
    else
    {
        // Add all function calls
        emitFunctionCalls(graph, context, stage);

        // Emit final output
        const ShaderOutput* outputConnection = outputSocket->getConnection();
        if (outputConnection)
        {
            string finalOutput = outputConnection->getVariable();
            const string& channels = outputSocket->getChannels();
            if (!channels.empty())
            {
                finalOutput = _syntax->getSwizzledVariable(finalOutput, outputConnection->getType(), channels, outputSocket->getType());
            }

            if (graph.hasClassification(ShaderNode::Classification::SURFACE))
            {
                if (context.getOptions().hwTransparency)
                {
                    emitLine("float outAlpha = clamp(1.0 - dot(" + finalOutput + ".transparency, vec3(0.3333)), 0.0, 1.0)", stage);
                    emitLine(outputSocket->getVariable() + " = vec4(" + finalOutput + ".color, outAlpha)", stage);
                }
                else
                {
                    emitLine(outputSocket->getVariable() + " = vec4(" + finalOutput + ".color, 1.0)", stage);
                }
            }
            else
            {
                if (!outputSocket->getType()->isFloat4())
                {
                    toVec4(outputSocket->getType(), finalOutput);
                }
                emitLine(outputSocket->getVariable() + " = " + finalOutput, stage);
            }
        }
        else
        {
            string outputValue = outputSocket->getValue() ? _syntax->getValue(outputSocket->getType(), *outputSocket->getValue()) : _syntax->getDefaultValue(outputSocket->getType());
            if (!outputSocket->getType()->isFloat4())
            {
                string finalOutput = outputSocket->getVariable() + "_tmp";
                emitLine(_syntax->getTypeName(outputSocket->getType()) + " " + finalOutput + " = " + outputValue, stage);
                toVec4(outputSocket->getType(), finalOutput);
                emitLine(outputSocket->getVariable() + " = " + finalOutput, stage);
            }
            else
            {
                emitLine(outputSocket->getVariable() + " = " + outputValue, stage);
            }
        }
    }

    // End main function
    emitScopeEnd(stage);
    emitLineBreak(stage);
}

void GlslShaderGenerator::emitFunctionDefinitions(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
BEGIN_SHADER_STAGE(stage, Stage::PIXEL)

    // Emit Light functions if requested
    if (context.getOptions().hwMaxActiveLightSources > 0)
    {
        // For surface shaders we need light shaders
        if (graph.hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE))
        {
            // Emit functions for all bound light shaders
            HwLightShadersPtr lightShaders = context.getUserData<HwLightShaders>(HW::USER_DATA_LIGHT_SHADERS);
            if (lightShaders)
            {
                for (const auto& it : lightShaders->get())
                {
                    emitFunctionDefinition(*it.second, context, stage);
                }
            }
            // Emit functions for light sampling
            for (const auto& it : _lightSamplingNodes)
            {
                emitFunctionDefinition(*it, context, stage);
            }
        }
    }
END_SHADER_STAGE(stage, Stage::PIXEL)

    // Call parent to emit all other functions
    HwShaderGenerator::emitFunctionDefinitions(graph, context, stage);
}

void GlslShaderGenerator::emitFunctionCalls(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
BEGIN_SHADER_STAGE(stage, Stage::VERTEX)
    // For vertex stage just emit all function calls in order
    // and ignore conditional scope.
    for (const ShaderNode* node : graph.getNodes())
    {
        emitFunctionCall(*node, context, stage, false);
    }
END_SHADER_STAGE(stage, Stage::VERTEX)

BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
    // For pixel stage surface shaders need special handling
    if (graph.hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE))
    {
        // Handle all texturing nodes. These are inputs to any
        // closure/shader nodes and need to be emitted first.
        emitTextureNodes(graph, context, stage);

        // Emit function calls for all surface shader nodes
        for (const ShaderNode* node : graph.getNodes())
        {
            if (node->hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE))
            {
                emitFunctionCall(*node, context, stage, false);
            }
        }
    }
    else
    {
        // No surface shader or closure graph,
        // so generate a normel function call.
        HwShaderGenerator::emitFunctionCalls(graph, context, stage);
    }
END_SHADER_STAGE(stage, Stage::PIXEL)
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

void GlslShaderGenerator::emitVariableDeclaration(const ShaderPort* variable, const string& qualifier, 
                                                  GenContext&, ShaderStage& stage,
                                                  bool assignValue) const
{
    // A file texture input needs special handling on GLSL
    if (variable->getType() == Type::FILENAME)
    {
        // Samplers must always be uniforms
        string str = qualifier.empty() ? EMPTY_STRING : qualifier + " ";
        emitString(str + "sampler2D " + variable->getVariable(), stage);
    }
    else
    {
        string str = qualifier.empty() ? EMPTY_STRING : qualifier + " ";
        str += _syntax->getTypeName(variable->getType()) + " " + variable->getVariable();

        // If an array we need an array qualifier (suffix) for the variable name
        if (variable->getType()->isArray() && variable->getValue())
        {
            str += _syntax->getArrayVariableSuffix(variable->getType(), *variable->getValue());
        }

        if (!variable->getSemantic().empty())
        {
            str += " : " + variable->getSemantic();
        }

        if (assignValue)
        {
            const string valueStr = (variable->getValue() ?
                _syntax->getValue(variable->getType(), *variable->getValue(), true) :
                _syntax->getDefaultValue(variable->getType(), true));
            str += valueStr.empty() ? EMPTY_STRING : " = " + valueStr;
        }

        emitString(str, stage);
    }
}

ShaderNodeImplPtr GlslShaderGenerator::createCompoundImplementation(const NodeGraph& impl) const
{
    NodeDefPtr nodeDef = impl.getNodeDef();
    if (!nodeDef)
    {
        throw ExceptionShaderGenError("Error creating compound implementation. Given nodegraph '" + impl.getName() + "' has no nodedef set");
    }
    if (TypeDesc::get(nodeDef->getType()) == Type::LIGHTSHADER)
    {
        return LightCompoundNodeGlsl::create();
    }
    return HwShaderGenerator::createCompoundImplementation(impl);
}


const string GlslImplementation::SPACE = "space";
const string GlslImplementation::TO_SPACE = "tospace";
const string GlslImplementation::FROM_SPACE = "fromspace";
const string GlslImplementation::WORLD = "world";
const string GlslImplementation::OBJECT = "object";
const string GlslImplementation::MODEL = "model";
const string GlslImplementation::INDEX = "index";
const string GlslImplementation::GEOMPROP = "geomprop";

namespace
{
    // List name of inputs that are not to be editable and
    // published as shader uniforms in GLSL.
    const std::set<string> IMMUTABLE_INPUTS = 
    {
        // Geometric node inputs are immutable since a shader needs regeneration if they change.
        "index", "space", "attrname"
    };
}

const string& GlslImplementation::getLanguage() const
{
    return GlslShaderGenerator::LANGUAGE;
}

const string& GlslImplementation::getTarget() const
{
    return GlslShaderGenerator::TARGET;
}

bool GlslImplementation::isEditable(const ShaderInput& input) const
{
    return IMMUTABLE_INPUTS.count(input.getName()) == 0;
}

}
