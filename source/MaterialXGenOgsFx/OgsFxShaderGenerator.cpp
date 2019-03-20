//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenOgsFx/OgsFxShaderGenerator.h>
#include <MaterialXGenOgsFx/OgsFxSyntax.h>

#include <MaterialXGenShader/Shader.h>

namespace MaterialX
{

namespace
{
    // Semantics used by OgsFx
    static const StringMap OGSFX_DEFAULT_SEMANTICS_MAP =
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

    static const StringMap OGSFX_GET_LIGHT_DATA_MAP =
    {
        { "type", "mx_getLightType" },
        { "position", "mx_getLightPos" },
        { "direction", "mx_getLightDir" },
        { "color", "mx_getLightColor" },
        { "intensity", "mx_getLightIntensity" },
        { "decay_rate", "mx_getLightDecayRate" },
        { "inner_angle", "mx_getLightConeAngle" },
        { "outer_angle", "mx_getLightPenumbraAngle" }
    };
}

namespace Stage
{
    const string EFFECT = "effect";
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
    ShaderStage& vs = shader->getStage(Stage::VERTEX);
    ShaderStage& ps = shader->getStage(Stage::PIXEL);
    ShaderStage& fx = shader->getStage(Stage::EFFECT);

    // Emit code for vertex and pixel shader stages
    emitVertexStage(graph, context, vs);
    emitPixelStage(graph, context, ps);

    //
    // Assemble the final effects shader
    //

    // Add version directive
    emitLine("#version " + getVersion(), fx, false);
    emitLineBreak(fx);

    // Add global constants and type definitions
    emitInclude("pbrlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_defines.glsl", context, fx);
    emitLine("#define MAX_LIGHT_SOURCES " + std::to_string(context.getOptions().hwMaxActiveLightSources), fx, false);
    emitLineBreak(fx);
    emitTypeDefinitions(context, fx);

    // Add vertex inputs block
    const VariableBlock& vertexInputs = vs.getInputBlock(HW::VERTEX_INPUTS);
    emitLine("attribute " + vertexInputs.getName(), fx, false);
    emitScopeBegin(fx);
    emitVariableDeclarations(vertexInputs, EMPTY_STRING, SEMICOLON, context, fx, false);
    emitScopeEnd(fx, true);
    emitLineBreak(fx);

    // Add vertex data outputs block
    const VariableBlock& vertexData = vs.getOutputBlock(HW::VERTEX_DATA);
    emitLine("attribute " + vertexData.getName(), fx, false);
    emitScopeBegin(fx);
    emitVariableDeclarations(vertexData, EMPTY_STRING, SEMICOLON, context, fx, false);
    emitScopeEnd(fx, true);
    emitLineBreak(fx);

    // Add the pixel shader output. This needs to be a vec4 for rendering
    // and upstream connection will be converted to vec4 below if needed.
    const ShaderGraphOutputSocket* outputSocket = graph.getOutputSocket();
    emitLine("attribute PixelOutput", fx, false);
    emitScopeBegin(fx);
    emitLine("vec4 " + outputSocket->getVariable(), fx);
    emitScopeEnd(fx, true);
    emitLineBreak(fx);

    // Add all private vertex shader uniforms
    {
        const VariableBlock& uniforms = vs.getUniformBlock(HW::PRIVATE_UNIFORMS);
        if (!uniforms.empty())
        {
            emitComment("Vertex stage uniform block: " + uniforms.getName(), fx);
            emitVariableDeclarations(uniforms, _syntax->getUniformQualifier(), SEMICOLON, context, fx);
            emitLineBreak(fx);
        }
    }
    // Add all public vertex shader uniforms
    {
        const VariableBlock& uniforms = vs.getUniformBlock(HW::PUBLIC_UNIFORMS);
        if (!uniforms.empty())
        {
            emitComment("Vertex stage uniform block: " + uniforms.getName(), fx);
            emitVariableDeclarations(uniforms, _syntax->getUniformQualifier(), SEMICOLON, context, fx);
            emitLineBreak(fx);
        }
    }
    // Add all private pixel shader uniforms
    {
        const VariableBlock& uniforms = ps.getUniformBlock(HW::PRIVATE_UNIFORMS);
        if (!uniforms.empty())
        {
            emitComment("Pixel stage uniform block: " + uniforms.getName(), fx);
            emitVariableDeclarations(uniforms, _syntax->getUniformQualifier(), SEMICOLON, context, fx);
            emitLineBreak(fx);
        }
    }
    // Add all public pixel shader uniforms
    {
        const VariableBlock& uniforms = ps.getUniformBlock(HW::PUBLIC_UNIFORMS);
        if (!uniforms.empty())
        {
            emitComment("Pixel stage uniform block: " + uniforms.getName(), fx);
            emitVariableDeclarations(uniforms, _syntax->getUniformQualifier(), SEMICOLON, context, fx);
            emitLineBreak(fx);
        }
    }

