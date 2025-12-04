//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenMsl/MslShaderGenerator.h>
#include <MaterialXGenMsl/MslSyntax.h>

#include <MaterialXGenHw/HwLightShaders.h>
#include <MaterialXGenHw/Nodes/HwImageNode.h>
#include <MaterialXGenHw/Nodes/HwGeomColorNode.h>
#include <MaterialXGenHw/Nodes/HwGeomPropValueNode.h>
#include <MaterialXGenHw/Nodes/HwTexCoordNode.h>
#include <MaterialXGenHw/Nodes/HwTransformNode.h>
#include <MaterialXGenHw/Nodes/HwPositionNode.h>
#include <MaterialXGenHw/Nodes/HwNormalNode.h>
#include <MaterialXGenHw/Nodes/HwTangentNode.h>
#include <MaterialXGenHw/Nodes/HwBitangentNode.h>
#include <MaterialXGenHw/Nodes/HwFrameNode.h>
#include <MaterialXGenHw/Nodes/HwTimeNode.h>
#include <MaterialXGenHw/Nodes/HwViewDirectionNode.h>
#include <MaterialXGenHw/Nodes/HwLightNode.h>
#include <MaterialXGenHw/Nodes/HwLightSamplerNode.h>
#include <MaterialXGenHw/Nodes/HwLightShaderNode.h>
#include <MaterialXGenHw/Nodes/HwNumLightsNode.h>
#include <MaterialXGenHw/Nodes/HwSurfaceNode.h>

#include <MaterialXGenShader/Exception.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Nodes/MaterialNode.h>

#include "MslResourceBindingContext.h"

#include <cctype>

MATERIALX_NAMESPACE_BEGIN

const string MslShaderGenerator::TARGET = "genmsl";
const string MslShaderGenerator::VERSION = "2.3";

//
// MslShaderGenerator methods
//

MslShaderGenerator::MslShaderGenerator(TypeSystemPtr typeSystem) :
    HwShaderGenerator(typeSystem, MslSyntax::create(typeSystem))
{
    //
    // Register all custom node implementation classes
    //

    StringVec elementNames;

    // <!-- <position> -->
    registerImplementation("IM_position_vector3_" + MslShaderGenerator::TARGET, HwPositionNode::create);
    // <!-- <normal> -->
    registerImplementation("IM_normal_vector3_" + MslShaderGenerator::TARGET, HwNormalNode::create);
    // <!-- <tangent> -->
    registerImplementation("IM_tangent_vector3_" + MslShaderGenerator::TARGET, HwTangentNode::create);
    // <!-- <bitangent> -->
    registerImplementation("IM_bitangent_vector3_" + MslShaderGenerator::TARGET, HwBitangentNode::create);
    // <!-- <texcoord> -->
    registerImplementation("IM_texcoord_vector2_" + MslShaderGenerator::TARGET, HwTexCoordNode::create);
    registerImplementation("IM_texcoord_vector3_" + MslShaderGenerator::TARGET, HwTexCoordNode::create);
    // <!-- <geomcolor> -->
    registerImplementation("IM_geomcolor_float_" + MslShaderGenerator::TARGET, HwGeomColorNode::create);
    registerImplementation("IM_geomcolor_color3_" + MslShaderGenerator::TARGET, HwGeomColorNode::create);
    registerImplementation("IM_geomcolor_color4_" + MslShaderGenerator::TARGET, HwGeomColorNode::create);
    // <!-- <geompropvalue> -->
    elementNames = {
        "IM_geompropvalue_integer_" + MslShaderGenerator::TARGET,
        "IM_geompropvalue_float_" + MslShaderGenerator::TARGET,
        "IM_geompropvalue_color3_" + MslShaderGenerator::TARGET,
        "IM_geompropvalue_color4_" + MslShaderGenerator::TARGET,
        "IM_geompropvalue_vector2_" + MslShaderGenerator::TARGET,
        "IM_geompropvalue_vector3_" + MslShaderGenerator::TARGET,
        "IM_geompropvalue_vector4_" + MslShaderGenerator::TARGET,
    };
    registerImplementation(elementNames, HwGeomPropValueNode::create);
    registerImplementation("IM_geompropvalue_boolean_" + MslShaderGenerator::TARGET, HwGeomPropValueNodeAsUniform::create);
    registerImplementation("IM_geompropvalue_string_" + MslShaderGenerator::TARGET, HwGeomPropValueNodeAsUniform::create);
    registerImplementation("IM_geompropvalue_filename_" + MslShaderGenerator::TARGET, HwGeomPropValueNodeAsUniform::create);


    // <!-- <frame> -->
    registerImplementation("IM_frame_float_" + MslShaderGenerator::TARGET, HwFrameNode::create);
    // <!-- <time> -->
    registerImplementation("IM_time_float_" + MslShaderGenerator::TARGET, HwTimeNode::create);
    // <!-- <viewdirection> -->
    registerImplementation("IM_viewdirection_vector3_" + MslShaderGenerator::TARGET, HwViewDirectionNode::create);

    // <!-- <surface> -->
    registerImplementation("IM_surface_" + MslShaderGenerator::TARGET, HwSurfaceNode::create);

    // <!-- <light> -->
    registerImplementation("IM_light_" + MslShaderGenerator::TARGET, HwLightNode::create);

    // <!-- <point_light> -->
    registerImplementation("IM_point_light_" + MslShaderGenerator::TARGET, HwLightShaderNode::create);
    // <!-- <directional_light> -->
    registerImplementation("IM_directional_light_" + MslShaderGenerator::TARGET, HwLightShaderNode::create);
    // <!-- <spot_light> -->
    registerImplementation("IM_spot_light_" + MslShaderGenerator::TARGET, HwLightShaderNode::create);

    // <!-- <ND_transformpoint> ->
    registerImplementation("IM_transformpoint_vector3_" + MslShaderGenerator::TARGET, HwTransformPointNode::create);

    // <!-- <ND_transformvector> ->
    registerImplementation("IM_transformvector_vector3_" + MslShaderGenerator::TARGET, HwTransformVectorNode::create);

    // <!-- <ND_transformnormal> ->
    registerImplementation("IM_transformnormal_vector3_" + MslShaderGenerator::TARGET, HwTransformNormalNode::create);

    // <!-- <image> -->
    elementNames = {
        "IM_image_float_" + MslShaderGenerator::TARGET,
        "IM_image_color3_" + MslShaderGenerator::TARGET,
        "IM_image_color4_" + MslShaderGenerator::TARGET,
        "IM_image_vector2_" + MslShaderGenerator::TARGET,
        "IM_image_vector3_" + MslShaderGenerator::TARGET,
        "IM_image_vector4_" + MslShaderGenerator::TARGET,
    };
    registerImplementation(elementNames, HwImageNode::create);

    // <!-- <surfacematerial> -->
    registerImplementation("IM_surfacematerial_" + MslShaderGenerator::TARGET, MaterialNode::create);

    _lightSamplingNodes.push_back(ShaderNode::create(nullptr, "numActiveLightSources", HwNumLightsNode::create()));
    _lightSamplingNodes.push_back(ShaderNode::create(nullptr, "sampleLightSource", HwLightSamplerNode::create()));

    _tokenSubstitutions[HW::T_CLOSURE_DATA_CONSTRUCTOR] = "{closureType, L, V, N, P, occlusion}";

    _tokenSubstitutions[HW::T_TEX_SAMPLER_SAMPLER2D]    = HW::TEX_SAMPLER_SAMPLER2D_MSL;
    _tokenSubstitutions[HW::T_TEX_SAMPLER_SIGNATURE]    = HW::TEX_SAMPLER_SIGNATURE_MSL;
}

