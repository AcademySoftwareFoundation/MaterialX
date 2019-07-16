//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenOgsXml/GlslFragmentGenerator.h>
#include <MaterialXGenOgsXml/OgsXmlGenerator.h>
#include <MaterialXGenGlsl/GlslShaderGenerator.h>

#include <MaterialXGenShader/Shader.h>

namespace MaterialX
{

string GlslFragmentSyntax::getVariableName(const string& name, const TypeDesc* type, GenContext& context) const
{
    string variable = GlslSyntax::getVariableName(name, type, context);
    // A filename input corresponds to a texture sampler uniform
    // which requires a special suffix in OGS XML fragments.
    if (type == Type::FILENAME)
    {
        // Make sure it's not already used.
        if (variable.size() <= OgsXmlGenerator::SAMPLER_SUFFIX.size() || 
            variable.substr(variable.size() - OgsXmlGenerator::SAMPLER_SUFFIX.size()) != OgsXmlGenerator::SAMPLER_SUFFIX)
        {
            variable += OgsXmlGenerator::SAMPLER_SUFFIX;
        }
    }
    return variable;
}

const string GlslFragmentGenerator::TARGET = "ogsxml";
const string GlslFragmentGenerator::MATRIX3_TO_MATRIX4_POSTFIX = "4";

GlslFragmentGenerator::GlslFragmentGenerator() :
    GlslShaderGenerator()
{
    // Use our custom syntax class
    _syntax = std::make_shared<GlslFragmentSyntax>();

    // Set identifier names to match OGS naming convention.
    _tokenSubstitutions[HW::T_POSITION_WORLD]       = "Pw";
    _tokenSubstitutions[HW::T_POSITION_OBJECT]      = "Pm";
    _tokenSubstitutions[HW::T_NORMAL_WORLD]         = "Nw";
    _tokenSubstitutions[HW::T_NORMAL_OBJECT]        = "Nm";
    _tokenSubstitutions[HW::T_TANGENT_WORLD]        = "Tw";
    _tokenSubstitutions[HW::T_TANGENT_OBJECT]       = "Tm";
    _tokenSubstitutions[HW::T_BITANGENT_WORLD]      = "Bw";
    _tokenSubstitutions[HW::T_BITANGENT_OBJECT]     = "Bm";
    _tokenSubstitutions[HW::T_VERTEX_DATA_INSTANCE] = "PIX_IN";
    _tokenSubstitutions[HW::T_ENV_IRRADIANCE]       = "u_envIrradianceSampler";
    _tokenSubstitutions[HW::T_ENV_RADIANCE]         = "u_envRadianceSampler";
}

ShaderGeneratorPtr GlslFragmentGenerator::create()
{
    return std::make_shared<GlslFragmentGenerator>();
}

ShaderPtr GlslFragmentGenerator::generate(const string& name, ElementPtr element, GenContext& context) const
{
    resetIdentifiers(context);
    ShaderPtr shader = createShader(name, element, context);

    ShaderStage& stage = shader->getStage(Stage::PIXEL);

    // Turn on fixed float formatting to make sure float values are
    // emitted with a decimal point and not as integers, and to avoid
    // any scientific notation which isn't supported by all OpenGL targets.
    ScopedFloatFormatting fmt(Value::FloatFormatFixed);

    const ShaderGraph& graph = shader->getGraph();

    // Add global constants and type definitions
    emitInclude("pbrlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_defines.glsl", context, stage);
    const unsigned int maxLights = std::max(1u, context.getOptions().hwMaxActiveLightSources);
    emitLine("#define MAX_LIGHT_SOURCES " + std::to_string(maxLights), stage, false);
    emitLineBreak(stage);
    emitTypeDefinitions(context, stage);

    // Add all constants
    const VariableBlock& constants = stage.getConstantBlock();
    if (!constants.empty())
    {
        emitVariableDeclarations(constants, _syntax->getConstantQualifier(), SEMICOLON, context, stage);
        emitLineBreak(stage);
    }

    bool lighting = graph.hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE) ||
                    graph.hasClassification(ShaderNode::Classification::BSDF);

    // Emit lighting functions
    if (lighting)
    {
        const VariableBlock& lightData = stage.getUniformBlock(HW::LIGHT_DATA);
        emitLine("struct " + lightData.getName(), stage, false);
        emitScopeBegin(stage);
        emitVariableDeclarations(lightData, EMPTY_STRING, SEMICOLON, context, stage, false);
        emitScopeEnd(stage, true);
        emitLineBreak(stage);
        emitLine("uniform " + lightData.getName() + " " + lightData.getInstance() + "[MAX_LIGHT_SOURCES]", stage);
        emitLineBreak(stage);
    }

    // Emit common math functions
    emitInclude("pbrlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_math.glsl", context, stage);
    emitLineBreak(stage);

    if (lighting)
    {
        emitSpecularEnvironment(context, stage);
    }

    // Emit sampling code if needed
    if (graph.hasClassification(ShaderNode::Classification::CONVOLUTION2D))
    {
        // Emit sampling functions
        emitInclude("stdlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_sampling.glsl", context, stage);
        emitLineBreak(stage);
    }

    // Emit uv transform function
    if (!context.getOptions().fileTextureVerticalFlip)
    {
        emitInclude("stdlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_transform_uv.glsl", context, stage);
    }
    else
    {
        emitInclude("stdlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_transform_uv_vflip.glsl", context, stage);
    }

    // Add all functions for node implementations
    emitFunctionDefinitions(graph, context, stage);

    const ShaderGraphOutputSocket* outputSocket = graph.getOutputSocket();

