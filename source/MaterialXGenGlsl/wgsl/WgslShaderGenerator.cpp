//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenGlsl/wgsl/WgslShaderGenerator.h>
#include <MaterialXGenGlsl/wgsl/converter/GlslToWgsl.h>
#include <MaterialXGenGlsl/wgsl/WgslSyntax.h>

#include <MaterialXGenHw/HwConstants.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGraph.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/Util.h>

#include <MaterialXCore/Util.h>

#include <MaterialXGenShader/GenUserData.h>

#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <iostream>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

MATERIALX_NAMESPACE_BEGIN

const string WgslShaderGenerator::TARGET = "genwgsl";
const string WgslShaderGenerator::LIGHTDATA_TYPEVAR_STRING = "light_type";

namespace
{
// Tracks include files (by basename) already expanded during a generation, so the
// genglsl headers (closure types, math) that many BSDF sources #include are emitted
// once rather than producing duplicate WGSL struct/const definitions.
class WgslIncludeSet : public GenUserData
{
  public:
    std::set<string> basenames;
};
const string WGSL_INCLUDES = "WGSL_INCLUDES";

bool isIdentChar(char c)
{
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

string getWgslTypeName(const Syntax& syntax, const TypeDesc& type)
{
    if (type == Type::BOOLEAN)
        return "i32";
    return GlslToWgsl::mapType(syntax.getTypeName(type));
}

string mapGlslValueExpr(string value)
{
    if (value == "true")
        return "1";
    if (value == "false")
        return "0";

    struct CtorMap
    {
        const char* from;
        const char* to;
    };
    static const CtorMap CTORS[] = {
        { "vec2(", "vec2f(" },
        { "vec3(", "vec3f(" },
        { "vec4(", "vec4f(" },
        { "mat2(", "mat2x2f(" },
        { "mat3(", "mat3x3f(" },
        { "mat4(", "mat4x4f(" },
    };
    for (const CtorMap& c : CTORS)
    {
        const string from = c.from;
        size_t pos = 0;
        while ((pos = value.find(from, pos)) != string::npos)
        {
            if (pos > 0 && isIdentChar(value[pos - 1]))
            {
                pos += from.size();
                continue;
            }
            value.replace(pos, from.size(), c.to);
            pos += std::strlen(c.to);
        }
    }
    return value;
}

string getWgslValue(const Syntax& syntax, const TypeDesc& type, const Value& value)
{
    if (type == Type::BOOLEAN)
        return value.getValueString() == "true" ? "1" : "0";
    return mapGlslValueExpr(syntax.getValue(type, value));
}

string getWgslDefaultValue(const Syntax& syntax, const TypeDesc& type)
{
    if (type == Type::BOOLEAN)
        return "0";
    return mapGlslValueExpr(syntax.getDefaultValue(type));
}
} // namespace

WgslShaderGenerator::WgslShaderGenerator(TypeSystemPtr typeSystem) :
    VkShaderGenerator(typeSystem)
{
    _syntax = WgslSyntax::create(typeSystem);

    // Set binding context to handle resource binding layouts
    _resourceBindingCtx = std::make_shared<MaterialX::WgslResourceBindingContext>(0);

    // For functions described in ::emitSpecularEnvironment()
    // override map value from HwShaderGenerator
    _tokenSubstitutions[HW::T_ENV_RADIANCE]             = HW::ENV_RADIANCE_SPLIT;
    _tokenSubstitutions[HW::T_ENV_RADIANCE_SAMPLER2D]   = HW::ENV_RADIANCE_SAMPLER2D_SPLIT;
    _tokenSubstitutions[HW::T_ENV_IRRADIANCE]           = HW::ENV_IRRADIANCE_SPLIT;
    _tokenSubstitutions[HW::T_ENV_IRRADIANCE_SAMPLER2D] = HW::ENV_IRRADIANCE_SAMPLER2D_SPLIT;
    _tokenSubstitutions[HW::T_TEX_SAMPLER_SAMPLER2D]    = HW::TEX_SAMPLER_SAMPLER2D_SPLIT;
    _tokenSubstitutions[HW::T_TEX_SAMPLER_SIGNATURE]    = HW::TEX_SAMPLER_SIGNATURE_SPLIT;
}

ShaderNodeImplPtr WgslShaderGenerator::getImplementation(const NodeDef& nodedef, GenContext& context) const
{
    InterfaceElementPtr implElement = nodedef.getImplementation(TARGET);
    if (!implElement)
    {
        // WGSL reuses genglsl node implementations until genwgsl-specific overrides exist.
        implElement = nodedef.getImplementation(VkShaderGenerator::TARGET);
    }
    if (!implElement)
    {
        return nullptr;
    }

    const string& name = implElement->getName();
    ShaderNodeImplPtr impl = context.findNodeImplementation(name);
    if (impl)
    {
        return impl;
    }

    if (implElement->isA<NodeGraph>())
    {
        impl = createShaderNodeImplForNodeGraph(*implElement->asA<NodeGraph>());
    }
    else if (implElement->isA<Implementation>())
    {
        if (getColorManagementSystem() && getColorManagementSystem()->hasImplementation(name))
        {
            impl = getColorManagementSystem()->createImplementation(name);
        }
        else
        {
            impl = _implFactory.create(name);
        }
        if (!impl)
        {
            impl = createShaderNodeImplForImplementation(*implElement->asA<Implementation>());
        }
    }
    if (!impl)
    {
        return nullptr;
    }

    impl->initialize(*implElement, context);
    context.addNodeImplementation(name, impl);
    return impl;
}

void WgslShaderGenerator::emitVariableDeclaration(const ShaderPort* variable, const string& qualifier,
                                                  GenContext&, ShaderStage& stage, bool assignValue) const
{
    if (variable->getType() == Type::FILENAME)
    {
        // Combined GLSL sampler2D becomes a WGSL texture (sampler emitted separately).
        emitString("var " + variable->getVariable() + ": texture_2d<f32>", stage);
        return;
    }

    string typeName = getWgslTypeName(*_syntax, variable->getType());
    if (variable->getType().isArray() && variable->getValue())
        typeName += _syntax->getArrayVariableSuffix(variable->getType(), *variable->getValue());

    const bool isConst = (qualifier == _syntax->getConstantQualifier());
    const string keyword = isConst ? "const " : "var ";
    string line = keyword + variable->getVariable() + ": " + typeName;

    const bool isUniform = (qualifier == _syntax->getUniformQualifier());
    if (assignValue && !isUniform)
    {
        const string valueStr = variable->getValue()
                                    ? getWgslValue(*_syntax, variable->getType(), *variable->getValue())
                                    : getWgslDefaultValue(*_syntax, variable->getType());
        if (!valueStr.empty())
            line += " = " + valueStr;
    }

    emitString(line, stage);
}

// Called by CompoundNode::emitFunctionDefinition()
void WgslShaderGenerator::emitFunctionDefinitionParameter(const ShaderPort* shaderPort, bool isOutput, GenContext& /*context*/, ShaderStage& stage) const
{
    const string& name = shaderPort->getVariable();
    if (shaderPort->getType() == Type::FILENAME)
    {
        VkShaderGenerator::emitString(name + "_texture: texture_2d<f32>, " + name + "_sampler: sampler", stage);
        return;
    }
    const string typeName = getWgslTypeName(*_syntax, shaderPort->getType());
    if (isOutput)
    {
        VkShaderGenerator::emitString(name + ": ptr<function, " + typeName + ">", stage);
    }
    else
    {
        VkShaderGenerator::emitString(name + ": " + typeName, stage);
    }
}

// Called by SourceCodeNode::emitFunctionCall()
void WgslShaderGenerator::emitInput(const ShaderInput* input, GenContext& context, ShaderStage& stage) const
{
    if (input->getType() == Type::FILENAME)
    {
        emitString(getUpstreamResult(input, context) + "_texture, " + getUpstreamResult(input, context) + "_sampler", stage);
    }
    else
    {
        VkShaderGenerator::emitInput(input, context, stage);
    }
}

void WgslShaderGenerator::emitOutput(const ShaderOutput* output, bool includeType, bool assignValue,
                                     GenContext& context, ShaderStage& stage) const
{
    if (includeType)
    {
        stage.addString("var " + output->getVariable() + ": " + getWgslTypeName(*_syntax, output->getType()));
    }
    else
    {
        // Function-call argument: WGSL out parameters are `ptr<function,T>`, passed as `&var`.
        stage.addString((assignValue ? "" : "&") + output->getVariable());
    }

    string suffix;
    context.getOutputSuffix(output, suffix);
    if (!suffix.empty())
        stage.addString(suffix);

    if (assignValue)
    {
        const string value = getWgslDefaultValue(*_syntax, output->getType());
        if (!value.empty())
            stage.addString(" = " + value);
    }
}

void WgslShaderGenerator::emitString(const string& str, ShaderStage& stage) const
{
    if (str.size() >= 5 && str.compare(0, 5, "void ") == 0)
    {
        string replaced = str;
        replaced.replace(0, 5, "fn ");
        VkShaderGenerator::emitString(replaced, stage);
        return;
    }
    VkShaderGenerator::emitString(str, stage);
}

void WgslShaderGenerator::emitLine(const string& str, ShaderStage& stage, bool semicolon) const
{
    // Split a run of statements onto separate lines, then apply the GLSL->WGSL rewrites.
    const size_t firstNewline = str.find('\n');
    if (firstNewline != string::npos)
    {
        emitLine(str.substr(0, firstNewline), stage, true);
        emitLine(str.substr(firstNewline + 1), stage, semicolon);
        return;
    }
    VkShaderGenerator::emitLine(GlslToWgsl::rewriteAll(str), stage, semicolon);
}

void WgslShaderGenerator::emitLineEnd(ShaderStage& stage, bool semicolon) const
{
    // Rewrite the line just built via emitString (e.g. by SourceCodeNode) to WGSL.
    const string& code = stage.getSourceCode();
    const string& newlineStr = _syntax->getNewline();
    const size_t newlineLen = newlineStr.empty() ? 1 : newlineStr.size();
    const size_t lastNewline = code.rfind(newlineStr);
    const string lastLine = (lastNewline == string::npos) ? code : code.substr(lastNewline + newlineLen);
    if (!lastLine.empty())
    {
        const string rewritten = GlslToWgsl::rewriteAll(lastLine);
        if (rewritten != lastLine)
        {
            const string newCode = (lastNewline == string::npos)
                                       ? rewritten
                                       : code.substr(0, lastNewline + newlineLen) + rewritten;
            stage.setSourceCode(newCode);
        }
    }
    VkShaderGenerator::emitLineEnd(stage, semicolon);
}

void WgslShaderGenerator::emitBlock(const string& str, const FilePath& sourceFilename, GenContext& context, ShaderStage& stage) const
{
    if (sourceFilename.getExtension() == "glsl" && !str.empty())
    {
        GlslToWgsl::LineRewriter rewriter;
        const string wgsl = expandAndRewriteGlsl(str, sourceFilename, context, rewriter);
        stage.addBlock(wgsl, sourceFilename, context);
        return;
    }
    VkShaderGenerator::emitBlock(str, sourceFilename, context, stage);
}

void WgslShaderGenerator::emitLibraryInclude(const FilePath& filename, GenContext& context, ShaderStage& stage) const
{
    FilePath libraryPrefix = context.getOptions().libraryPrefix;
    FilePath fullFilename = libraryPrefix.isEmpty() ? filename : libraryPrefix / filename;
    FilePath resolvedFilename = context.resolveSourceFile(fullFilename, FilePath());
    if (resolvedFilename.exists())
    {
        emitBlock(readFile(resolvedFilename), resolvedFilename, context, stage);
    }
}

void WgslShaderGenerator::emitTypeDefinitions(GenContext&, ShaderStage&) const
{
    // GLSL typedefs are `#define` directives (e.g. `#define EDF vec3`). WGSL has no
    // preprocessor — surface/closure aliases are emitted by emitWgslSurfaceTypes().
}

void WgslShaderGenerator::emitVertexStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    emitDirectives(context, stage);
    emitLineBreak(stage);

