//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenWgsl/WgslShaderGenerator.h>

#include <MaterialXGenWgsl/WgslSyntax.h>
#include <MaterialXGenWgsl/WgslResourceBindingContext.h>
#include <MaterialXGenWgsl/Nodes/WgslSurfaceNode.h>
#include <MaterialXGenWgsl/Nodes/WgslCompoundNode.h>
#include <MaterialXGenWgsl/Nodes/WgslLightNodes.h>
#include <MaterialXGenWgsl/Nodes/WgslSourceCodeNode.h>
#include <MaterialXGenWgsl/Nodes/WgslMaterialNode.h>
#include <MaterialXGenHw/HwConstants.h>
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
#include <MaterialXGenHw/Nodes/HwLightShaderNode.h>

#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGraph.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Util.h>

MATERIALX_NAMESPACE_BEGIN

const string WgslShaderGenerator::TARGET = "genwgsl";
const string WgslShaderGenerator::VERSION = "1.0";
const string WgslShaderGenerator::LIGHTDATA_TYPEVAR_STRING = "light_type";

namespace
{

// Name of the @builtin(position) member of the vertex-data struct.
const string WGSL_CLIP_POSITION = "clipPosition";

// User data to track emitted WGSL function names and prevent duplicate definitions.
class WgslEmittedFunctions : public GenUserData
{
  public:
    std::set<string> names;
};
const string WGSL_EMITTED_FUNCTIONS = "WGSL_EMITTED_FUNCTIONS";

// Scan WGSL source for "fn NAME(" and collect the function names.
void trackEmittedFunctions(const string& source, std::set<string>& names)
{
    size_t pos = 0;
    while (pos < source.size())
    {
        size_t fnPos = source.find("fn ", pos);
        if (fnPos == string::npos)
            break;
        size_t nameStart = fnPos + 3;
        size_t nameEnd = source.find_first_of("( \t\n", nameStart);
        if (nameEnd != string::npos && nameEnd > nameStart)
        {
            names.insert(source.substr(nameStart, nameEnd - nameStart));
        }
        pos = (nameEnd != string::npos) ? nameEnd : fnPos + 3;
    }
}

// Wrap a non-vec4 value in a vec4f for the final pixel output.
void toVec4Wgsl(const TypeDesc& type, string& variable)
{
    if (type.isFloat3())
        variable = "vec4f(" + variable + ", 1.0)";
    else if (type.isFloat2())
        variable = "vec4f(" + variable + ", 0.0, 1.0)";
    else if (type == Type::FLOAT || type == Type::INTEGER)
        variable = "vec4f(" + variable + ", " + variable + ", " + variable + ", 1.0)";
    else if (type == Type::BSDF || type == Type::EDF)
        variable = "vec4f(" + variable + ", 1.0)";
    else
        variable = "vec4f(0.0, 0.0, 0.0, 1.0)";
}

} // anonymous namespace

WgslShaderGenerator::WgslShaderGenerator(TypeSystemPtr typeSystem) :
    HwShaderGenerator(typeSystem, WgslSyntax::create(typeSystem))
{
    registerImplementations(TARGET);

    // WGSL splits each FILENAME uniform into a separate texture and sampler (see
    // WgslResourceBindingContext), so the environment lookups take the texture and
    // sampler as two arguments. Override the default combined-sampler tokens to the
    // split "<name>_texture" / "<name>_sampler" forms emitted by the binding context.
    // ($envRadianceSampler / $envIrradianceSampler are WGSL-specific companion tokens
    // used by the genwgsl environment libraries; they have no HwConstants entry.)
    _tokenSubstitutions[HW::T_ENV_RADIANCE] = HW::ENV_RADIANCE + "_texture";
    _tokenSubstitutions["$envRadianceSampler"] = HW::ENV_RADIANCE + "_sampler";
    _tokenSubstitutions[HW::T_ENV_IRRADIANCE] = HW::ENV_IRRADIANCE + "_texture";
    _tokenSubstitutions["$envIrradianceSampler"] = HW::ENV_IRRADIANCE + "_sampler";

    _lightSamplingNodes.push_back(ShaderNode::create(nullptr, "numActiveLightSources", WgslNumLightsNode::create()));
    _lightSamplingNodes.push_back(ShaderNode::create(nullptr, "sampleLightSource", WgslLightSamplerNode::create()));
}

