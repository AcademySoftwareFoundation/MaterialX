#include <MaterialXGenOgsFx/OgsFxShaderGenerator.h>
#include <MaterialXGenOgsFx/OgsFxSyntax.h>

namespace MaterialX
{

namespace
{
    // Semantics used by OgsFx
    static const std::unordered_map<string,string> OGSFX_DEFAULT_SEMANTICS_MAP =
    {
        { "i_position", "POSITION"},
        { "i_normal", "NORMAL" },
        { "i_tangent", "TANGENT" },
        { "i_bitangent", "BINORMAL" },

        { "i_texcoord_0", "TEXCOORD0" },
        { "i_texcoord_1", "TEXCOORD1" },
        { "i_texcoord_2", "TEXCOORD2" },
        { "i_texcoord_3", "TEXCOORD3" },
        { "i_texcoord_4", "TEXCOORD4" },
        { "i_texcoord_5", "TEXCOORD5" },
        { "i_texcoord_6", "TEXCOORD6" },
        { "i_texcoord_7", "TEXCOORD7" },

        { "i_color_0", "COLOR0" },

        { "u_worldMatrix", "World" },
        { "u_worldInverseMatrix", "WorldInverse" },
        { "u_worldTransposeMatrix", "WorldTranspose" },
        { "u_worldInverseTransposeMatrix", "WorldInverseTranspose" },

        { "u_viewMatrix", "View" },
        { "u_viewInverseMatrix", "ViewInverse" },
        { "u_viewTransposeMatrix", "ViewTranspose" },
        { "u_viewInverseTransposeMatrix", "ViewInverseTranspose" },

        { "u_projectionMatrix", "Projection" },
        { "u_projectionInverseMatrix", "ProjectionInverse" },
        { "u_projectionTransposeMatrix", "ProjectionTranspose" },
        { "u_projectionInverseTransposeMatrix", "ProjectionInverseTranspose" },

        { "u_worldViewMatrix", "WorldView" },
        { "u_viewProjectionMatrix", "ViewProjection" },
        { "u_worldViewProjectionMatrix", "WorldViewProjection" },

        { "u_viewDirection", "ViewDirection" },
        { "u_viewPosition", "WorldCameraPosition" },

        { "u_frame", "Frame" },
        { "u_time", "Time" }
    };

