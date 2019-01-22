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
const string GlslShaderGenerator::LIGHT_DIR = "L";
const string GlslShaderGenerator::VIEW_DIR = "V";
const string GlslShaderGenerator::INCIDENT = "incident";
const string GlslShaderGenerator::OUTGOING = "outgoing";
const string GlslShaderGenerator::NORMAL = "normal";
const string GlslShaderGenerator::EVAL = "eval";

GlslShaderGenerator::GlslShaderGenerator()
    : ParentClass(GlslSyntax::create())
{
    //
    // Create the node contexts used by this generator
    //

    // BSDF reflection context
    GenContextPtr ctxBsdfReflection = createContext(CONTEXT_BSDF_REFLECTION);
    ctxBsdfReflection->addArgument(Argument("vec3", INCIDENT));
    ctxBsdfReflection->addArgument(Argument("vec3", OUTGOING));
    ctxBsdfReflection->setFunctionSuffix("_reflection");

    // BSDF transmission context
    GenContextPtr ctxBsdfTransmission = createContext(CONTEXT_BSDF_TRANSMISSION);
    ctxBsdfTransmission->addArgument(Argument("vec3", OUTGOING));
    ctxBsdfTransmission->setFunctionSuffix("_transmission");

    // BSDF indirect context
    GenContextPtr ctxBsdfIndirect = createContext(CONTEXT_BSDF_INDIRECT);
    ctxBsdfIndirect->addArgument(Argument("vec3", OUTGOING));
    ctxBsdfIndirect->setFunctionSuffix("_indirect");

    // EDF emission context
    GenContextPtr ctxEdf = createContext(CONTEXT_EDF);
    ctxEdf->addArgument(Argument("vec3", NORMAL));
    ctxEdf->addArgument(Argument("vec3", EVAL));

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
}