void WgslShaderGenerator::registerImplementations(const string& target)
{
    StringVec elementNames;

    registerImplementation("IM_position_vector3_" + target, HwPositionNode::create);
    registerImplementation("IM_normal_vector3_" + target, HwNormalNode::create);
    registerImplementation("IM_tangent_vector3_" + target, HwTangentNode::create);
    registerImplementation("IM_bitangent_vector3_" + target, HwBitangentNode::create);
    registerImplementation("IM_texcoord_vector2_" + target, HwTexCoordNode::create);
    registerImplementation("IM_texcoord_vector3_" + target, HwTexCoordNode::create);
    registerImplementation("IM_geomcolor_float_" + target, HwGeomColorNode::create);
    registerImplementation("IM_geomcolor_color3_" + target, HwGeomColorNode::create);
    registerImplementation("IM_geomcolor_color4_" + target, HwGeomColorNode::create);

    elementNames = {
        "IM_geompropvalue_integer_" + target,
        "IM_geompropvalue_float_" + target,
        "IM_geompropvalue_color3_" + target,
        "IM_geompropvalue_color4_" + target,
        "IM_geompropvalue_vector2_" + target,
        "IM_geompropvalue_vector3_" + target,
        "IM_geompropvalue_vector4_" + target,
    };
    registerImplementation(elementNames, HwGeomPropValueNode::create);
    registerImplementation("IM_geompropvalue_boolean_" + target, HwGeomPropValueNodeAsUniform::create);
    registerImplementation("IM_geompropvalue_string_" + target, HwGeomPropValueNodeAsUniform::create);
    registerImplementation("IM_geompropvalue_filename_" + target, HwGeomPropValueNodeAsUniform::create);

    registerImplementation("IM_frame_float_" + target, HwFrameNode::create);
    registerImplementation("IM_time_float_" + target, HwTimeNode::create);
    registerImplementation("IM_viewdirection_vector3_" + target, HwViewDirectionNode::create);

    registerImplementation("IM_surface_" + target, WgslSurfaceNode::create);
    registerImplementation("IM_light_" + target, HwLightNode::create);
    registerImplementation("IM_point_light_" + target, HwLightShaderNode::create);
    registerImplementation("IM_directional_light_" + target, HwLightShaderNode::create);
    registerImplementation("IM_spot_light_" + target, HwLightShaderNode::create);

    registerImplementation("IM_transformpoint_vector3_" + target, HwTransformPointNode::create);
    registerImplementation("IM_transformvector_vector3_" + target, HwTransformVectorNode::create);
    registerImplementation("IM_transformnormal_vector3_" + target, HwTransformNormalNode::create);

    elementNames = {
        "IM_image_float_" + target,
        "IM_image_color3_" + target,
        "IM_image_color4_" + target,
        "IM_image_vector2_" + target,
        "IM_image_vector3_" + target,
        "IM_image_vector4_" + target,
    };
    registerImplementation(elementNames, HwImageNode::create);
    registerImplementation("IM_surfacematerial_" + target, WgslMaterialNode::create);

    registerImplementation("IM_dot_lightshader_" + target, HwLightShaderNode::create);
}

ShaderNodeImplPtr WgslShaderGenerator::createShaderNodeImplForNodeGraph(const NodeGraph& nodegraph) const
{
    vector<OutputPtr> outputs = nodegraph.getActiveOutputs();
    if (outputs.empty())
    {
        throw ExceptionShaderGenError("NodeGraph '" + nodegraph.getName() + "' has no outputs defined");
    }

    const TypeDesc outputType = _typeSystem->getType(outputs[0]->getType());
    if (outputType == Type::LIGHTSHADER)
    {
        return WgslLightCompoundNode::create();
    }
    return WgslCompoundNode::create();
}

ShaderNodeImplPtr WgslShaderGenerator::createShaderNodeImplForImplementation(const Implementation&) const
{
    return WgslSourceCodeNode::create();
}

HwResourceBindingContextPtr WgslShaderGenerator::getResourceBindingContext(GenContext& context) const
{
    return context.getUserData<HwResourceBindingContext>(HW::USER_DATA_BINDING_CONTEXT);
}