    // Add function signature
    // Keep track of arguments we changed from matrix3 to matrix4 as additional
    // code must be inserted to get back the matrix3 version
    StringVec convertMatrixStrings;

    string functionName = shader->getName();
    context.makeIdentifier(functionName);
    setFunctionName(functionName, stage);

    emitLine((context.getOptions().hwTransparency ? "vec4 " : "vec3 ") + functionName, stage, false);

    emitScopeBegin(stage, Syntax::PARENTHESES);
    const VariableBlock& uniforms = stage.getUniformBlock(HW::PUBLIC_UNIFORMS);
    const size_t numUniforms = uniforms.size();
    for (size_t i = 0; i < numUniforms; ++i)
    {
        emitLineBegin(stage);
        if (uniforms[i]->getType() == Type::MATRIX33)
        {
            convertMatrixStrings.push_back(uniforms[i]->getVariable());
        }
        emitVariableDeclaration(uniforms[i], EMPTY_STRING, context, stage, false);
        if (i < numUniforms - 1)
        {
            emitString(COMMA, stage);
        }
        emitLineEnd(stage, false);
    }
    // Special case handling of world space normals which for now must be added 
    // as a "dummy" argument if it exists.
    const VariableBlock& streams = stage.getInputBlock(HW::VERTEX_DATA);
    const ShaderPort* port = streams.find(HW::T_NORMAL_WORLD);
    if (port)
    { 
        emitLineBegin(stage);
        emitString(COMMA, stage);
        emitVariableDeclaration(port, EMPTY_STRING, context, stage, false);
        emitLineEnd(stage, false);
    }

    if (context.getOptions().hwTransparency)
    {
        // A dummy argument not used in the generated shader code but necessary to
        // map onto an OGS fragment parameter and a shading node DG attribute with
        // the same name that can be set to a non-0 value to let Maya know that the
        // surface is transparent.
        emitLineBegin(stage);
        emitString(COMMA, stage);
        emitString("float ", stage);
        emitString(OgsXmlGenerator::VP_TRANSPARENCY_NAME, stage);
        emitLineEnd(stage, false);
    }

    emitScopeEnd(stage);

    // Add function body
    emitScopeBegin(stage);

    if (graph.hasClassification(ShaderNode::Classification::CLOSURE))
    {
        // Handle the case where the graph is a direct closure.
        // We don't support rendering closures without attaching 
        // to a surface shader, so just output black.
        emitLine("return vec3(0.0)", stage);
    }
    else
    {
        // Insert matrix converters
        for (const string& argument : convertMatrixStrings)
        {
            emitLine("mat3 " + argument + " = mat3(" + argument + GlslFragmentGenerator::MATRIX3_TO_MATRIX4_POSTFIX + ")", stage, true);
        }

        // Add all function calls
        emitFunctionCalls(graph, context, stage);

        // Emit final result
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
                    emitLine("return vec4(" + finalOutput + ".color, clamp(1.0 - dot(" + finalOutput + ".transparency, vec3(0.3333)), 0.0, 1.0))", stage);
                }
                else
                {
                    emitLine("return " + finalOutput + ".color", stage);
                }
            }
            else
            {
                if (!outputSocket->getType()->isFloat3())
                {
                    toVec3(outputSocket->getType(), finalOutput);
                }
                emitLine("return " + finalOutput, stage);
            }
        }
        else
        {
            string outputValue = outputSocket->getValue() ? _syntax->getValue(outputSocket->getType(), *outputSocket->getValue()) : _syntax->getDefaultValue(outputSocket->getType());
            if (!outputSocket->getType()->isFloat3())
            {
                string finalOutput = outputSocket->getVariable() + "_tmp";
                emitLine(_syntax->getTypeName(outputSocket->getType()) + " " + finalOutput + " = " + outputValue, stage);
                toVec3(outputSocket->getType(), finalOutput);
                emitLine("return " + finalOutput, stage);
            }
            else
            {
                emitLine("return " + outputValue, stage);
            }
        }
    }

    // End function
    emitScopeEnd(stage);

    // Replace all tokens with real identifier names
    replaceTokens(_tokenSubstitutions, stage);

    return shader;
}

void GlslFragmentGenerator::toVec3(const TypeDesc* type, string& variable)
{
    if (type->isFloat2())
    {
        variable = "vec3(" + variable + ", 0.0)";
    }
    else if (type->isFloat4())
    {
        variable = variable + ".xyz";
    }
    else if (type == Type::FLOAT || type == Type::INTEGER)
    {
        variable = "vec3(" + variable + ", " + variable + ", " + variable + ")";
    }
    else if (type == Type::BSDF || type == Type::EDF)
    {
        variable = "vec3(" + variable + ")";
    }
    else
    {
        // Can't understand other types. Just return black.
        variable = "vec3(0.0, 0.0, 0.0)";
    }
}

void GlslFragmentGenerator::emitVariableDeclaration(const ShaderPort* variable, const string& qualifier,
                                                    GenContext& context, ShaderStage& stage,
                                                    bool assignValue) const
{
    if (variable->getType() == Type::FILENAME)
    {
        emitString("sampler2D " + variable->getVariable(), stage);
    }
    // We change matrix3 to matrix4 input arguments
    else if (variable->getType() == Type::MATRIX33)
    {
        emitString("mat4 " + variable->getVariable() + MATRIX3_TO_MATRIX4_POSTFIX, stage);
    }
    else
    {
        GlslShaderGenerator::emitVariableDeclaration(variable, qualifier, context, stage, assignValue);
    }
}

} // namespace MaterialX