    emitConstants(context, stage);
    emitUniforms(context, stage);
    emitInputs(context, stage);
    emitOutputs(context, stage);

    emitLibraryInclude("stdlib/genglsl/lib/mx_math.glsl", context, stage);
    emitLineBreak(stage);

    emitFunctionDefinitions(graph, context, stage);

    const VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
    const string vdInstance = vertexData.getInstance();

    setFunctionName("VertexMain", stage);
    emitLine("fn VertexMain(inputs: VertexInput) -> VertexOutput", stage, false);
    emitFunctionBodyBegin(graph, context, stage);

    emitLine("var output: VertexOutput", stage);
    emitLine("var " + vdInstance + ": " + vertexData.getName(), stage);
    emitLine("let hPositionWorld = " + HW::T_WORLD_MATRIX + " * vec4f(inputs." + HW::T_IN_POSITION + ", 1.0)", stage);
    emitLine("output.position = " + HW::T_VIEW_PROJECTION_MATRIX + " * hPositionWorld", stage);

    for (const ShaderNode* node : graph.getNodes())
        emitFunctionCall(*node, context, stage);

    emitLine("output." + vdInstance + " = " + vdInstance, stage);
    emitLine("return output", stage);
    emitFunctionBodyEnd(graph, context, stage);

    finalizeStageSource(stage);
}