ShaderPtr MslShaderGenerator::generate(const string& name, ElementPtr element, GenContext& context) const
{
    ShaderPtr shader = createShader(name, element, context);

    // Request fixed floating-point notation for consistency across targets.
    ScopedFloatFormatting fmt(Value::FloatFormatFixed);

    // Make sure we initialize/reset the binding context before generation.
    HwResourceBindingContextPtr resourceBindingCtx = getResourceBindingContext(context);
    if (!resourceBindingCtx)
    {
        context.pushUserData(HW::USER_DATA_BINDING_CONTEXT, MslResourceBindingContext::create());
        resourceBindingCtx = context.getUserData<HwResourceBindingContext>(HW::USER_DATA_BINDING_CONTEXT);
    }

    // Make sure we initialize/reset the binding context before generation.
    if (resourceBindingCtx)
    {
        resourceBindingCtx->initialize();
    }

    // Emit code for vertex shader stage
    ShaderStage& vs = shader->getStage(Stage::VERTEX);
    emitVertexStage(shader->getGraph(), context, vs);
    replaceTokens(_tokenSubstitutions, vs);

    MetalizeGeneratedShader(vs);

    // Emit code for pixel shader stage
    ShaderStage& ps = shader->getStage(Stage::PIXEL);
    emitPixelStage(shader->getGraph(), context, ps);
    replaceTokens(_tokenSubstitutions, ps);

    MetalizeGeneratedShader(ps);

    return shader;
}