ShaderPtr GlslShaderGenerator::generate(const string& shaderName, ElementPtr element, const GenOptions& options)
{
    HwShaderPtr shaderPtr = std::make_shared<HwShader>(shaderName);
    shaderPtr->initialize(element, *this, options);

    HwShader& shader = *shaderPtr;

    // Turn on fixed float formatting to make sure float values are
    // emitted with a decimal point and not as integers, and to avoid
    // any scientific notation which isn't supported by all OpenGL targets.
    Value::ScopedFloatFormatting fmt(Value::FloatFormatFixed);

    //
    // Emit code for vertex shader stage
    //

    shader.setActiveStage(HwShader::VERTEX_STAGE);

    // Create required variables for vertex stage
    shader.createAppData(Type::VECTOR3, "i_position");
    shader.createUniform(HwShader::VERTEX_STAGE, HwShader::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldMatrix");
    shader.createUniform(HwShader::VERTEX_STAGE, HwShader::PRIVATE_UNIFORMS, Type::MATRIX44, "u_viewProjectionMatrix");

    // Add version directive
    shader.addLine("#version " + getVersion(), false);
    shader.newLine();

    // Add all constants
    const Shader::VariableBlock& vsConstants = shader.getConstantBlock(HwShader::VERTEX_STAGE);
    if (!vsConstants.empty())
    {
        shader.addComment("Constant block: " + vsConstants.name);
        emitVariableBlock(vsConstants, _syntax->getConstantQualifier(), shader);
    }

    // Add all private uniforms
    const Shader::VariableBlock& vsPrivateUniforms = shader.getUniformBlock(HwShader::VERTEX_STAGE, HwShader::PRIVATE_UNIFORMS);
    if (!vsPrivateUniforms.empty())
    {
        shader.addComment("Uniform block: " + vsPrivateUniforms.name);
        emitVariableBlock(vsPrivateUniforms, _syntax->getUniformQualifier(), shader);
    }

    // Add any public uniforms
    const Shader::VariableBlock& vsPublicUniforms = shader.getUniformBlock(HwShader::VERTEX_STAGE, HwShader::PUBLIC_UNIFORMS);
    if (!vsPublicUniforms.empty())
    {
        shader.addComment("Uniform block: " + vsPublicUniforms.name);
        emitVariableBlock(vsPublicUniforms, _syntax->getUniformQualifier(), shader);
    }

    // Add all app data inputs
    const Shader::VariableBlock& appDataBlock = shader.getAppDataBlock();
    if (!appDataBlock.empty())
    {
        shader.addComment("Application data block: " + appDataBlock.name);
        for (const Shader::Variable* input : appDataBlock.variableOrder)
        {
            const string& type = _syntax->getTypeName(input->type);
            shader.addLine("in " + type + " " + input->name);
        }
        shader.newLine();
    }

    // Add vertex data block
    const Shader::VariableBlock& vertexDataBlock = shader.getVertexDataBlock();
    if (!vertexDataBlock.empty())
    {
        shader.addLine("out VertexData", false);
        shader.beginScope(Shader::Brackets::BRACES);
        for (const Shader::Variable* output : vertexDataBlock.variableOrder)
        {
            const string& type = _syntax->getTypeName(output->type);
            shader.addLine(type + " " + output->name);
        }
        shader.endScope(false, false);
        shader.addStr(" " + vertexDataBlock.instance + ";\n");
        shader.newLine();
    }

    emitFunctionDefinitions(shader);

    // Add main function
    shader.addLine("void main()", false);
    shader.beginScope(Shader::Brackets::BRACES);
    shader.addLine("vec4 hPositionWorld = u_worldMatrix * vec4(i_position, 1.0)");
    shader.addLine("gl_Position = u_viewProjectionMatrix * hPositionWorld");
    emitFunctionCalls(*_defaultContext, shader);
    shader.endScope();
    shader.newLine();

    //
    // Emit code for pixel shader stage
    //

    shader.setActiveStage(HwShader::PIXEL_STAGE);

    // Add version directive
    shader.addLine("#version " + getVersion(), false);
    shader.newLine();

    // Add global constants and type definitions
    shader.addInclude("pbrlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_defines.glsl", *this);
    shader.addLine("#define MAX_LIGHT_SOURCES " + std::to_string(getMaxActiveLightSources()), false);
    shader.newLine();
    emitTypeDefinitions(shader);

    // Add constants
    const Shader::VariableBlock& psConstants = shader.getConstantBlock(HwShader::PIXEL_STAGE);
    if (!psConstants.empty())
    {
        shader.addComment("Constant block: " + psConstants.name);
        emitVariableBlock(psConstants, _syntax->getConstantQualifier(), shader);
    }

    // Add all private uniforms
    const Shader::VariableBlock& psPrivateUniforms = shader.getUniformBlock(HwShader::PIXEL_STAGE, HwShader::PRIVATE_UNIFORMS);
    if (!psPrivateUniforms.empty())
    {
        shader.addComment("Uniform block: " + psPrivateUniforms.name);
        emitVariableBlock(psPrivateUniforms, _syntax->getUniformQualifier(), shader);
    }

    // Add all public uniforms
    const Shader::VariableBlock& psPublicUniforms = shader.getUniformBlock(HwShader::PIXEL_STAGE, HwShader::PUBLIC_UNIFORMS);
    if (!psPublicUniforms.empty())
    {
        shader.addComment("Uniform block: " + psPublicUniforms.name);
        emitVariableBlock(psPublicUniforms, _syntax->getUniformQualifier(), shader);
    }

    bool lighting = shader.hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE) ||
                    shader.hasClassification(ShaderNode::Classification::BSDF);

    // Add light data block if needed
    if (lighting)
    {
        const Shader::VariableBlock& lightData = shader.getUniformBlock(HwShader::PIXEL_STAGE, HwShader::LIGHT_DATA_BLOCK);
        shader.addLine("struct " + lightData.name, false);
        shader.beginScope(Shader::Brackets::BRACES);
        for (const Shader::Variable* uniform : lightData.variableOrder)
        {
            const string& type = _syntax->getTypeName(uniform->type);
            shader.addLine(type + " " + uniform->name);
        }
        shader.endScope(true);
        shader.newLine();
        shader.addLine("uniform " + lightData.name + " " + lightData.instance + "[MAX_LIGHT_SOURCES]");
        shader.newLine();
    }

    // Add vertex data block
    if (!vertexDataBlock.empty())
    {
        shader.addLine("in VertexData", false);
        shader.beginScope(Shader::Brackets::BRACES);
        for (const Shader::Variable* input : vertexDataBlock.variableOrder)
        {
            const string& type = _syntax->getTypeName(input->type);
            shader.addLine(type + " " + input->name);
        }
        shader.endScope(false, false);
        shader.addStr(" " + vertexDataBlock.instance + ";\n");
        shader.newLine();
    }

    // Add the pixel shader output. This needs to be a vec4 for rendering
    // and upstream connection will be converted to vec4 if needed in emitFinalOutput()
    shader.addComment("Data output by the pixel shader");
    const ShaderGraphOutputSocket* outputSocket = shader.getGraph()->getOutputSocket();
    shader.addLine("out vec4 " + outputSocket->variable);
    shader.newLine();

    // Emit common math functions
    shader.addInclude("pbrlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_math.glsl", *this);
    shader.newLine();

    // Emit lighting functions
    if (lighting)
    {
        shader.addInclude("pbrlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_lighting.glsl", *this);
        shader.newLine();
    }

    // Emit sampling code if needed
    if (shader.hasClassification(ShaderNode::Classification::CONVOLUTION2D))
    {
        // Emit sampling functions
        shader.addInclude("stdlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_sampling.glsl", *this);
        shader.newLine();
    }

    // Add all functions for node implementations
    emitFunctionDefinitions(shader);

    // Add main function
    shader.addLine("void main()", false);
    shader.beginScope(Shader::Brackets::BRACES);
    emitFunctionCalls(*_defaultContext, shader);
    emitFinalOutput(shader);
    shader.endScope();
    shader.newLine();

    return shaderPtr;
}