void WgslShaderGenerator::emitPixelStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    _tokenSubstitutions[ShaderGenerator::T_FILE_TRANSFORM_UV] =
        context.getOptions().fileTextureVerticalFlip ? "mx_transform_uv_vflip.glsl" : "mx_transform_uv.glsl";

    auto includeSet = std::make_shared<WgslIncludeSet>();
    context.pushUserData(WGSL_INCLUDES, includeSet);

    emitDirectives(context, stage);
    emitLineBreak(stage);

    const bool lighting = requiresLighting(graph);
    const ShaderGraphOutputSocket* outputSocket = graph.getOutputSocket();
    const TypeDesc outSocketType = outputSocket->getType();
    const bool isSurface = lighting || outSocketType == Type::SURFACESHADER || outSocketType == Type::MATERIAL;

    if (isSurface)
        emitWgslSurfaceTypes(context, stage);

    emitTypeDefinitions(context, stage);
    emitConstants(context, stage);

    emitPixelUniforms(context, stage, lighting);

    const VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
    emitVertexDataStruct(vertexData, stage);

    emitLibraryInclude("stdlib/genglsl/lib/mx_math.glsl", context, stage);
    emitLineBreak(stage);

    if (lighting || context.getOptions().hwWriteAlbedoTable || context.getOptions().hwWriteEnvPrefilter)
    {
        emitLine("const DIRECTIONAL_ALBEDO_METHOD: i32 = " + std::to_string(int(context.getOptions().hwDirectionalAlbedoMethod)), stage);
        emitLineBreak(stage);
    }

    emitLine("const AIRY_FRESNEL_ITERATIONS: i32 = " + std::to_string(context.getOptions().hwAiryFresnelIterations), stage);
    emitLineBreak(stage);

    if (lighting)
    {
        if (context.getOptions().hwMaxActiveLightSources > 0)
        {
            const unsigned int maxLights = std::max(1u, context.getOptions().hwMaxActiveLightSources);
            emitLine("const " + HW::LIGHT_DATA_MAX_LIGHT_SOURCES + ": i32 = " + std::to_string(maxLights), stage);
            emitLineBreak(stage);
        }
        emitSpecularEnvironment(context, stage);
        emitTransmissionRender(context, stage);
        if (context.getOptions().hwMaxActiveLightSources > 0)
            emitLightData(context, stage);
    }

    emitLightFunctionDefinitions(graph, context, stage);
    emitFunctionDefinitions(graph, context, stage);
    emitLineBreak(stage);

    const string vdInstance = vertexData.getInstance();
    const string entryParam = vdInstance + ": " + vertexData.getName();
    setFunctionName("FragmentMain", stage);
    emitLine("fn FragmentMain(" + entryParam + ") -> vec4f", stage, false);
    emitScopeBegin(stage);

    if (graph.hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE))
    {
        emitFunctionCalls(graph, context, stage, ShaderNode::Classification::TEXTURE);
        for (const ShaderGraphOutputSocket* socket : graph.getOutputSockets())
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
    string outValue = outputConnection ? outputConnection->getVariable() : getWgslDefaultValue(*_syntax, Type::COLOR3);
    const TypeDesc outType = outputSocket->getType();
    string vec4Value;
    if (isSurface && outputConnection)
    {
        string outColor = outValue + ".color";
        const string outTransparency = outValue + ".transparency";
        if (context.getOptions().hwSrgbEncodeOutput)
            outColor = "mx_srgb_encode(" + outColor + ")";
        if (context.getOptions().hwTransparency)
        {
            emitLine("let outAlpha: f32 = clamp(1.0 - dot(" + outTransparency + ", vec3f(0.3333)), 0.0, 1.0)", stage);
            vec4Value = "vec4f(" + outColor + ", outAlpha)";
            emitLine("if (outAlpha < " + HW::T_ALPHA_THRESHOLD + ")", stage, false);
            emitScopeBegin(stage);
            emitLine("discard", stage);
            emitScopeEnd(stage);
        }
        else
        {
            vec4Value = "vec4f(" + outColor + ", 1.0)";
        }
    }
    else if (outType.isFloat4())
    {
        vec4Value = outValue;
    }
    else if (outType.isFloat3())
    {
        if (context.getOptions().hwSrgbEncodeOutput)
            outValue = "mx_srgb_encode(" + outValue + ")";
        vec4Value = "vec4f(" + outValue + ", 1.0)";
    }
    else
    {
        vec4Value = "vec4f(vec3f(" + outValue + "), 1.0)";
    }
    emitLine("return " + vec4Value, stage);
    emitScopeEnd(stage);

    finalizeStageSource(stage);
}