void MslShaderGenerator::MetalizeGeneratedShader(ShaderStage& shaderStage) const
{
    std::string sourceCode = shaderStage.getSourceCode();

    // Used to convert shared code between GLSL pass by reference parameters to MSL pass by reference.
    // Converts "inout/out Type variableName" to "thread Type& variableName"
    size_t pos = 0;
    {
        std::array<string, 2> refKeywords = { "out", "inout" };
        for (const auto& keyword : refKeywords)
        {
            pos = sourceCode.find(keyword);
            while (pos != std::string::npos)
            {
                char preceding = sourceCode[pos - 1], succeeding = sourceCode[pos + keyword.length()];
                bool isOutKeyword =
                    (preceding == '(' || preceding == ',' || std::isspace(preceding)) &&
                    std::isspace(succeeding) &&
                    succeeding != '\n';
                size_t beg = pos;
                pos += keyword.length();
                if (isOutKeyword)
                {
                    while (std::isspace(sourceCode[pos]))
                    {
                        ++pos;
                    }
                    size_t typename_beg = pos;
                    while (!std::isspace(sourceCode[pos]))
                    {
                        ++pos;
                    }
                    size_t typename_end = pos;
                    std::string typeName = sourceCode.substr(typename_beg, typename_end - typename_beg);

                    while (std::isspace(sourceCode[pos]))
                    {
                        ++pos;
                    }
                    size_t varname_beg = pos;
                    while (!std::isspace(sourceCode[pos]) && sourceCode[pos] != '\n' && sourceCode[pos] != ',' && sourceCode[pos] != ')' )
                    {
                        ++pos;
                    }
                    size_t varname_end = pos;
                    std::string varName = sourceCode.substr(varname_beg, varname_end - varname_beg);

                    if (varName.find('[') != std::string::npos)
                    {
                        // if the variable is an array then we don't need to declare it as a reference,
                        // we will effectively just be passing the pointer to the array
                        sourceCode.replace(beg, typename_end - beg, "thread " + typeName);
                    }
                    else
                    {
                        sourceCode.replace(beg, typename_end - beg, "thread " + typeName + " &");
                    }
                }
                pos = sourceCode.find(keyword, pos);
            }
        }
    }

    // Renames GLSL constructs that are used in shared code to MSL equivalent constructs.
    std::unordered_map<string, string> replaceTokens;
    replaceTokens["sampler2D"] = "MetalTexture";
    replaceTokens["dFdy"] = "dfdy";
    replaceTokens["dFdx"] = "dfdx";

    replaceTokens["vec2"] = "float2";
    replaceTokens["vec3"] = "float3";
    replaceTokens["vec4"] = "float4";
    replaceTokens["ivec2"] = "int2";
    replaceTokens["ivec3"] = "int3";
    replaceTokens["ivec4"] = "int4";
    replaceTokens["uvec2"] = "uint2";
    replaceTokens["uvec3"] = "uint3";
    replaceTokens["uvec4"] = "uint4";
    replaceTokens["bvec2"] = "bool2";
    replaceTokens["bvec3"] = "bool3";
    replaceTokens["bvec4"] = "bool4";
    replaceTokens["mat2"] = "float2x2";
    replaceTokens["mat3"] = "float3x3";
    replaceTokens["mat4"] = "float4x4";

    auto isAllowedAfterToken = [](char ch) -> bool
    {
        return std::isspace(ch) || ch == '(' || ch == ')' || ch == ',';
    };

    auto isAllowedBeforeToken = [](char ch) -> bool
    {
        return std::isspace(ch) || ch == '(' || ch == ',' || ch == '+' || ch == '-';
    };

    for (const auto& t : replaceTokens)
    {
        pos = sourceCode.find(t.first);
        while (pos != std::string::npos)
        {
            bool isOutKeyword = isAllowedBeforeToken(sourceCode[pos - 1]);
            size_t beg = pos;
            pos += t.first.length();
            isOutKeyword &= isAllowedAfterToken(sourceCode[pos]);

            if (isOutKeyword)
            {
                sourceCode.replace(beg, t.first.length(), t.second);
                pos = sourceCode.find(t.first, beg + t.second.length());
            }
            else
            {
                pos = sourceCode.find(t.first, beg + t.first.length());
            }
        }
    }

    shaderStage.setSourceCode(sourceCode);
}