void GlslShaderGenerator::emitFunctionDefinitions(Shader& shader)
{
    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

        // For surface shaders we need light shaders
        if (shader.hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE))
        {
            // Emit functions for all bound light shaders
            for (auto lightShader : getBoundLightShaders())
            {
                lightShader.second->emitFunctionDefinition(*ShaderNode::NONE, *this, shader);
            }

            // Emit active light count function
            shader.addLine("int numActiveLightSources()", false);
            shader.beginScope(Shader::Brackets::BRACES);
            shader.addLine("return min(u_numActiveLightSources, MAX_LIGHT_SOURCES)");
            shader.endScope();
            shader.newLine();

            // Emit light sampler function with all bound light types
            shader.addLine("void sampleLightSource(LightData light, vec3 position, out lightshader result)", false);
            shader.beginScope(Shader::Brackets::BRACES);
            shader.addLine("result.intensity = vec3(0.0)");
            string ifstatement = "if ";
            for (auto lightShader : getBoundLightShaders())
            {
                shader.addLine(ifstatement + "(light.type == " + std::to_string(lightShader.first) + ")", false);
                shader.beginScope(Shader::Brackets::BRACES);
                lightShader.second->emitFunctionCall(*ShaderNode::NONE, *_defaultContext, *this, shader);
                shader.endScope();
                ifstatement = "else if ";
            }
            shader.endScope();
            shader.newLine();
        }

    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

    // Call parent to emit all other functions
    ParentClass::emitFunctionDefinitions(shader);
}