ShaderPtr WgslShaderGenerator::generate(const string& name, ElementPtr element, GenContext& context) const
{
    // Provide a default standalone resource binding context if none was supplied.
    if (!getResourceBindingContext(context))
    {
        auto bindingCtx = WgslResourceBindingContext::create(0);
        context.pushUserData(HW::USER_DATA_BINDING_CONTEXT, std::static_pointer_cast<HwResourceBindingContext>(bindingCtx));
    }
    if (HwResourceBindingContextPtr binding = getResourceBindingContext(context))
    {
        binding->initialize();
    }

    ShaderPtr shader = createShader(name, element, context);
    ScopedFloatFormatting fmt(Value::FloatFormatFixed);

    auto setDataSemantics = [](VariableBlock& vertexData)
    {
        for (size_t i = 0; i < vertexData.size(); ++i)
        {
            ShaderPort* port = vertexData[i];
            if (port->getSemantic().empty())
                port->setSemantic(port->getName());
        }
    };

    // Vertex stage.
    ShaderStage& vs = shader->getStage(Stage::VERTEX);
    setDataSemantics(vs.getOutputBlock(HW::VERTEX_DATA));
    emitVertexStage(shader->getGraph(), context, vs);
    replaceTokens(_tokenSubstitutions, vs);

    // Pixel stage.
    ShaderStage& ps = shader->getStage(Stage::PIXEL);
    setDataSemantics(ps.getInputBlock(HW::VERTEX_DATA));
    emitPixelStage(shader->getGraph(), context, ps);
    replaceTokens(_tokenSubstitutions, ps);

    return shader;
}

string WgslShaderGenerator::getVertexDataPrefix(const VariableBlock& vertexData) const
{
    return vertexData.getInstance() + ".";
}

bool WgslShaderGenerator::requiresLighting(const ShaderGraph& graph) const
{
    const bool isBsdf = graph.hasClassification(ShaderNode::Classification::BSDF);
    const bool isLitSurfaceShader = graph.hasClassification(ShaderNode::Classification::SHADER) &&
                                    graph.hasClassification(ShaderNode::Classification::SURFACE) &&
                                    !graph.hasClassification(ShaderNode::Classification::UNLIT);
    return isBsdf || isLitSurfaceShader;
}

void WgslShaderGenerator::emitDirectives(GenContext&, ShaderStage&) const
{
    // WGSL has no #version or preprocessor directives.
}

void WgslShaderGenerator::emitConstants(GenContext& context, ShaderStage& stage) const
{
    emitLine("const M_PI: f32 = 3.1415926535897932", stage);
    emitLine("const M_FLOAT_EPS: f32 = 1e-6", stage);
    emitLine("const CLOSURE_TYPE_DEFAULT: i32 = 0", stage);
    emitLine("const CLOSURE_TYPE_REFLECTION: i32 = 1", stage);
    emitLine("const CLOSURE_TYPE_TRANSMISSION: i32 = 2", stage);
    emitLine("const CLOSURE_TYPE_INDIRECT: i32 = 3", stage);
    emitLine("const CLOSURE_TYPE_EMISSION: i32 = 4", stage);

    const VariableBlock& constants = stage.getConstantBlock();
    for (size_t i = 0; i < constants.size(); ++i)
    {
        emitLineBegin(stage);
        emitVariableDeclaration(constants[i], _syntax->getConstantQualifier(), context, stage);
        emitLineEnd(stage);
    }
    emitLineBreak(stage);
}

void WgslShaderGenerator::emitTypeDefinitions(GenContext& context, ShaderStage& stage) const
{
    // surfaceshader must be defined before mx_closure_type.wgsl (material = surfaceshader alias).
    emitLine("struct surfaceshader {", stage, false);
    emitLine("    color: vec3f,", stage, false);
    emitLine("    transparency: vec3f,", stage, false);
    emitLine("}", stage, false);
    emitLineBreak(stage);
    emitLine("struct displacementshader {", stage, false);
    emitLine("    offset: vec3f,", stage, false);
    emitLine("    scale: f32,", stage, false);
    emitLine("}", stage, false);
    emitLineBreak(stage);
    emitLine("struct lightshader {", stage, false);
    emitLine("    intensity: vec3f,", stage, false);
    emitLine("    direction: vec3f,", stage, false);
    emitLine("}", stage, false);
    emitLineBreak(stage);

    // ClosureData, BSDF, VDF, EDF, material, FresnelData, makeClosureData.
    emitLibraryInclude("pbrlib/genwgsl/lib/mx_closure_type.wgsl", context, stage);
    emitLineBreak(stage);
}