void MslShaderGenerator::emitGlobalVariables(GenContext& context,
                                             ShaderStage& stage,
                                             EmitGlobalScopeContext situation,
                                             bool isVertexShader, bool needsLightData) const
{
    int tex_slot = 0;
    int buffer_slot = isVertexShader ? std::max(static_cast<int>(stage.getInputBlock(HW::VERTEX_INPUTS).size()), 0) : 0;

    bool entryFunctionArgs =
        situation == EMIT_GLOBAL_SCOPE_CONTEXT_ENTRY_FUNCTION_RESOURCES;
    bool globalContextInit =
        situation == EMIT_GLOBAL_SCOPE_CONTEXT_MEMBER_INIT;
    bool globalContextMembers =
        situation == EMIT_GLOBAL_SCOPE_CONTEXT_MEMBER_DECL;
    bool globalContextConstructorParams =
        situation == EMIT_GLOBAL_SCOPE_CONTEXT_CONSTRUCTOR_ARGS;
    bool globalContextConstructorInit =
        situation == EMIT_GLOBAL_SCOPE_CONTEXT_CONSTRUCTOR_INIT;

    std::string separator = "";
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        if (globalContextMembers)
        {
            emitLine("float4 gl_FragCoord", stage);
        }
        if (globalContextConstructorInit)
        {
            emitString("gl_FragCoord(", stage);
            emitLine(stage.getInputBlock(HW::VERTEX_DATA).getInstance() + ".pos)", stage, false);
            separator = ",";
        }
    }

    {
        auto vertex_inputs = isVertexShader ? stage.getInputBlock(HW::VERTEX_INPUTS)
                                            : stage.getInputBlock(HW::VERTEX_DATA);
        if (!entryFunctionArgs)
        {
            if (isVertexShader)
            {
                for (const auto& it : vertex_inputs.getVariableOrder())
                {
                    emitString(separator, stage);
                    if (globalContextInit)
                    {
                        emitString(vertex_inputs.getInstance() + "." + it->getName(), stage);
                    }
                    else if (globalContextMembers || globalContextConstructorParams)
                    {
                        emitLine(_syntax->getTypeName(it->getType()) + " " + it->getName(), stage, !globalContextConstructorParams);
                    }
                    else if (globalContextConstructorInit)
                    {
                        emitLine(it->getName() + "(" + it->getName() + ")", stage, false);
                    }

                    if (globalContextInit || globalContextConstructorParams || globalContextConstructorInit)
                        separator = ", ";
                    else if (globalContextMembers)
                        separator = "\n";
                }
            }
            else
            {
                emitString(separator, stage);
                if (globalContextInit)
                {
                    emitString(vertex_inputs.getInstance(), stage);
                    separator = ", ";
                }
                else if (globalContextMembers || globalContextConstructorParams)
                {
                    emitLine(vertex_inputs.getName() + " " + vertex_inputs.getInstance(), stage, !globalContextConstructorParams);
                    if (globalContextConstructorParams)
                        separator = ", ";
                    else
                        separator = "\n";
                }
                else if (globalContextConstructorInit)
                {
                    emitLine(vertex_inputs.getInstance() + "(" + vertex_inputs.getInstance() + ")", stage, false);
                    separator = ", ";
                }
            }
        }
        else
        {
            emitString(vertex_inputs.getName() + " " + vertex_inputs.getInstance() + " [[ stage_in ]]", stage);
            separator = ", ";
        }
    }

    // Add all uniforms
    for (const auto& it : stage.getUniformBlocks())
    {
        const VariableBlock& uniforms = *it.second;
        bool isLightData = uniforms.getName() == HW::LIGHT_DATA;

        if (!needsLightData && isLightData)
            continue;

        if (isLightData)
        {
            emitString(separator, stage);
            if (entryFunctionArgs)
            {
                emitString(_syntax->getUniformQualifier() + " " +
                               uniforms.getName() + "_" + stage.getName() + "& " +
                               uniforms.getInstance() +
                               "[[ buffer(" + std::to_string(buffer_slot++) + ") ]]",
                           stage);
            }
            else if (globalContextInit)
            {
                emitLine(uniforms.getInstance() + "." + uniforms.getInstance(), stage, false);
            }
            else if (globalContextMembers || globalContextConstructorParams)
            {
                const string structArraySuffix = "[" + HW::LIGHT_DATA_MAX_LIGHT_SOURCES + "]";
                emitLine((globalContextConstructorParams ? (_syntax->getUniformQualifier() + " ") : std::string()) +
                             uniforms.getName() + " " +
                             uniforms.getInstance() +
                             structArraySuffix,
                         stage, !globalContextConstructorParams);
            }
            else if (globalContextConstructorInit)
            {
                const unsigned int maxLights = std::max(1u, context.getOptions().hwMaxActiveLightSources);
                emitLine(uniforms.getInstance(), stage, false);
                emitScopeBegin(stage);
                for (unsigned int l = 0; l < maxLights; ++l)
                {
                    emitString(l == 0 ? "" : ", ", stage);
                    emitLine(uniforms.getInstance() +
                                 "[" + std::to_string(l) + "]",
                             stage, false);
                }
                emitScopeEnd(stage);
            }
        }
        else
        {
            if (!entryFunctionArgs)
            {
                if (!uniforms.empty())
                {
                    for (size_t i = 0; i < uniforms.size(); ++i)
                    {
                        if (uniforms[i]->getType() != Type::FILENAME)
                        {
                            emitLineBegin(stage);
                            emitString(separator, stage);
                            if (globalContextInit)
                            {
                                emitString(uniforms.getInstance() + "." + uniforms[i]->getVariable(), stage);
                            }
                            else if (globalContextMembers || globalContextConstructorParams)
                            {
                                emitLine(_syntax->getTypeName(uniforms[i]->getType()) + " " + uniforms[i]->getVariable(), stage, !globalContextConstructorParams);
                            }
                            else if (globalContextConstructorInit)
                            {
                                emitLine(uniforms[i]->getVariable() + "(" + uniforms[i]->getVariable() + ")", stage, false);
                            }
                            emitLineEnd(stage, false);
                        }
                        else
                        {
                            if (globalContextInit)
                            {
                                emitString(separator, stage);
                                emitString("MetalTexture", stage);
                                emitScopeBegin(stage);
                                emitString(TEXTURE_NAME(uniforms[i]->getVariable()), stage);
                                emitString(separator, stage);
                                emitString(SAMPLER_NAME(uniforms[i]->getVariable()), stage);
                                emitScopeEnd(stage);
                            }
                            else if (globalContextMembers || globalContextConstructorParams)
                            {
                                emitString(separator, stage);
                                emitVariableDeclaration(uniforms[i], EMPTY_STRING, context, stage, false);
                                emitString(globalContextConstructorParams ? "" : ";", stage);
                            }
                            else if (globalContextConstructorInit)
                            {
                                emitString(separator, stage);
                                emitLine(uniforms[i]->getVariable() + "(" + uniforms[i]->getVariable() + ")", stage, false);
                            }
                        }

                        if (globalContextInit || globalContextConstructorParams || globalContextConstructorInit)
                            separator = ", ";
                        else if (globalContextMembers)
                            separator = "\n";
                    }
                }
            }
            else
            {
                if (!uniforms.empty())
                {
                    bool hasUniforms = false;
                    for (const ShaderPort* uniform : uniforms.getVariableOrder())
                    {
                        if (uniform->getType() == Type::FILENAME)
                        {
                            emitString(separator, stage);
                            emitString("texture2d<float> " + TEXTURE_NAME(uniform->getVariable()), stage);
                            emitString(" [[texture(" + std::to_string(tex_slot) + ")]], ", stage);
                            emitString("sampler " + SAMPLER_NAME(uniform->getVariable()), stage);
                            emitString(" [[sampler(" + std::to_string(tex_slot++) + ")]]", stage);
                            emitLineEnd(stage, false);
                        }
                        else
                        {
                            hasUniforms = true;
                        }
                    }

                    if (hasUniforms)
                    {
                        emitString(separator, stage);
                        emitString(_syntax->getUniformQualifier() + " " +
                                       uniforms.getName() + "& " +
                                       uniforms.getInstance() +
                                       "[[ buffer(" + std::to_string(buffer_slot++) + ") ]]",
                                   stage);
                    }
                }
            }
        }

        if (globalContextInit || entryFunctionArgs || globalContextConstructorParams || globalContextConstructorInit)
            separator = ", ";
        else
            separator = "\n";
    }

    if (!isVertexShader)
    {
        const VariableBlock& outputs = stage.getOutputBlock(HW::PIXEL_OUTPUTS);
        for (auto& it : outputs.getVariableOrder())
        {
            if (globalContextMembers)
            {
                emitLine(_syntax->getTypeName(it->getType()) + " " + it->getVariable(), stage, true);
            }
        }
    }
};