    static const std::unordered_map<string, string> OGSFX_GET_LIGHT_DATA_MAP =
    {
        { "type", "mx_getLightType" },
        { "position", "mx_getLightPos" },
        { "direction", "mx_getLightDir" },
        { "color", "mx_getLightColor" },
        { "intensity", "mx_getLightIntensity" },
        { "decayRate", "mx_getLightDecayRate" },
        { "innerConeAngle", "mx_getLightConeAngle" },
        { "outerConeAngle", "mx_getLightPenumbraAngle" }
    };
}

namespace HW
{
    const string FX_STAGE = "fx";
}

const string OgsFxShaderGenerator::TARGET = "ogsfx";

OgsFxShaderGenerator::OgsFxShaderGenerator()
    : GlslShaderGenerator()
{
    _syntax = OgsFxSyntax::create();
}

ShaderGeneratorPtr OgsFxShaderGenerator::create()
{
    return std::make_shared<OgsFxShaderGenerator>();
}

ShaderPtr OgsFxShaderGenerator::generate(const string& name, ElementPtr element, GenContext& context) const
{
    ShaderPtr shader = createShader(name, element, context);

    // Turn on fixed formatting since OgsFx doesn't support scientific values
    Value::ScopedFloatFormatting fmt(Value::FloatFormatFixed);

    const ShaderGraph& graph = shader->getGraph();
    ShaderStage& vs = shader->getStage(HW::VERTEX_STAGE);
    ShaderStage& ps = shader->getStage(HW::PIXEL_STAGE);
    ShaderStage& fx = shader->getStage(HW::FX_STAGE);

    // Emit code for vertex and pixel shader stages
    emitVertexStage(vs, graph, context);
    emitPixelStage(ps, graph, context);

    //
    // Assemble the final effects shader
    //

    // Add version directive
    emitLine(fx, "#version " + getVersion(), false);
    emitLineBreak(fx);

    // Add global constants and type definitions
    emitInclude(fx, "pbrlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_defines.glsl", context);
    emitLine(fx, "#define MAX_LIGHT_SOURCES " + std::to_string(getMaxActiveLightSources()), false);
    emitLineBreak(fx);
    emitTypeDefinitions(fx);

    // Add vertex inputs block
    const VariableBlock& vertexInputs = vs.getInputBlock(HW::VERTEX_INPUTS);
    if (!vertexInputs.empty())
    {
        emitLine(fx, "attribute " + vertexInputs.getName(), false);
        emitScopeBegin(fx, ShaderStage::Brackets::BRACES);
        emitVariableDeclarations(fx, vertexInputs, EMPTY_STRING, SEMICOLON, false);
        emitScopeEnd(fx, true);
        emitLineBreak(fx);
    }

    // Add vertex data outputs block
    const VariableBlock& vertexData = vs.getOutputBlock(HW::VERTEX_DATA);
    if (!vertexData.empty())
    {
        emitLine(fx, "attribute " + vertexData.getName(), false);
        emitScopeBegin(fx, ShaderStage::Brackets::BRACES);
        emitVariableDeclarations(fx, vertexData, EMPTY_STRING, SEMICOLON, false);
        emitScopeEnd(fx, true);
        emitLineBreak(fx);
    }

    // Add the pixel shader output. This needs to be a vec4 for rendering
    // and upstream connection will be converted to vec4 below if needed.
    const ShaderGraphOutputSocket* outputSocket = graph.getOutputSocket();
    emitLine(fx, "attribute PixelOutput", false);
    emitScopeBegin(fx, ShaderStage::Brackets::BRACES);
    emitLine(fx, "vec4 " + outputSocket->getVariable());
    emitScopeEnd(fx, true);
    emitLineBreak(fx);

    // Add all private vertex shader uniforms
    {
        const VariableBlock& uniforms = vs.getUniformBlock(HW::PRIVATE_UNIFORMS);
        if (!uniforms.empty())
        {
            emitComment(fx, "Vertex stage uniform block: " + uniforms.getName());
            emitVariableDeclarations(fx, uniforms, _syntax->getUniformQualifier(), SEMICOLON);
            emitLineBreak(fx);
        }
    }
    // Add all public vertex shader uniforms
    {
        const VariableBlock& uniforms = vs.getUniformBlock(HW::PUBLIC_UNIFORMS);
        if (!uniforms.empty())
        {
            emitComment(fx, "Vertex stage uniform block: " + uniforms.getName());
            emitVariableDeclarations(fx, uniforms, _syntax->getUniformQualifier(), SEMICOLON);
            emitLineBreak(fx);
        }
    }
    // Add all private pixel shader uniforms
    {
        const VariableBlock& uniforms = ps.getUniformBlock(HW::PRIVATE_UNIFORMS);
        if (!uniforms.empty())
        {
            emitComment(fx, "Pixel stage uniform block: " + uniforms.getName());
            emitVariableDeclarations(fx, uniforms, _syntax->getUniformQualifier(), SEMICOLON);
            emitLineBreak(fx);
        }
    }
    // Add all public pixel shader uniforms
    {
        const VariableBlock& uniforms = ps.getUniformBlock(HW::PUBLIC_UNIFORMS);
        if (!uniforms.empty())
        {
            emitComment(fx, "Pixel stage uniform block: " + uniforms.getName());
            emitVariableDeclarations(fx, uniforms, _syntax->getUniformQualifier(), SEMICOLON);
            emitLineBreak(fx);
        }
    }

    bool lighting = graph.hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE) ||
                    graph.hasClassification(ShaderNode::Classification::BSDF);

    if (lighting)
    {
        // Add light struct declaration
        const VariableBlock& lightData = ps.getUniformBlock(HW::LIGHT_DATA);
        emitLine(fx, "struct " + lightData.getName(), false);
        emitScopeBegin(fx, ShaderStage::Brackets::BRACES);
        for (size_t i=0; i<lightData.size(); ++i)
        {
            const ShaderPort* uniform = lightData[i];
            const string& type = _syntax->getTypeName(uniform->getType());
            emitLine(fx, type + " " + uniform->getName());
        }
        emitScopeEnd(fx, true);
        emitLineBreak(fx);

        // Emit OGS lighting uniforms
        emitInclude(fx, "pbrlib/" + GlslShaderGenerator::LANGUAGE + "/" + OgsFxShaderGenerator::TARGET + "/mx_lighting_uniforms.glsl", context);
        emitLineBreak(fx);

        // Emit lighting functions
        emitLine(fx, "GLSLShader LightingFunctions", false);
        emitScopeBegin(fx, ShaderStage::Brackets::BRACES);
        emitInclude(fx, "pbrlib/" + GlslShaderGenerator::LANGUAGE + "/" + OgsFxShaderGenerator::TARGET + "/mx_lighting_functions.glsl", context);
        emitLineBreak(fx);

        // Emit environment lighting functions
        if (context.getOptions().hwSpecularEnvironmentMethod == SPECULAR_ENVIRONMENT_FIS)
        {
            emitInclude(fx, "pbrlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_environment_fis.glsl", context);
        }
        else
        {
            emitInclude(fx, "pbrlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_environment_prefilter.glsl", context);
        }
        emitLineBreak(fx);

        emitScopeEnd(fx);
        emitLineBreak(fx);
    }