    bool lighting = graph.hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE) ||
                    graph.hasClassification(ShaderNode::Classification::BSDF);

    if (lighting)
    {
        // Add light struct declaration
        const VariableBlock& lightData = ps.getUniformBlock(HW::LIGHT_DATA);
        emitLine("struct " + lightData.getName(), fx, false);
        emitScopeBegin(fx);
        for (size_t i=0; i<lightData.size(); ++i)
        {
            const ShaderPort* uniform = lightData[i];
            const string& type = _syntax->getTypeName(uniform->getType());
            emitLine(type + " " + uniform->getName(), fx);
        }
        emitScopeEnd(fx, true);
        emitLineBreak(fx);

        // Emit OGS lighting uniforms
        emitInclude("pbrlib/" + GlslShaderGenerator::LANGUAGE + "/" + OgsFxShaderGenerator::TARGET + "/mx_lighting_uniforms.glsl", context, fx);
        emitLineBreak(fx);

        // Emit lighting functions
        emitLine("GLSLShader LightingFunctions", fx, false);
        emitScopeBegin(fx);
        emitInclude("pbrlib/" + GlslShaderGenerator::LANGUAGE + "/" + OgsFxShaderGenerator::TARGET + "/mx_lighting_functions.glsl", context, fx);
        emitLineBreak(fx);

        // Emit environment lighting functions
        if (context.getOptions().hwSpecularEnvironmentMethod == SPECULAR_ENVIRONMENT_FIS)
        {
            emitInclude("pbrlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_environment_fis.glsl", context, fx);
        }
        else
        {
            emitInclude("pbrlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_environment_prefilter.glsl", context, fx);
        }
        emitLineBreak(fx);

        emitScopeEnd(fx);
        emitLineBreak(fx);
    }

    // Add code blocks from the vertex and pixel shader stages generated above
    emitBlock(vs.getSourceCode(), context, fx);
    emitBlock(ps.getSourceCode(), context, fx);

    // Add Main technique block
    string techniqueParams;
    getTechniqueParams(*shader, techniqueParams);
    if (techniqueParams.size())
    {
        emitLine("technique Main< " + techniqueParams + " >", fx, false);
    }
    else
    {
        emitLine("technique Main", fx, false);
    }
    emitScopeBegin(fx);
    emitLine("pass p0", fx, false);
    emitScopeBegin(fx);
    emitLine("VertexShader(in VertexInputs, out VertexData vd) = { VS }", fx);
    emitLine(lighting ?
        "PixelShader(in VertexData vd, out PixelOutput) = { LightingFunctions, PS }" :
        "PixelShader(in VertexData vd, out PixelOutput) = { PS }", fx);
    emitScopeEnd(fx);
    emitScopeEnd(fx);
    emitLineBreak(fx);

    return shader;
}

void OgsFxShaderGenerator::emitVertexStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    emitComment("---------------------------------- Vertex shader ----------------------------------------\n", stage);
    emitLine("GLSLShader VS", stage, false);
    emitScopeBegin(stage);

    // Add all constants
    const VariableBlock& constants = stage.getConstantBlock();
    if (!constants.empty())
    {
        emitVariableDeclarations(constants, _syntax->getConstantQualifier(), SEMICOLON, context, stage);
        emitLineBreak(stage);
    }

    emitFunctionDefinitions(graph, context, stage);

    // Add main function
    emitLine("void main()", stage, false);
    emitScopeBegin(stage);
    emitLine("vec4 hPositionWorld = u_worldMatrix * vec4(i_position, 1.0)", stage);
    emitLine("gl_Position = u_viewProjectionMatrix * hPositionWorld", stage);
    emitFunctionCalls(graph, context, stage);
    emitScopeEnd(stage);
    emitScopeEnd(stage);
    emitLineBreak(stage);
}