void MslShaderGenerator::emitVertexStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    HwResourceBindingContextPtr resourceBindingCtx = getResourceBindingContext(context);

    emitDirectives(context, stage);
    if (resourceBindingCtx)
    {
        resourceBindingCtx->emitDirectives(context, stage);
    }
    emitLineBreak(stage);

    emitConstantBufferDeclarations(context, resourceBindingCtx, stage);

    // Add vertex inputs
    emitInputs(context, stage);

    // Add vertex data outputs block
    emitOutputs(context, stage);

    emitLine("struct GlobalContext", stage, false);
    emitScopeBegin(stage);
    {
        emitLine("GlobalContext(", stage, false);
        emitGlobalVariables(context, stage, EMIT_GLOBAL_SCOPE_CONTEXT_CONSTRUCTOR_ARGS, true, false);
        emitLine(") : ", stage, false);
        emitGlobalVariables(context, stage, EMIT_GLOBAL_SCOPE_CONTEXT_CONSTRUCTOR_INIT, true, false);
        emitLine("{}", stage, false);

        // Add common math functions
        emitLibraryInclude("stdlib/genmsl/lib/mx_math.metal", context, stage);
        emitLineBreak(stage);

        emitGlobalVariables(context, stage, EMIT_GLOBAL_SCOPE_CONTEXT_MEMBER_DECL, true, false);

        emitFunctionDefinitions(graph, context, stage);

        const VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        emitLine(vertexData.getName() + " VertexMain()", stage, false);
        emitScopeBegin(stage);
        {
            emitLine(vertexData.getName() + " " + vertexData.getInstance(), stage, true);
            emitLine("float4 hPositionWorld = " + HW::T_WORLD_MATRIX + " * float4(" + HW::T_IN_POSITION + ", 1.0)", stage);
            emitLine(vertexData.getInstance() + ".pos" + " = " + HW::T_VIEW_PROJECTION_MATRIX + " * hPositionWorld", stage);
            emitFunctionCalls(graph, context, stage);
            emitLineBreak(stage);
            emitLine("return " + vertexData.getInstance(), stage, true);

            // Emit all function calls in order
            for (const ShaderNode* node : graph.getNodes())
            {
                emitFunctionCall(*node, context, stage);
            }

            emitFunctionBodyEnd(graph, context, stage);
        }
    }
    emitScopeEnd(stage, true, true);

    // Add main function
    setFunctionName("VertexMain", stage);
    const VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
    emitLine("vertex " + vertexData.getName() + " VertexMain(", stage, false);
    emitGlobalVariables(context, stage, EMIT_GLOBAL_SCOPE_CONTEXT_ENTRY_FUNCTION_RESOURCES, true, false);
    emitLine(")", stage, false);
    emitScopeBegin(stage);
    {
        emitString("\tGlobalContext ctx {", stage);
        emitGlobalVariables(context, stage, EMIT_GLOBAL_SCOPE_CONTEXT_MEMBER_INIT, true, false);
        emitLine("}", stage, true);
        emitLine(vertexData.getName() + " outVertex = ctx.VertexMain()", stage, true);
        emitLine("outVertex.pos.y = -outVertex.pos.y", stage, true);
        emitLine("return outVertex", stage, true);
    }
    emitScopeEnd(stage);
    emitLineBreak(stage);
}

void MslShaderGenerator::emitSpecularEnvironment(GenContext& context, ShaderStage& stage) const
{
    int specularMethod = context.getOptions().hwSpecularEnvironmentMethod;
    if (specularMethod == SPECULAR_ENVIRONMENT_FIS)
    {
        emitLibraryInclude("pbrlib/genglsl/lib/mx_environment_fis.glsl", context, stage);
    }
    else if (specularMethod == SPECULAR_ENVIRONMENT_PREFILTER)
    {
        emitLibraryInclude("pbrlib/genglsl/lib/mx_environment_prefilter.glsl", context, stage);
    }
    else if (specularMethod == SPECULAR_ENVIRONMENT_NONE)
    {
        emitLibraryInclude("pbrlib/genglsl/lib/mx_environment_none.glsl", context, stage);
    }
    else
    {
        throw ExceptionShaderGenError("Invalid hardware specular environment method specified: '" + std::to_string(specularMethod) + "'");
    }
    emitLineBreak(stage);
}

void MslShaderGenerator::emitTransmissionRender(GenContext& context, ShaderStage& stage) const
{
    int transmissionMethod = context.getOptions().hwTransmissionRenderMethod;
    if (transmissionMethod == TRANSMISSION_REFRACTION)
    {
        emitLibraryInclude("pbrlib/genglsl/lib/mx_transmission_refract.glsl", context, stage);
    }
    else if (transmissionMethod == TRANSMISSION_OPACITY)
    {
        emitLibraryInclude("pbrlib/genglsl/lib/mx_transmission_opacity.glsl", context, stage);
    }
    else
    {
        throw ExceptionShaderGenError("Invalid transmission render specified: '" + std::to_string(transmissionMethod) + "'");
    }
    emitLineBreak(stage);
}