void WgslShaderGenerator::emitDirectives(GenContext&, ShaderStage&) const
{
    // WGSL has no #version or preprocessor directives.
}

void WgslShaderGenerator::emitWgslSurfaceTypes(GenContext&, ShaderStage& stage) const
{
    stage.addString(R"WGSL(// MaterialX closure result types (multi-line so the
// overload/broadcast type-inference pass can read their members).
struct BSDF {
    response: vec3f,
    throughput: vec3f,
}
struct VDF {
    response: vec3f,
    throughput: vec3f,
}
alias EDF = vec3<f32>;
struct surfaceshader {
    color: vec3f,
    transparency: vec3f,
}
struct volumeshader {
    color: vec3f,
    transparency: vec3f,
}
struct displacementshader {
    offset: vec3f,
    scale: f32,
}
alias material = surfaceshader;
struct lightshader {
    direction: vec3f,
    intensity: vec3f,
}
)WGSL");
    emitLineBreak(stage);
}

void WgslShaderGenerator::emitPixelUniforms(GenContext& context, ShaderStage& stage, bool lighting) const
{
    HwResourceBindingContextPtr resourceBindingCtx = getResourceBindingContext(context);
    if (!resourceBindingCtx)
        return;

    auto wgslCtx = std::dynamic_pointer_cast<WgslResourceBindingContext>(resourceBindingCtx);
    wgslCtx->setBindingLocation(0);

    for (const auto& it : stage.getUniformBlocks())
    {
        const VariableBlock& uniforms = *it.second;
        if (uniforms.empty() || uniforms.getName() == HW::LIGHT_DATA)
            continue;

        emitComment("Uniform block: " + uniforms.getName(), stage);
        resourceBindingCtx->emitResourceBindings(context, uniforms, stage);
        emitLineBreak(stage);
    }

    if (lighting && context.getOptions().hwMaxActiveLightSources > 0)
    {
        const VariableBlock& lightData = stage.getUniformBlock(HW::LIGHT_DATA);
        const string structArraySuffix = "[" + HW::LIGHT_DATA_MAX_LIGHT_SOURCES + "]";
        resourceBindingCtx->emitStructuredResourceBindings(context, lightData, stage, lightData.getInstance(), structArraySuffix);
    }
}