void WgslShaderGenerator::emitUniforms(GenContext& context, ShaderStage& stage) const
{
    HwResourceBindingContextPtr binding = getResourceBindingContext(context);
    if (!binding)
        return;

    for (const auto& it : stage.getUniformBlocks())
    {
        const VariableBlock& uniforms = *it.second;
        if (uniforms.empty() || uniforms.getName() == HW::LIGHT_DATA)
            continue;
        binding->emitResourceBindings(context, uniforms, stage);
    }
}

void WgslShaderGenerator::emitLightData(GenContext& context, ShaderStage& stage) const
{
    const VariableBlock& lightData = stage.getUniformBlock(HW::LIGHT_DATA);
    if (lightData.empty())
        return;

    HwResourceBindingContextPtr binding = getResourceBindingContext(context);
    if (!binding)
        return;

    const string structArraySuffix = "[" + HW::LIGHT_DATA_MAX_LIGHT_SOURCES + "]";
    binding->emitStructuredResourceBindings(context, lightData, stage, lightData.getInstance(), structArraySuffix);
}

void WgslShaderGenerator::emitSpecularEnvironment(GenContext& context, ShaderStage& stage) const
{
    const int specularMethod = context.getOptions().hwSpecularEnvironmentMethod;
    if (specularMethod == SPECULAR_ENVIRONMENT_FIS)
    {
        emitLibraryInclude("pbrlib/genwgsl/lib/mx_environment_fis.wgsl", context, stage);
    }
    else if (specularMethod == SPECULAR_ENVIRONMENT_PREFILTER)
    {
        emitLibraryInclude("pbrlib/genwgsl/lib/mx_environment_prefilter.wgsl", context, stage);
    }
    else if (specularMethod == SPECULAR_ENVIRONMENT_NONE)
    {
        emitLibraryInclude("pbrlib/genwgsl/lib/mx_environment_none.wgsl", context, stage);
    }
    else
    {
        throw ExceptionShaderGenError("Invalid hardware specular environment method specified: '" + std::to_string(specularMethod) + "'");
    }
    emitLineBreak(stage);
}

void WgslShaderGenerator::emitTransmissionRender(GenContext& context, ShaderStage& stage) const
{
    const int transmissionMethod = context.getOptions().hwTransmissionRenderMethod;
    if (transmissionMethod == TRANSMISSION_REFRACTION)
    {
        emitLibraryInclude("pbrlib/genwgsl/lib/mx_transmission_refract.wgsl", context, stage);
    }
    else if (transmissionMethod == TRANSMISSION_OPACITY)
    {
        emitLibraryInclude("pbrlib/genwgsl/lib/mx_transmission_opacity.wgsl", context, stage);
    }
    emitLineBreak(stage);
}

void WgslShaderGenerator::emitLightFunctionDefinitions(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        if (requiresLighting(graph) && context.getOptions().hwMaxActiveLightSources > 0)
        {
            if (graph.hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE))
            {
                HwLightShadersPtr lightShaders = context.getUserData<HwLightShaders>(HW::USER_DATA_LIGHT_SHADERS);
                if (lightShaders)
                {
                    for (const auto& it : lightShaders->get())
                        emitFunctionDefinition(*it.second, context, stage);
                }
                for (const auto& it : _lightSamplingNodes)
                    emitFunctionDefinition(*it, context, stage);
            }
        }
    }
}

void WgslShaderGenerator::emitInputs(GenContext& /*context*/, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        const VariableBlock& vertexInputs = stage.getInputBlock(HW::VERTEX_INPUTS);
        if (!vertexInputs.empty())
        {
            emitLine("struct " + vertexInputs.getName() + " ", stage, false);
            emitScopeBegin(stage);
            for (size_t i = 0; i < vertexInputs.size(); ++i)
            {
                const ShaderPort* p = vertexInputs[i];
                emitLine("@location(" + std::to_string(i) + ") " + p->getVariable() + ": " +
                             _syntax->getTypeName(p->getType()) + ",",
                         stage, false);
            }
            emitScopeEnd(stage, false, false);
            emitLineBreak(stage);
        }
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
        emitLine("struct " + vertexData.getName() + " ", stage, false);
        emitScopeBegin(stage);
        emitLine("@builtin(position) " + WGSL_CLIP_POSITION + ": vec4f,", stage, false);
        size_t loc = 0;
        for (size_t i = 0; i < vertexData.size(); ++i)
        {
            const ShaderPort* p = vertexData[i];
            string attr = "@location(" + std::to_string(loc++) + ")";
            if (p->getType() == Type::INTEGER)
                attr += " " + WgslSyntax::FLAT_QUALIFIER;
            emitLine(attr + " " + p->getVariable() + ": " + _syntax->getTypeName(p->getType()) + ",", stage, false);
        }
        emitScopeEnd(stage, false, false);
        emitLineBreak(stage);
    }
}