void MslShaderGenerator::emitDirectives(GenContext&, ShaderStage& stage) const
{
    // Add directives
    emitLine("//Metal Shading Language version " + getVersion(), stage, false);
    emitLine("#define __METAL__ ", stage, false);
    emitLine("#include <metal_stdlib>", stage, false);
    emitLine("#include <simd/simd.h>", stage, false);
    emitLine("using namespace metal;", stage, false);

    emitLineBreak(stage);
}

void MslShaderGenerator::emitConstants(GenContext& context, ShaderStage& stage) const
{
    const VariableBlock& constants = stage.getConstantBlock();
    if (!constants.empty())
    {
        emitVariableDeclarations(constants, _syntax->getUniformQualifier(), Syntax::SEMICOLON, context, stage);
        emitLineBreak(stage);
    }
}

void MslShaderGenerator::emitConstantBufferDeclarations(GenContext& context,
                                                        HwResourceBindingContextPtr resourceBindingCtx,
                                                        ShaderStage& stage) const
{
    // Add all uniforms
    for (const auto& it : stage.getUniformBlocks())
    {
        const VariableBlock& uniforms = *it.second;
        if (!uniforms.empty())
        {
            if (uniforms.getName() == HW::LIGHT_DATA)
                continue;

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
}

void MslShaderGenerator::emitMetalTextureClass(GenContext& context, ShaderStage& stage) const
{
    emitLibraryInclude("stdlib/genmsl/lib/mx_texture.metal", context, stage);
}

void MslShaderGenerator::emitLightData(GenContext& context, ShaderStage& stage) const
{
    const VariableBlock& lightData = stage.getUniformBlock(HW::LIGHT_DATA);
    const string structArraySuffix = "[" + HW::LIGHT_DATA_MAX_LIGHT_SOURCES + "]";
    const string structName = lightData.getInstance();
    HwResourceBindingContextPtr resourceBindingCtx = getResourceBindingContext(context);
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

void MslShaderGenerator::emitInputs(GenContext& context, ShaderStage& stage, const VariableBlock& inputs) const
{
    emitComment("Inputs block: " + inputs.getName(), stage);
    emitLine("struct " + inputs.getName(), stage, false);
    emitScopeBegin(stage);

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        emitLine("float4 pos [[position]]", stage);
    }

    for (size_t i = 0; i < inputs.size(); ++i)
    {
        string line = "";
        line += context.getShaderGenerator().getSyntax().getTypeName(inputs[i]->getType());
        line += " " + inputs[i]->getName() + " ";
        DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
        {
            line += "[[attribute(";
            line += std::to_string(i);
            line += ")]]";
        };

        emitLine(line, stage, true);
    }

    emitScopeEnd(stage, true, false);
    emitLineBreak(stage);
}

void MslShaderGenerator::emitInputs(GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        const VariableBlock& vertexInputs = stage.getInputBlock(HW::VERTEX_INPUTS);
        emitInputs(context, stage, vertexInputs);
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
        emitInputs(context, stage, vertexData);
    }
}

void MslShaderGenerator::emitOutputs(GenContext& context, ShaderStage& stage) const
{
    // Add vertex inputs
    auto emitOutputsOfShaderSource = [&](const VariableBlock& outputs)
    {
        if (!outputs.empty())
        {
            emitLine("struct " + outputs.getName(), stage, false);
            emitScopeBegin(stage);
            DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
            {
                emitLine("float4 pos [[position]]", stage, true);
            }
            emitVariableDeclarations(outputs, EMPTY_STRING, Syntax::SEMICOLON, context, stage, false);
            emitScopeEnd(stage, true, false);
            emitLineBreak(stage);
            emitLineBreak(stage);
        }
        else
        {
            emitLine("struct VertexData", stage, false);
            emitScopeBegin(stage);
            emitLine("float4 pos [[position]]", stage, true);
            emitScopeEnd(stage, true, false);
            emitLineBreak(stage);
            emitLineBreak(stage);
        }
    };

    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        const VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        emitOutputsOfShaderSource(vertexData);
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        emitComment("Pixel shader outputs", stage);
        const VariableBlock& outputs = stage.getOutputBlock(HW::PIXEL_OUTPUTS);
        emitOutputsOfShaderSource(outputs);
    }
}

HwResourceBindingContextPtr MslShaderGenerator::getResourceBindingContext(GenContext& context) const
{
    return context.getUserData<HwResourceBindingContext>(HW::USER_DATA_BINDING_CONTEXT);
}

string MslShaderGenerator::getVertexDataPrefix(const VariableBlock& vertexData) const
{
    return vertexData.getInstance() + ".";
}

void MslShaderGenerator::emitMathMatrixScalarMathOperators(GenContext& context, ShaderStage& stage) const
{
    emitLibraryInclude("stdlib/genmsl/lib/mx_matscalaroperators.metal", context, stage);
}