void GlslShaderGenerator::emitFunctionCalls(const GenContext& context, Shader &shader)
{
    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)
        // For vertex stage just emit all function calls in order
        // and ignore conditional scope.
        for (ShaderNode* node : shader.getGraph()->getNodes())
        {
            shader.addFunctionCall(node, context, *this);
        }
    END_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        // For pixel stage surface shaders need special handling
        if (shader.hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE))
        {
            // Handle all texturing nodes. These are inputs to any
            // closure/shader nodes and need to be emitted first.
            emitTextureNodes(shader);

            // Emit function calls for all surface shader nodes
            for (ShaderNode* node : shader.getGraph()->getNodes())
            {
                if (node->hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE))
                {
                    shader.addFunctionCall(node, context, *this);
                }
            }
        }
        else
        {
            // No surface shader, fallback to parent class
            ParentClass::emitFunctionCalls(context, shader);
        }
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

void GlslShaderGenerator::emitFinalOutput(Shader& shader) const
{
    const ShaderGraphOutputSocket* outputSocket = shader.getGraph()->getOutputSocket();

    // Early out for the rare case where the whole graph is just a single value
    if (!outputSocket->connection)
    {
        string outputValue = outputSocket->value ? _syntax->getValue(outputSocket->type, *outputSocket->value) : _syntax->getDefaultValue(outputSocket->type);
        if (!outputSocket->type->isFloat4())
        {
            string finalOutput = outputSocket->variable + "_tmp";
            shader.addLine(_syntax->getTypeName(outputSocket->type) + " " + finalOutput + " = " + outputValue);
            toVec4(outputSocket->type, finalOutput);
            shader.addLine(outputSocket->variable + " = " + finalOutput);
        }
        else
        {
            shader.addLine(outputSocket->variable + " = " + outputValue);
        }
        return;
    }

    string finalOutput = outputSocket->connection->variable;

    if (shader.hasClassification(ShaderNode::Classification::SURFACE))
    {
        const HwShader& hwShader = static_cast<const HwShader&>(shader);
        if (hwShader.hasTransparency())
        {
            shader.addLine("float outAlpha = clamp(1.0 - dot(" + finalOutput + ".transparency, vec3(0.3333)), 0.0, 1.0)");
            shader.addLine(outputSocket->variable + " = vec4(" + finalOutput + ".color, outAlpha)");
        }
        else
        {
            shader.addLine(outputSocket->variable + " = vec4(" + finalOutput + ".color, 1.0)");
        }
    }
    else
    {
        if (!outputSocket->type->isFloat4())
        {
            toVec4(outputSocket->type, finalOutput);
        }
        shader.addLine(outputSocket->variable + " = " + finalOutput);
    }
}

void GlslShaderGenerator::addNodeContextIDs(ShaderNode* node) const
{
    if (node->hasClassification(ShaderNode::Classification::BSDF))
    {
        if (node->hasClassification(ShaderNode::Classification::BSDF_R))
        {
            // A BSDF for reflection only
            node->addContextID(CONTEXT_BSDF_REFLECTION);
            node->addContextID(CONTEXT_BSDF_INDIRECT);
        }
        else if (node->hasClassification(ShaderNode::Classification::BSDF_T))
        {
            // A BSDF for transmission only
            node->addContextID(CONTEXT_BSDF_TRANSMISSION);
        }
        else
        {
            // A general BSDF handling both reflection and transmission
            node->addContextID(CONTEXT_BSDF_REFLECTION);
            node->addContextID(CONTEXT_BSDF_TRANSMISSION);
            node->addContextID(CONTEXT_BSDF_INDIRECT);
        }
    }
    else if (node->hasClassification(ShaderNode::Classification::EDF))
    {
        node->addContextID(CONTEXT_EDF);
    }
    else
    {
        ParentClass::addNodeContextIDs(node);
    }
}

void GlslShaderGenerator::emitTextureNodes(Shader& shader)
{
    // Emit function calls for all texturing nodes
    bool found = false;
    for (ShaderNode* node : shader.getGraph()->getNodes())
    {
        if (node->hasClassification(ShaderNode::Classification::TEXTURE) && !node->referencedConditionally())
        {
            shader.addFunctionCall(node, *_defaultContext, *this);
            found = true;
        }
    }

    if (found)
    {
        shader.newLine();
    }
}