void WgslShaderGenerator::emitLightData(GenContext&, ShaderStage&) const
{
    // emitPixelUniforms() already emits the LightData struct plus u_lightData array.
    // The GLSL base class would emit a second copy here.
}

void WgslShaderGenerator::emitInputs(GenContext& /*context*/, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        const VariableBlock& vertexInputs = stage.getInputBlock(HW::VERTEX_INPUTS);
        if (!vertexInputs.empty())
        {
            emitComment("Inputs block: " + vertexInputs.getName(), stage);
            emitLine("struct VertexInput", stage, false);
            emitScopeBegin(stage);
            for (size_t i = 0; i < vertexInputs.size(); ++i)
            {
                const string name = replaceSubstrings(vertexInputs[i]->getVariable(), getTokenSubstitutions());
                emitLine("@location(" + std::to_string(i) + ") " + name + ": " +
                             getWgslTypeName(*_syntax, vertexInputs[i]->getType()),
                         stage);
            }
            emitScopeEnd(stage, false);
            emitLineBreak(stage);
        }
    }
}

void WgslShaderGenerator::emitVertexDataStruct(const VariableBlock& vertexData, ShaderStage& stage) const
{
    if (vertexData.empty())
        return;
    emitLine("struct " + vertexData.getName(), stage, false);
    emitScopeBegin(stage);
    for (size_t i = 0; i < vertexData.size(); ++i)
    {
        const string name = replaceSubstrings(vertexData[i]->getVariable(), getTokenSubstitutions());
        // WGSL struct members are comma-separated (not `;`-terminated like GLSL).
        emitLine(name + ": " + getWgslTypeName(*_syntax, vertexData[i]->getType()) + ",", stage, false);
    }
    emitScopeEnd(stage, false);
    emitLineBreak(stage);
}