void MslShaderGenerator::emitPixelStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    HwResourceBindingContextPtr resourceBindingCtx = getResourceBindingContext(context);

    // Add directives
    emitDirectives(context, stage);
    if (resourceBindingCtx)
    {
        resourceBindingCtx->emitDirectives(context, stage);
    }
    emitLineBreak(stage);

    emitMetalTextureClass(context, stage);

    // Add type definitions
    emitTypeDefinitions(context, stage);

    emitConstantBufferDeclarations(context, resourceBindingCtx, stage);

    // Add all constants
    emitConstants(context, stage);

    // Add vertex data inputs block
    emitInputs(context, stage);

    // Add the pixel shader output. This needs to be a float4 for rendering
    // and upstream connection will be converted to float4 if needed in emitFinalOutput()
    emitOutputs(context, stage);

    // Determine whether lighting is required
    bool lighting = requiresLighting(graph);

    // Define directional albedo approach
    if (lighting || context.getOptions().hwWriteAlbedoTable)
    {
        emitLine("#define DIRECTIONAL_ALBEDO_METHOD " + std::to_string(int(context.getOptions().hwDirectionalAlbedoMethod)), stage, false);
        emitLineBreak(stage);
    }

    // Define Airy Fresnel iterations
    emitLine("#define AIRY_FRESNEL_ITERATIONS " + std::to_string(context.getOptions().hwAiryFresnelIterations), stage, false);
    emitLineBreak(stage);

    // Add lighting support
    if (lighting)
    {
        if (context.getOptions().hwMaxActiveLightSources > 0)
        {
            const unsigned int maxLights = std::max(1u, context.getOptions().hwMaxActiveLightSources);
            emitLine("#define " + HW::LIGHT_DATA_MAX_LIGHT_SOURCES + " " + std::to_string(maxLights), stage, false);
        }

        if (context.getOptions().hwMaxActiveLightSources > 0)
        {
            emitLightData(context, stage);
        }
    }

    bool needsLightBuffer = lighting && context.getOptions().hwMaxActiveLightSources > 0;

    emitMathMatrixScalarMathOperators(context, stage);
    emitLine("struct GlobalContext", stage, false);
    emitScopeBegin(stage);
    {
        emitLine("GlobalContext(", stage, false);
        emitGlobalVariables(context, stage, EMIT_GLOBAL_SCOPE_CONTEXT_CONSTRUCTOR_ARGS, false, needsLightBuffer);
        emitLine(") : ", stage, false);
        emitGlobalVariables(context, stage, EMIT_GLOBAL_SCOPE_CONTEXT_CONSTRUCTOR_INIT, false, needsLightBuffer);
        emitLine("{}", stage, false);

        // Add common math functions
        emitLibraryInclude("stdlib/genmsl/lib/mx_math.metal", context, stage);
        emitLineBreak(stage);

        if (lighting)
        {
            emitSpecularEnvironment(context, stage);
            emitTransmissionRender(context, stage);
        }

        emitGlobalVariables(context, stage, EMIT_GLOBAL_SCOPE_CONTEXT_MEMBER_DECL, false, needsLightBuffer);

        // Add shadowing support
        bool shadowing = (lighting && context.getOptions().hwShadowMap) ||
                         context.getOptions().hwWriteDepthMoments;
        if (shadowing)
        {
            emitLibraryInclude("pbrlib/genglsl/lib/mx_shadow.glsl", context, stage);
            emitLibraryInclude("pbrlib/genmsl/lib/mx_shadow_platform.metal", context, stage);
        }

        // Emit directional albedo table code.
        if (context.getOptions().hwWriteAlbedoTable)
        {
            emitLibraryInclude("pbrlib/genglsl/lib/mx_generate_albedo_table.glsl", context, stage);
            emitLineBreak(stage);
        }

        // Emit environment prefiltering code
        if (context.getOptions().hwWriteEnvPrefilter)
        {
            emitLibraryInclude("pbrlib/genglsl/lib/mx_generate_prefilter_env.glsl", context, stage);
            emitLineBreak(stage);
        }

        // Set the include file to use for uv transformations,
        // depending on the vertical flip flag.
        if (context.getOptions().fileTextureVerticalFlip)
        {
            _tokenSubstitutions[ShaderGenerator::T_FILE_TRANSFORM_UV] = "mx_transform_uv_vflip.glsl";
        }
        else
        {
            _tokenSubstitutions[ShaderGenerator::T_FILE_TRANSFORM_UV] = "mx_transform_uv.glsl";
        }

        emitLightFunctionDefinitions(graph, context, stage);

        // Emit function definitions for all nodes in the graph.
        emitFunctionDefinitions(graph, context, stage);

        const ShaderGraphOutputSocket* outputSocket = graph.getOutputSocket();

        // Add main function
        const VariableBlock& outputs = stage.getOutputBlock(HW::PIXEL_OUTPUTS);
        emitLine(outputs.getName() + " FragmentMain()", stage, false);
        emitFunctionBodyBegin(graph, context, stage);

        if (graph.hasClassification(ShaderNode::Classification::CLOSURE) &&
            !graph.hasClassification(ShaderNode::Classification::SHADER))
        {
            // Handle the case where the graph is a direct closure.
            // We don't support rendering closures without attaching
            // to a surface shader, so just output black.
            emitLine(outputSocket->getVariable() + " = float4(0.0, 0.0, 0.0, 1.0)", stage);
        }
        else if (context.getOptions().hwWriteDepthMoments)
        {
            emitLine(outputSocket->getVariable() + " = float4(mx_compute_depth_moments(), 0.0, 1.0)", stage);
        }
        else if (context.getOptions().hwWriteAlbedoTable)
        {
            emitLine(outputSocket->getVariable() + " = float4(mx_generate_dir_albedo_table(), 1.0)", stage);
        }
        else if (context.getOptions().hwWriteEnvPrefilter)
        {
            emitLine(outputSocket->getVariable() + " = float4(mx_generate_prefilter_env(), 1.0)", stage);
        }
        else
        {
            // Add all function calls.
            //
            // Surface shaders need special handling.
            if (graph.hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE))
            {
                // Emit all texturing nodes. These are inputs to any
                // closure/shader nodes and need to be emitted first.
                emitFunctionCalls(graph, context, stage, ShaderNode::Classification::TEXTURE);

                // Emit function calls for "root" closure/shader nodes.
                // These will internally emit function calls for any dependent closure nodes upstream.
                for (ShaderGraphOutputSocket* socket : graph.getOutputSockets())
                {
                    if (socket->getConnection())
                    {
                        const ShaderNode* upstream = socket->getConnection()->getNode();
                        if (upstream->getParent() == &graph &&
                            (upstream->hasClassification(ShaderNode::Classification::CLOSURE) ||
                             upstream->hasClassification(ShaderNode::Classification::SHADER)))
                        {
                            emitFunctionCall(*upstream, context, stage);
                        }
                    }
                }
            }
            else
            {
                // No surface shader graph so just generate all
                // function calls in order.
                emitFunctionCalls(graph, context, stage);
            }

            // Emit final output
            const ShaderOutput* outputConnection = outputSocket->getConnection();
            if (outputConnection)
            {
                string finalOutput = outputConnection->getVariable();

                if (graph.hasClassification(ShaderNode::Classification::SURFACE))
                {
                    if (context.getOptions().hwTransparency)
                    {
                        emitLine("float outAlpha = clamp(1.0 - dot(" + finalOutput + ".transparency, float3(0.3333)), 0.0, 1.0)", stage);
                        emitLine(outputSocket->getVariable() + " = float4(" + finalOutput + ".color, outAlpha)", stage);
                        emitLine("if (outAlpha < " + HW::T_ALPHA_THRESHOLD + ")", stage, false);
                        emitScopeBegin(stage);
                        emitLine("discard_fragment()", stage);
                        emitScopeEnd(stage);
                    }
                    else
                    {
                        emitLine(outputSocket->getVariable() + " = float4(" + finalOutput + ".color, 1.0)", stage);
                    }
                }
                else
                {
                    if (!outputSocket->getType().isFloat4())
                    {
                        toVec4(outputSocket->getType(), finalOutput);
                    }
                    emitLine(outputSocket->getVariable() + " = " + finalOutput, stage);
                }
            }
            else
            {
                string outputValue = outputSocket->getValue() ?
                                    _syntax->getValue(outputSocket->getType(), *outputSocket->getValue()) :
                                    _syntax->getDefaultValue(outputSocket->getType());
                if (!outputSocket->getType().isFloat4())
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

        {
            std::string separator = "";
            emitString("return " + outputs.getName() + "{", stage);
            for (auto it : outputs.getVariableOrder())
            {
                emitString(separator + it->getVariable(), stage);
                separator = ", ";
            }
            emitLine("}", stage, true);
        }

        // End main function
        emitFunctionBodyEnd(graph, context, stage);
    }
    emitScopeEnd(stage, true, true);

    // Add main function
    {
        setFunctionName("FragmentMain", stage);
        const VariableBlock& outputs = stage.getOutputBlock(HW::PIXEL_OUTPUTS);
        emitLine("fragment " + outputs.getName() + " FragmentMain(", stage, false);
        emitGlobalVariables(context, stage, EMIT_GLOBAL_SCOPE_CONTEXT_ENTRY_FUNCTION_RESOURCES, false, needsLightBuffer);
        emitLine(")", stage, false);
        emitScopeBegin(stage);
        {
            emitString("\tGlobalContext ctx {", stage);
            emitGlobalVariables(context, stage, EMIT_GLOBAL_SCOPE_CONTEXT_MEMBER_INIT, false, needsLightBuffer);
            emitLine("}", stage, true);
            emitLine("return ctx.FragmentMain()", stage, true);
        }
        emitScopeEnd(stage);
        emitLineBreak(stage);
    }
}