    // Add code blocks from the vertex and pixel shader stages generated above
    emitBlock(fx, vs.getSourceCode(), context);
    emitBlock(fx, ps.getSourceCode(), context);

    // Add Main technique block
    string techniqueParams;
    getTechniqueParams(*shader, techniqueParams);
    if (techniqueParams.size())
    {
        emitLine(fx, "technique Main< " + techniqueParams + " >", false);
    }
    else
    {
        emitLine(fx, "technique Main", false);
    }
    emitScopeBegin(fx, ShaderStage::Brackets::BRACES);
    emitLine(fx, "pass p0", false);
    emitScopeBegin(fx, ShaderStage::Brackets::BRACES);
    emitLine(fx, "VertexShader(in AppData, out VertexData vd) = { VS }");
    emitLine(fx, lighting ?
        "PixelShader(in VertexData vd, out PixelOutput) = { LightingFunctions, PS }" :
        "PixelShader(in VertexData vd, out PixelOutput) = { PS }");
    emitScopeEnd(fx);
    emitScopeEnd(fx);
    emitLineBreak(fx);

    return shader;
}

void OgsFxShaderGenerator::emitVertexStage(ShaderStage& stage, const ShaderGraph& graph, GenContext& context) const
{
    emitComment(stage, "---------------------------------- Vertex shader ----------------------------------------\n");
    emitLine(stage, "GLSLShader VS", false);
    emitScopeBegin(stage, ShaderStage::Brackets::BRACES);

    // Add all constants
    const VariableBlock& constants = stage.getConstantBlock();
    if (!constants.empty())
    {
        emitVariableDeclarations(stage, constants, _syntax->getConstantQualifier(), SEMICOLON);
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

void OgsFxShaderGenerator::emitPixelStage(ShaderStage& stage, const ShaderGraph& graph, GenContext& context) const
{
    emitComment(stage, "---------------------------------- Pixel shader ----------------------------------------\n");
    emitLine(stage, "GLSLShader PS", false);
    emitScopeBegin(stage, ShaderStage::Brackets::BRACES);

    bool lighting = graph.hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE) ||
                    graph.hasClassification(ShaderNode::Classification::BSDF);

    if (lighting)
    {
        const VariableBlock& lightData = stage.getUniformBlock(HW::LIGHT_DATA);
        emitLine(stage, lightData.getName() + " " + lightData.getInstance() + "[MAX_LIGHT_SOURCES]");
        emitLineBreak(stage);
    }

    // Add all constants
    const VariableBlock& constants = stage.getConstantBlock();
    if (!constants.empty())
    {
        emitVariableDeclarations(stage, constants, _syntax->getConstantQualifier(), SEMICOLON);
        emitLineBreak(stage);
    }

    // Emit common math functions
    emitInclude(stage, "pbrlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_math.glsl", context);
    emitLineBreak(stage);

    // Emit sampling code if needed
    if (graph.hasClassification(ShaderNode::Classification::CONVOLUTION2D))
    {
        // Emit sampling functions
        emitInclude(stage, "stdlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_sampling.glsl", context);
        emitLineBreak(stage);
    }

    // Emit uv transform function
    if (context.getOptions().fileTextureVerticalFlip)
    {
        emitInclude(stage, "stdlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_get_target_uv_vflip.glsl", context);
        emitLineBreak(stage);
    }
    else
    {
        emitInclude(stage, "stdlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_get_target_uv_noop.glsl", context);
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
        emitLine(stage, outputSocket->getVariable() + " = vec4(0.0, 0.0, 0.0, 1.0)");
    }
    else
    {
        if (lighting)
        {
            // Add code for retreiving light data from OgsFx light uniforms
            emitScopeBegin(stage);
            emitLine(stage, "int numLights = numActiveLightSources()");
            emitLine(stage, "for (int i = 0; i<numLights; ++i)", false);
            emitScopeBegin(stage, ShaderStage::Brackets::BRACES);
            const VariableBlock& lightData = stage.getUniformBlock(HW::LIGHT_DATA);
            for (size_t i = 0; i < lightData.size(); ++i)
            {
                const ShaderPort* uniform = lightData[i];
                auto it = OGSFX_GET_LIGHT_DATA_MAP.find(uniform->getName());
                if (it != OGSFX_GET_LIGHT_DATA_MAP.end())
                {
                    emitLine(stage, lightData.getInstance() + "[i]." + uniform->getName() + " = " + it->second + "(i)");
                }
            }
            emitScopeEnd(stage);
            emitScopeEnd(stage);
            emitLineBreak(stage);
        }

        // Add all function calls
        emitFunctionCalls(stage, graph, context);

        // Emit final output
        if (outputSocket->getConnection())
        {
            string finalOutput = outputSocket->getConnection()->getVariable();

            if (graph.hasClassification(ShaderNode::Classification::SURFACE))
            {
                if (context.getOptions().hwTransparency)
                {
                    emitLine(stage, "float outAlpha = clamp(1.0 - dot(" + finalOutput + ".transparency, vec3(0.3333)), 0.0, 1.0)");
                    emitLine(stage, outputSocket->getVariable() + " = vec4(" + finalOutput + ".color, outAlpha)");
                }
                else
                {
                    emitLine(stage, outputSocket->getVariable() + " = vec4(" + finalOutput + ".color, 1.0)");
                }
            }
            else
            {
                if (!outputSocket->getType()->isFloat4())
                {
                    toVec4(outputSocket->getType(), finalOutput);
                }
                emitLine(stage, outputSocket->getVariable() + " = " + finalOutput);
            }
        }
        else
        {
            string outputValue = outputSocket->getValue() ? _syntax->getValue(outputSocket->getType(), *outputSocket->getValue()) : _syntax->getDefaultValue(outputSocket->getType());
            if (!outputSocket->getType()->isFloat4())
            {
                string finalOutput = outputSocket->getVariable() + "_tmp";
                emitLine(stage, _syntax->getTypeName(outputSocket->getType()) + " " + finalOutput + " = " + outputValue);
                toVec4(outputSocket->getType(), finalOutput);
                emitLine(stage, outputSocket->getVariable() + " = " + finalOutput);
            }
            else
            {
                emitLine(stage, outputSocket->getVariable() + " = " + outputValue);
            }
        }
    }

    emitScopeEnd(stage);
    emitScopeEnd(stage);
    emitLineBreak(stage);
}

void OgsFxShaderGenerator::emitVariableDeclaration(ShaderStage& stage, const ShaderPort* variable,
                                                   const string& qualifier, bool assignValue) const
{
    // A file texture input needs special handling on GLSL
    if (variable->getType() == Type::FILENAME)
    {
        string str = "uniform texture2D " + variable->getVariable() + "_texture : SourceTexture\n" \
                     "uniform sampler2D " + variable->getVariable() + " = sampler_state\n"         \
                     "{\n    Texture = <" + variable->getVariable() + "_texture>;\n}";
        emitString(stage, str);
    }
    else
    {
        string str = qualifier.empty() ? EMPTY_STRING : qualifier + " ";
        str += _syntax->getTypeName(variable->getType()) + " " + variable->getVariable();

        // If an array we need an array qualifier (suffix) for the variable name
        if (variable->getType()->isArray() && variable->getValue())
        {
            str += _syntax->getArraySuffix(variable->getType(), *variable->getValue());
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

        emitString(stage, str);
    }
}

ShaderPtr OgsFxShaderGenerator::createShader(const string& name, ElementPtr element, GenContext& context) const
{
    ShaderPtr shader = GlslShaderGenerator::createShader(name, element, context);
    createStage(*shader, HW::FX_STAGE);

    // Update semantics to match OGSFX semantics.
    for (size_t i = 0; i < shader->numStages(); ++i)
    {
        ShaderStage& stage = shader->getStage(i);

        for (auto it : stage.getInputBlocks())
        {
            VariableBlock& block = *it.second;
            for (size_t j = 0; j < block.size(); ++j)
            {
                ShaderPort* v = block[j];
                auto sematic = OGSFX_DEFAULT_SEMANTICS_MAP.find(v->getName());
                if (sematic != OGSFX_DEFAULT_SEMANTICS_MAP.end())
                {
                    v->setSemantic(sematic->second);
                }
            }
        }
        for (auto it : stage.getUniformBlocks())
        {
            VariableBlock& block = *it.second;
            for (size_t j = 0; j < block.size(); ++j)
            {
                ShaderPort* v = block[j];
                auto sematic = OGSFX_DEFAULT_SEMANTICS_MAP.find(v->getName());
                if (sematic != OGSFX_DEFAULT_SEMANTICS_MAP.end())
                {
                    v->setSemantic(sematic->second);
                }
            }
        }
    }

    return shader;
}

void OgsFxShaderGenerator::getTechniqueParams(const Shader&, string&) const
{
    // Default implementation doesn't use any technique parameters
}

} // namespace MaterialX