void OgsFxShaderGenerator::emitPixelStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    emitComment("---------------------------------- Pixel shader ----------------------------------------\n", stage);
    emitLine("GLSLShader PS", stage, false);
    emitScopeBegin(stage);

    bool lighting = graph.hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE) ||
                    graph.hasClassification(ShaderNode::Classification::BSDF);

    if (lighting)
    {
        const VariableBlock& lightData = stage.getUniformBlock(HW::LIGHT_DATA);
        emitLine(lightData.getName() + " " + lightData.getInstance() + "[MAX_LIGHT_SOURCES]", stage);
        emitLineBreak(stage);
    }

    // Add all constants
    const VariableBlock& constants = stage.getConstantBlock();
    if (!constants.empty())
    {
        emitVariableDeclarations(constants, _syntax->getConstantQualifier(), SEMICOLON, context, stage);
        emitLineBreak(stage);
    }

    // Emit common math functions
    emitInclude("pbrlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_math.glsl", context, stage);
    emitLineBreak(stage);

    // Emit sampling code if needed
    if (graph.hasClassification(ShaderNode::Classification::CONVOLUTION2D))
    {
        // Emit sampling functions
        emitInclude("stdlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_sampling.glsl", context, stage);
        emitLineBreak(stage);
    }

    // Emit uv transform function
    if (context.getOptions().fileTextureVerticalFlip)
    {
        emitInclude("stdlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_get_target_uv_vflip.glsl", context, stage);
        emitLineBreak(stage);
    }
    else
    {
        emitInclude("stdlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_get_target_uv_noop.glsl", context, stage);
        emitLineBreak(stage);
    }

    // Add all functions for node implementations
    emitFunctionDefinitions(graph, context, stage);

    const ShaderGraphOutputSocket* outputSocket = graph.getOutputSocket();

    // Add main function
    emitLine("void main()", stage, false);
    emitScopeBegin(stage);

    if (graph.hasClassification(ShaderNode::Classification::CLOSURE))
    {
        // Handle the case where the graph is a direct closure.
        // We don't support rendering closures without attaching 
        // to a surface shader, so just output black.
        emitLine(outputSocket->getVariable() + " = vec4(0.0, 0.0, 0.0, 1.0)", stage);
    }
    else
    {
        if (lighting)
        {
            // Add code for retreiving light data from OgsFx light uniforms
            emitScopeBegin(stage);
            emitLine("int numLights = numActiveLightSources()", stage);
            emitLine("for (int i = 0; i<numLights; ++i)", stage, false);
            emitScopeBegin(stage);
            const VariableBlock& lightData = stage.getUniformBlock(HW::LIGHT_DATA);
            for (size_t i = 0; i < lightData.size(); ++i)
            {
                const ShaderPort* uniform = lightData[i];
                auto it = OGSFX_GET_LIGHT_DATA_MAP.find(uniform->getName());
                if (it != OGSFX_GET_LIGHT_DATA_MAP.end())
                {
                    emitLine(lightData.getInstance() + "[i]." + uniform->getName() + " = " + it->second + "(i)", stage);
                }
            }
            emitScopeEnd(stage);
            emitScopeEnd(stage);
            emitLineBreak(stage);
        }

        // Add all function calls
        emitFunctionCalls(graph, context, stage);

        // Emit final output
        if (outputSocket->getConnection())
        {
            string finalOutput = outputSocket->getConnection()->getVariable();

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

    emitScopeEnd(stage);
    emitScopeEnd(stage);
    emitLineBreak(stage);
}

void OgsFxShaderGenerator::emitVariableDeclaration(const ShaderPort* variable, const string& qualifier,
                                                   GenContext&, ShaderStage& stage,
                                                   bool assignValue) const
{
    // A file texture input needs special handling on GLSL
    if (variable->getType() == Type::FILENAME)
    {
        string str = "uniform texture2D " + variable->getVariable() + "_texture : SourceTexture;\n" \
                     "uniform sampler2D " + variable->getVariable() + " = sampler_state\n"         \
                     "{\n    Texture = <" + variable->getVariable() + "_texture>;\n}";
        emitString(str, stage);
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

        emitString(str, stage);
    }
}

ShaderPtr OgsFxShaderGenerator::createShader(const string& name, ElementPtr element, GenContext& context) const
{
    ShaderPtr shader = GlslShaderGenerator::createShader(name, element, context);
    createStage(Stage::EFFECT, *shader);

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