void MslShaderGenerator::emitLightFunctionDefinitions(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {

        // Emit Light functions if requested
        if (requiresLighting(graph) && context.getOptions().hwMaxActiveLightSources > 0)
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
    }
}

void MslShaderGenerator::emitVariableDeclaration(const ShaderPort* variable, const string& qualifier,
                                                 GenContext&, ShaderStage& stage,
                                                 bool assignValue) const
{
    // A file texture input needs special handling on MSL
    if (variable->getType() == Type::FILENAME)
    {
        // Samplers must always be uniforms
        string str = qualifier.empty() ? EMPTY_STRING : qualifier + " ";
        emitString(str + "MetalTexture " + variable->getVariable(), stage);
    }
    else
    {
        string str = qualifier.empty() ? EMPTY_STRING : qualifier + " ";

        str += _syntax->getTypeName(variable->getType()) + " " + variable->getVariable();

        // If an array we need an array qualifier (suffix) for the variable name
        if (variable->getType().isArray() && variable->getValue())
        {
            str += _syntax->getArrayVariableSuffix(variable->getType(), *variable->getValue());
        }

        if (!variable->getSemantic().empty())
        {
            str += " : " + variable->getSemantic();
        }

        // Varying parameters of type int must be flat qualified on output from vertex stage and
        // input to pixel stage. The only way to get these is with geompropvalue_integer nodes.
        if (qualifier.empty() && variable->getType() == Type::INTEGER && !assignValue && variable->getName().rfind(HW::T_IN_GEOMPROP, 0) == 0)
        {
            str += "[[ " + MslSyntax::FLAT_QUALIFIER + " ]]";
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

MATERIALX_NAMESPACE_END