void WgslShaderGenerator::emitOutputs(GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        const VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        emitLine("struct " + vertexData.getName() + " ", stage, false);
        emitScopeBegin(stage);
        emitLine("@builtin(position) " + WGSL_CLIP_POSITION + ": vec4f,", stage, false);
        size_t loc = 0;
        for (size_t i = 0; i < vertexData.size(); ++i)
        {
            const ShaderPort* p = vertexData[i];
            string attr = "@location(" + std::to_string(loc++) + ")";
            if (p->getType() == Type::INTEGER)
                attr += " " + WgslSyntax::FLAT_QUALIFIER;
            emitLine(attr + " " + p->getVariable() + ": " + _syntax->getTypeName(p->getType()) + ",", stage, false);
        }
        emitScopeEnd(stage, false, false);
        emitLineBreak(stage);
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        // Declare the pixel output variables as locals within fragmentMain.
        const VariableBlock& outputs = stage.getOutputBlock(HW::PIXEL_OUTPUTS);
        for (size_t i = 0; i < outputs.size(); ++i)
        {
            emitLineBegin(stage);
            emitVariableDeclaration(outputs[i], EMPTY_STRING, context, stage, false);
            emitLineEnd(stage);
        }
        emitLineBreak(stage);
    }
}

void WgslShaderGenerator::emitVertexStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    emitDirectives(context, stage);
    emitConstants(context, stage);
    emitUniforms(context, stage);
    emitInputs(context, stage);
    emitOutputs(context, stage);

    emitLibraryInclude("stdlib/genwgsl/lib/mx_math.wgsl", context, stage);
    emitLineBreak(stage);

    emitFunctionDefinitions(graph, context, stage);

    const VariableBlock& vertexInputs = stage.getInputBlock(HW::VERTEX_INPUTS);
    const VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
    const string prefix = getVertexDataPrefix(vertexData);

    setFunctionName("vertexMain", stage);
    emitLine("@vertex", stage, false);
    emitLine("fn vertexMain(vsIn: " + vertexInputs.getName() + ") -> " + vertexData.getName() + " ", stage, false);
    emitFunctionBodyBegin(graph, context, stage);

    // Alias the vertex inputs as locals so token-substituted node code resolves.
    emitComment("Vertex input variables", stage);
    for (size_t i = 0; i < vertexInputs.size(); ++i)
    {
        emitLine("var " + vertexInputs[i]->getVariable() + " = vsIn." + vertexInputs[i]->getVariable(), stage);
    }

    emitLine("var hPositionWorld: vec4f = " + HW::T_WORLD_MATRIX + " * vec4f(" + HW::T_IN_POSITION + ", 1.0)", stage);
    emitLine("var " + vertexData.getInstance() + ": " + vertexData.getName(), stage);
    emitLine(prefix + WGSL_CLIP_POSITION + " = " + HW::T_VIEW_PROJECTION_MATRIX + " * hPositionWorld", stage);

    for (const ShaderNode* node : graph.getNodes())
    {
        emitFunctionCall(*node, context, stage);
    }

    emitLine("return " + vertexData.getInstance(), stage);
    emitFunctionBodyEnd(graph, context, stage);
}