void WgslShaderGenerator::emitOutputs(GenContext& /*context*/, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        const VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        if (!vertexData.empty())
        {
            emitVertexDataStruct(vertexData, stage);
            emitLine("struct VertexOutput", stage, false);
            emitScopeBegin(stage);
            emitLine("@builtin(position) position: vec4f", stage);
            emitLine("@location(" + std::to_string(vertexDataLocation) + ") " + vertexData.getInstance() + ": " +
                         vertexData.getName(),
                     stage);
            emitScopeEnd(stage, false);
            emitLineBreak(stage);
        }
    }
}

void WgslShaderGenerator::finalizeStageSource(ShaderStage& stage) const
{
    string src = stage.getSourceCode();
    // HwNumLightsNode / HwLightSamplerNode emit GLSL function definitions that per-line
    // rewriteAll() does not convert. Rewrite only those stubs (not the whole stage).
    src = GlslToWgsl::rewriteResidualGlslFunctions(src);
    src = GlslToWgsl::dedupDefinitions(src);
    src = GlslToWgsl::derefPointerParams(src);
    src = GlslToWgsl::resolveOverloads(src);
    src = GlslToWgsl::coerceBoolCallSites(src);
    src = GlslToWgsl::repairEmptyElseCommentBlocks(src);
    stage.setSourceCode(src);

    for (const string& issue : GlslToWgsl::findResidualGlsl(src))
    {
        std::cerr << "Warning: WgslShaderGenerator: " << issue << " in generated shader." << std::endl;
    }
}

string WgslShaderGenerator::expandAndRewriteGlsl(const string& source, const FilePath& sourceFilename,
                                                 GenContext& context, GlslToWgsl::LineRewriter& rewriter) const
{
    // Apply token substitutions ($fileTransformUv etc.) up front so include paths
    // and sampler/texture rewrites see resolved text. Apply longest keys first: some
    // tokens are prefixes of others (`$envRadiance` of `$envRadianceSamples`), and the
    // stock GLSL substitution values mask this by accident (`u_envRadiance`+`Samples` ==
    // `u_envRadianceSamples`) — but the split sampler values used here do not, so an
    // unordered pass would corrupt the longer token.
    std::vector<std::pair<string, string>> subs(getTokenSubstitutions().begin(),
                                                getTokenSubstitutions().end());
    std::sort(subs.begin(), subs.end(),
              [](const std::pair<string, string>& a, const std::pair<string, string>& b)
    {
        return a.first.length() > b.first.length();
    });
    string substituted = source;
    for (const auto& pair : subs)
    {
        if (pair.first.empty())
            continue;
        size_t pos = 0;
        while ((pos = substituted.find(pair.first, pos)) != string::npos)
        {
            substituted.replace(pos, pair.first.length(), pair.second);
            pos += pair.second.length();
        }
    }

    std::istringstream stream(substituted);
    string line;
    string out;
    out.reserve(substituted.size() + substituted.size() / 8);

    while (std::getline(stream, line))
    {
        const bool hadCr = !line.empty() && line.back() == '\r';
        if (hadCr)
            line.pop_back();

        const size_t firstNonSpace = line.find_first_not_of(" \t");
        const bool isInclude = (firstNonSpace != string::npos &&
                                line.compare(firstNonSpace, 8, "#include") == 0);
        if (isInclude)
        {
            const size_t q1 = line.find('"', firstNonSpace + 8);
            const size_t q2 = (q1 == string::npos) ? string::npos : line.find('"', q1 + 1);
            if (q1 == string::npos || q2 == string::npos || q2 <= q1 + 1)
                continue; // malformed include: drop it
            const string includePath = line.substr(q1 + 1, q2 - q1 - 1);
            // Expand each header only once per generation (avoids duplicate WGSL defs).
            auto seen = context.getUserData<WgslIncludeSet>(WGSL_INCLUDES);
            const string baseName = FilePath(includePath).getBaseName();
            if (seen && seen->basenames.count(baseName))
                continue;
            if (seen)
                seen->basenames.insert(baseName);
            const FilePath resolved = context.resolveSourceFile(includePath, sourceFilename.getParentPath());
            const string content = readFile(resolved);
            if (!content.empty())
                out += expandAndRewriteGlsl(content, resolved, context, rewriter);
            continue;
        }

        out += rewriter.rewrite(line);
        if (hadCr)
            out += '\r';
        out += '\n';
    }
    return out;
}

MATERIALX_NAMESPACE_END