void GlslShaderGenerator::emitBsdfNodes(const ShaderNode& shaderNode, int bsdfContext, const string& incident, const string& outgoing, Shader& shader, string& bsdf)
{
    GenContext context(bsdfContext);

    switch (bsdfContext)
    {
    case CONTEXT_BSDF_REFLECTION:
        context.addArgument(Argument("vec3", incident));
        context.addArgument(Argument("vec3", outgoing));
        context.setFunctionSuffix("_reflection");
        break;
    case CONTEXT_BSDF_TRANSMISSION:
        context.addArgument(Argument("vec3", outgoing));
        context.setFunctionSuffix("_transmission");
        break;
    case CONTEXT_BSDF_INDIRECT:
        context.addArgument(Argument("vec3", outgoing));
        context.setFunctionSuffix("_indirect");
        break;
    default:
        throw ExceptionShaderGenError("Unknown bsdf context id given when generating bsdf node function calls");
    }

    ShaderNode* last = nullptr;

    // Emit function calls for all BSDF nodes used by this shader.
    // The last node will hold the final result.
    for (ShaderNode* node : shader.getGraph()->getNodes())
    {
        if (node->hasClassification(ShaderNode::Classification::BSDF) && shaderNode.isUsedClosure(node))
        {
            // Check if the node is defined in this context.
            if (node->getContextIDs().count(bsdfContext))
            {
                shader.addFunctionCall(node, context, *this);
            }
            else
            {
                // Node is not defined in this context so just 
                // emit the output variable set to default value.
                shader.beginLine();
                emitOutput(context, node->getOutput(), true, true, shader);
                shader.endLine();
            }
            last = node;
        }
    }

    if (last)
    {
        bsdf = last->getOutput()->variable;
    }
}

void GlslShaderGenerator::emitEdfNodes(const ShaderNode& shaderNode, const string& normalDir, const string& evalDir, Shader& shader, string& edf)
{
    GenContext context(CONTEXT_EDF);

    // Set extra arguments according to the given directions
    context.addArgument(Argument("vec3", normalDir));
    context.addArgument(Argument("vec3", evalDir));

    edf = "EDF(0.0)";

    ShaderNode* last = nullptr;

    // Emit function calls for all EDF nodes used by this shader
    // The last node will hold the final result
    for (ShaderNode* node : shader.getGraph()->getNodes())
    {
        if (node->hasClassification(ShaderNode::Classification::EDF) && shaderNode.isUsedClosure(node))
        {
            shader.addFunctionCall(node, context, *this);
            last = node;
        }
    }

    if (last)
    {
        edf = last->getOutput()->variable;
    }
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
    else
    {
        // Can't understand other types. Just return black.
        variable = "vec4(0.0, 0.0, 0.0, 1.0)";
    }
}

void GlslShaderGenerator::emitVariable(const Shader::Variable& variable, const string& qualifier, Shader& shader)
{
    // A file texture input needs special handling on GLSL
    if (variable.type == Type::FILENAME)
    {
        // Samplers must always be uniforms
        shader.addLine("uniform sampler2D " + variable.name);
    }
    else
    {
        const string& type = _syntax->getTypeName(variable.type);

        string line = qualifier + " " + type + " " + variable.name;
        if (variable.semantic.length())
            line += " : " + variable.semantic;
        if (variable.value)
        {
            // If an array we need an array qualifier (suffix) for the variable name
            string arraySuffix;
            variable.getArraySuffix(arraySuffix);
            line += arraySuffix;

            line += " = " + _syntax->getValue(variable.type, *variable.value, true);
        }
        else
        {
            line += " = " + _syntax->getDefaultValue(variable.type, true);
        }
        shader.addLine(line);
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
    return ParentClass::createCompoundImplementation(impl);
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