void WgslShaderGenerator::emitPixelStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    emitDirectives(context, stage);

    // Type definitions (surfaceshader, displacementshader, lightshader, closure types).
    emitTypeDefinitions(context, stage);

    // Constants.
    emitConstants(context, stage);

    // Uniforms (material + private), excluding the light data block.
    emitUniforms(context, stage);

    // Shared vertex-data input struct.
    emitInputs(context, stage);

    // Common math helpers.
    emitLibraryInclude("stdlib/genwgsl/lib/mx_math.wgsl", context, stage);
    emitLineBreak(stage);

    const bool lighting = requiresLighting(graph);
    const bool emitLightUniforms = lighting && context.getOptions().hwMaxActiveLightSources > 0;

    if (emitLightUniforms)
    {
        const unsigned int maxLights = std::max(1u, context.getOptions().hwMaxActiveLightSources);
        emitLine("const " + HW::LIGHT_DATA_MAX_LIGHT_SOURCES + ": i32 = " + std::to_string(maxLights), stage);
        emitLineBreak(stage);
    }

    if (lighting)
    {
        emitSpecularEnvironment(context, stage);
        emitTransmissionRender(context, stage);
    }

    if (emitLightUniforms)
    {
        emitLightData(context, stage);
    }

    _tokenSubstitutions[ShaderGenerator::T_FILE_TRANSFORM_UV] =
        context.getOptions().fileTextureVerticalFlip ? "mx_transform_uv_vflip.wgsl" : "mx_transform_uv.wgsl";

    emitLightFunctionDefinitions(graph, context, stage);
    emitFunctionDefinitions(graph, context, stage);

    const ShaderGraphOutputSocket* outputSocket = graph.getOutputSocket();
    const VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);

    setFunctionName("fragmentMain", stage);
    emitLine("@fragment", stage, false);
    // The fragment entry takes only the interpolated vertex data. (Double-sided shading via
    // @builtin(front_facing) is not currently emitted; a single struct parameter also keeps the
    // entry consumable as a Three.js wgslFn by the TSL bridge in the web viewer.)
    emitLine("fn fragmentMain(" + vertexData.getInstance() + ": " + vertexData.getName() +
                 ") -> @location(0) vec4f ",
             stage, false);
    emitFunctionBodyBegin(graph, context, stage);

    // Declare the pixel output local variable(s).
    emitOutputs(context, stage);

    if (graph.hasClassification(ShaderNode::Classification::CLOSURE) &&
        !graph.hasClassification(ShaderNode::Classification::SHADER))
    {
        // A bare closure with no surface: output black.
        emitLine(outputSocket->getVariable() + " = vec4f(0.0, 0.0, 0.0, 1.0)", stage);
    }
    else
    {
        if (graph.hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE))
        {
            emitFunctionCalls(graph, context, stage, ShaderNode::Classification::TEXTURE);
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
            emitFunctionCalls(graph, context, stage);
        }

        const ShaderOutput* outputConnection = outputSocket->getConnection();
        if (outputConnection)
        {
            if (graph.hasClassification(ShaderNode::Classification::SURFACE))
            {
                string outColor = outputConnection->getVariable() + ".color";
                const string outTransparency = outputConnection->getVariable() + ".transparency";
                if (context.getOptions().hwSrgbEncodeOutput)
                {
                    outColor = "mx_srgb_encode(" + outColor + ")";
                }
                if (context.getOptions().hwTransparency)
                {
                    emitLine("var outAlpha: f32 = clamp(1.0 - dot(" + outTransparency + ", vec3f(0.3333, 0.3333, 0.3333)), 0.0, 1.0)", stage);
                    emitLine(outputSocket->getVariable() + " = vec4f(" + outColor + ", outAlpha)", stage);
                    emitLine("if (outAlpha < " + HW::T_ALPHA_THRESHOLD + ") ", stage, false);
                    emitScopeBegin(stage);
                    emitLine("discard", stage);
                    emitScopeEnd(stage);
                }
                else
                {
                    emitLine(outputSocket->getVariable() + " = vec4f(" + outColor + ", 1.0)", stage);
                }
            }
            else
            {
                string outValue = outputConnection->getVariable();
                if (context.getOptions().hwSrgbEncodeOutput && outputSocket->getType().isFloat3())
                {
                    outValue = "mx_srgb_encode(" + outValue + ")";
                }
                if (!outputSocket->getType().isFloat4())
                {
                    toVec4Wgsl(outputSocket->getType(), outValue);
                }
                emitLine(outputSocket->getVariable() + " = " + outValue, stage);
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
                emitLine("var " + finalOutput + ": " + _syntax->getTypeName(outputSocket->getType()) + " = " + outputValue, stage);
                toVec4Wgsl(outputSocket->getType(), finalOutput);
                emitLine(outputSocket->getVariable() + " = " + finalOutput, stage);
            }
            else
            {
                emitLine(outputSocket->getVariable() + " = " + outputValue, stage);
            }
        }
    }

    emitLine("return " + outputSocket->getVariable(), stage);
    emitFunctionBodyEnd(graph, context, stage);
}

//
// WGSL token mechanics
//

void WgslShaderGenerator::emitVariableDeclaration(const ShaderPort* variable, const string& qualifier,
                                                  GenContext&, ShaderStage& stage, bool assignValue) const
{
    if (variable->getType() == Type::FILENAME)
    {
        emitString("var " + variable->getVariable() + ": texture_2d<f32>", stage);
        return;
    }

    string typeName = _syntax->getTypeName(variable->getType());
    if (variable->getType().isArray() && variable->getValue())
        typeName += _syntax->getArrayVariableSuffix(variable->getType(), *variable->getValue());

    const bool isConst = (qualifier == _syntax->getConstantQualifier());
    string line = isConst ? "const " : "var ";
    line += variable->getVariable() + ": " + typeName;

    if (assignValue && !isConst && qualifier == _syntax->getUniformQualifier())
    {
        // Uniforms are not assigned inline.
        emitString(line, stage);
        return;
    }
    if (assignValue)
    {
        const string valueStr = variable->getValue() ?
            _syntax->getValue(variable->getType(), *variable->getValue()) :
            _syntax->getDefaultValue(variable->getType());
        if (!valueStr.empty())
            line += " = " + valueStr;
    }
    emitString(line, stage);
}

void WgslShaderGenerator::emitOutput(const ShaderOutput* output, bool includeType, bool assignValue, GenContext& context, ShaderStage& stage) const
{
    if (includeType)
    {
        stage.addString("var " + output->getVariable() + ": " + _syntax->getTypeName(output->getType()));
    }
    else
    {
        // As a function-call argument an output is passed by pointer.
        stage.addString(assignValue ? output->getVariable() : "&" + output->getVariable());
    }

    string suffix;
    context.getOutputSuffix(output, suffix);
    if (!suffix.empty())
        stage.addString(suffix);

    if (assignValue)
    {
        const string& value = _syntax->getDefaultValue(output->getType());
        if (!value.empty())
            stage.addString(" = " + value);
    }
}

void WgslShaderGenerator::emitFunctionDefinitionParameter(const ShaderPort* shaderPort, bool isOutput, GenContext&, ShaderStage& stage) const
{
    if (isOutput)
        return;
    if (shaderPort->getType() == Type::FILENAME)
    {
        const string& varName = shaderPort->getVariable();
        HwShaderGenerator::emitString(varName + "_texture: texture_2d<f32>, " + varName + "_sampler: sampler", stage);
        return;
    }
    HwShaderGenerator::emitString(shaderPort->getVariable() + ": " + _syntax->getTypeName(shaderPort->getType()), stage);
}

void WgslShaderGenerator::emitInput(const ShaderInput* input, GenContext& context, ShaderStage& stage) const
{
    if (input->getType() == Type::FILENAME)
    {
        const string base = getUpstreamResult(input, context);
        HwShaderGenerator::emitString(base + "_texture, " + base + "_sampler", stage);
        return;
    }
    if (input->getType() == Type::BOOLEAN)
    {
        // Boolean public uniforms are stored as u32 (bool is not host-shareable), so
        // convert to bool at the use site. bool(bool) is an identity conversion, so this
        // is also safe for boolean values originating from computed locals or literals.
        HwShaderGenerator::emitString("bool(" + getUpstreamResult(input, context) + ")", stage);
        return;
    }
    HwShaderGenerator::emitInput(input, context, stage);
}

void WgslShaderGenerator::emitClosureDataParameter(const ShaderNode& node, GenContext&, ShaderStage& stage) const
{
    if (nodeNeedsClosureData(node))
    {
        // WGSL parameter syntax: "closureData: ClosureData, ".
        emitString(HW::CLOSURE_DATA_ARG + ": " + HW::CLOSURE_DATA_TYPE + ", ", stage);
    }
}

void WgslShaderGenerator::emitBlock(const string& str, const FilePath& sourceFilename, GenContext& context, ShaderStage& stage) const
{
    stage.addBlock(str, sourceFilename, context);

    if (sourceFilename.getExtension() == "wgsl" && !str.empty())
    {
        auto data = context.getUserData<WgslEmittedFunctions>(WGSL_EMITTED_FUNCTIONS);
        if (!data)
        {
            data = std::make_shared<WgslEmittedFunctions>();
            context.pushUserData(WGSL_EMITTED_FUNCTIONS, data);
        }
        trackEmittedFunctions(str, data->names);
    }
}

MATERIALX_NAMESPACE_END
