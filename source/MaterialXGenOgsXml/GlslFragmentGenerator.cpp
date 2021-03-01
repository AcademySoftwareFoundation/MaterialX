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

namespace Stage
{
    const string UNIFORMS = "uniforms";
}

namespace
{
    // This is the same algorithm as found in libraries\pbrlib\genglsl\lib\mx_environment_prefilter.glsl
    // but adjusted for Maya. At this time we will compute a roughness based on the radiance and
    // irradiance samples, so materials with small amount of roughness will look wrong.
    //
    // A more precise roughness computation can be done using Maya samplers, but this requires
    // knowing that the Maya sampling functions are there, otherwise compilation will fail unless
    // there is an IBL active in the Maya lighting.
    //
    // **Maya does not currently declare the list of defines you see below** so do not expect correct
    // environment at this point in time.
    const string MX_ENVIRONMENT_MAYA =
        "#include \"pbrlib/genglsl/lib/mx_microfacet_specular.glsl\"\r\n"
        "vec3 mx_environment_irradiance(vec3 N)\r\n"
        "{\r\n"
        "    return g_diffuseI;\r\n"
        "}\r\n"
        "#ifdef MAYA_HAS_mayaRoughnessToPhongExp\r\n"
        "#ifdef MAYA_HAS_adjustGain\r\n"
        "#ifdef MAYA_HAS_SampleLatLongReflect\r\n"
        "#ifdef MAYA_HAS_SampleLatLongVolumeReflect\r\n"
        "#ifdef MAYA_HAS_blendGlossyAndRadianceEnvironment\r\n"
        "#define MATERIALX_USE_MAYA_SAMPLERS\r\n"
        "#endif\r\n"
        "#endif\r\n"
        "#endif\r\n"
        "#endif\r\n"
        "#endif\r\n"
        "vec3 mx_environment_radiance(vec3 N, vec3 V, vec3 X, vec2 roughness, int distribution, FresnelData fd)\r\n"
        "{\r\n"
        "    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);\r\n"
        "    float avgRoughness = mx_average_roughness(roughness);\r\n"
        "    vec3 F = mx_compute_fresnel(NdotV, fd);\r\n"
        "    float G = mx_ggx_smith_G(NdotV, NdotV, avgRoughness);\r\n"
        "    vec3 comp = mx_ggx_energy_compensation(NdotV, avgRoughness, F);\r\n"
        "#ifdef MATERIALX_USE_MAYA_SAMPLERS\r\n"
        "    float phongExp = mayaRoughnessToPhongExp(sqrt(avgRoughness));\r\n"
        "    vec3 radiance = adjustGain(SampleLatLongReflect(radiance_samp, radiance_transform, radiance_reflectionCoeff, V, N), radiance_gain);\r\n"
        "    vec3 glossy = adjustGain( SampleLatLongVolumeReflect(glossy_samp, glossy_transform, glossy_reflectionCoeff, V, N, phongExp, minExponent, maxExponent, exponentCount), glossy_gain);\r\n"
        "    vec3 Li = blendGlossyAndRadianceEnvironment(phongExp, maxExponent, glossy, radiance);\r\n"
        "#else\r\n"
        "    vec3 Li = mix(g_specularI, g_diffuseI, avgRoughness);\r\n"
        "#endif\r\n"
        "    return Li * F * G * comp;\r\n"
        "}\r\n";

    // Lighting support found in the materialXLightDataBuilder fragment found in
    // source\MaterialXContrib\MaterialXMaya\vp2ShaderFragments.
    const string LIGHT_LOOP_RESULT = "lightLoopResult";
    const string MAYA_ENV_IRRADIANCE_SAMPLE = "diffuseI";
    const string MAYA_ENV_RADIANCE_SAMPLE = "specularI";
    const string MAYA_ENV_ROUGHNESS = "roughness";
}

string GlslFragmentSyntax::getVariableName(const string& name, const TypeDesc* type, IdentifierMap& identifiers) const
{
    string variable = GlslSyntax::getVariableName(name, type, identifiers);
    // A filename input corresponds to a texture sampler uniform
    // which requires a special suffix in OGS XML fragments.
    if (type == Type::FILENAME)
    {
        // Make sure it's not already used.
        if (!OgsXmlGenerator::isSamplerName(variable))
        {
            variable = OgsXmlGenerator::textureToSamplerName(variable);
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
    _tokenSubstitutions[HW::T_VERTEX_DATA_INSTANCE] = "g_mxVertexData"; // name of a global non-const variable
    _tokenSubstitutions[HW::T_LIGHT_DATA_INSTANCE]  = "g_lightData"; // Store Maya lights in global non-const
    _tokenSubstitutions[HW::T_NUM_ACTIVE_LIGHT_SOURCES] = "g_numActiveLightSources";
}

ShaderGeneratorPtr GlslFragmentGenerator::create()
{
    return std::make_shared<GlslFragmentGenerator>();
}

ShaderPtr GlslFragmentGenerator::createShader(const string& name, ElementPtr element, GenContext& context) const
{
    ShaderPtr shader = GlslShaderGenerator::createShader(name, element, context);
    ShaderGraph& graph = shader->getGraph();
    ShaderStage& pixelStage = shader->getStage(Stage::PIXEL);

    // Add uniforms for environment lighting.
    if (requiresLighting(graph))
    {
        VariableBlock& psPrivateUniforms = pixelStage.getUniformBlock(HW::PUBLIC_UNIFORMS);
        psPrivateUniforms.add(Type::COLOR3, LIGHT_LOOP_RESULT, Value::createValue(Color3(0.0f, 0.0f, 0.0f)));
        psPrivateUniforms.add(Type::COLOR3, MAYA_ENV_IRRADIANCE_SAMPLE, Value::createValue(Color3(0.0f, 0.0f, 0.0f)));
        psPrivateUniforms.add(Type::COLOR3, MAYA_ENV_RADIANCE_SAMPLE, Value::createValue(Color3(0.0f, 0.0f, 0.0f)));
        psPrivateUniforms.add(Type::FLOAT, MAYA_ENV_ROUGHNESS,  Value::createValue(0.0f));
    }

    createStage(Stage::UNIFORMS, *shader);
    return shader;
}

ShaderPtr GlslFragmentGenerator::generate(const string& fragmentName, ElementPtr element, GenContext& context) const
{
    ShaderPtr shader = createShader(fragmentName, element, context);

    ShaderStage& pixelStage = shader->getStage(Stage::PIXEL);
    ShaderGraph& graph = shader->getGraph();

    // Turn on fixed float formatting to make sure float values are
    // emitted with a decimal point and not as integers, and to avoid
    // any scientific notation which isn't supported by all OpenGL targets.
    ScopedFloatFormatting floatFormatting(Value::FloatFormatFixed);

    const VariableBlock& vertexData = pixelStage.getInputBlock(HW::VERTEX_DATA);
    if (!vertexData.empty())
    {
        // GLSL shader input blocks are essentially global constants accessible
        // from any function in the shader, not just the entry point. The
        // MaterialX GLSL generator makes use of this global access pattern.
        // We could make it work for VP2 GLSL fragments by referencing the
        // PIX_IN input block generated by VP2 in the final effect but this
        // approach has two problems:
        // 1. VP2 pre-processes some inputs (e.g. Nw) before passing them as
        // arguments to the fragment's root function. These are the correct
        // values to use for shading, not the original ones from PIX_IN.
        // 2. This pattern is not portable to HLSL where shader stage inputs
        // are passed as arguments to the entry point function and have to be
        // passed to other functions explicitly.
        //
        // Since passing vertex inputs to all functions as arguments would be
        // too intrusive in GLSL codegen, we instead define a non-const global
        // struct variable which gets populated from the function arguments in
        // the fragment's root function and is read from in all other functions

        emitLine("struct", pixelStage, false);
        emitScopeBegin(pixelStage);
        emitVariableDeclarations(vertexData, EMPTY_STRING, Syntax::SEMICOLON, context, pixelStage, false);
        emitScopeEnd(pixelStage, false, false);
        emitString(" " + HW::T_VERTEX_DATA_INSTANCE, pixelStage);
        emitLineEnd(pixelStage, true);
        emitLineBreak(pixelStage);
    }

    // Add global constants and type definitions
    emitInclude("pbrlib/genglsl/lib/mx_defines.glsl", context, pixelStage);

    const unsigned int maxLights = std::max(1u, context.getOptions().hwMaxActiveLightSources);
    emitLine("#define MAX_LIGHT_SOURCES " + std::to_string(maxLights), pixelStage, false);
    emitLine("#define " + HW::ENV_RADIANCE_MAX_SAMPLES + " " + std::to_string(context.getOptions().hwMaxRadianceSamples), pixelStage, false);
    emitLineBreak(pixelStage);
    emitTypeDefinitions(context, pixelStage);

    // Add all constants
    const VariableBlock& constants = pixelStage.getConstantBlock();
    if (!constants.empty())
    {
        emitVariableDeclarations(constants, _syntax->getConstantQualifier(), Syntax::SEMICOLON, context, pixelStage);
        emitLineBreak(pixelStage);
    }

    const bool lighting = requiresLighting(graph);

    // Emit common math functions
    emitInclude("pbrlib/genglsl/lib/mx_math.glsl", context, pixelStage);
    emitLineBreak(pixelStage);

    if (lighting)
    {
        emitBlock(MX_ENVIRONMENT_MAYA, context, pixelStage);
    }

    // Set the include file to use for uv transformations,
    // depending on the vertical flip flag.
    _tokenSubstitutions[ShaderGenerator::T_FILE_TRANSFORM_UV] = string("stdlib/genglsl") +
        (context.getOptions().fileTextureVerticalFlip ? "/lib/mx_transform_uv_vflip.glsl": "/lib/mx_transform_uv.glsl");

    // Add all functions for node implementations
    emitFunctionDefinitions(graph, context, pixelStage);

    const ShaderGraphOutputSocket* outputSocket = graph.getOutputSocket();

    // Add the function signature for the fragment's root function

    // Keep track of arguments we changed from matrix3 to matrix4 as additional
    // code must be inserted to get back the matrix3 version
    StringVec convertMatrixStrings;

    string functionName = shader->getName();
    _syntax->makeIdentifier(functionName, graph.getIdentifierMap());
    setFunctionName(functionName, pixelStage);

    emitLine((context.getOptions().hwTransparency ? "vec4 " : "vec3 ") + functionName, pixelStage, false);

    // Emit public uniforms and vertex data as function arguments
    //
    emitScopeBegin(pixelStage, Syntax::PARENTHESES);
    {
        bool firstArgument = true;

        auto emitArgument = [this, &firstArgument, &pixelStage, &context](
            const ShaderPort* shaderPort
        ) -> void
        {
            if (firstArgument)
            {
                firstArgument = false;
            }
            else
            {
                emitString(Syntax::COMMA, pixelStage);
                emitLineEnd(pixelStage, false);
            }

            emitLineBegin(pixelStage);
            emitVariableDeclaration(shaderPort, EMPTY_STRING, context, pixelStage, false);
        };

        const VariableBlock& publicUniforms = pixelStage.getUniformBlock(HW::PUBLIC_UNIFORMS);
        for (size_t i = 0; i < publicUniforms.size(); ++i)
        {
            const ShaderPort* const shaderPort = publicUniforms[i];

            if (shaderPort->getType() == Type::MATRIX33)
                convertMatrixStrings.push_back(shaderPort->getVariable());

            emitArgument(shaderPort);
        }

        for (size_t i = 0; i < vertexData.size(); ++i)
            emitArgument(vertexData[i]);

        if (context.getOptions().hwTransparency)
        {
            // A dummy argument not used in the generated shader code but necessary to
            // map onto an OGS fragment parameter and a shading node DG attribute with
            // the same name that can be set to a non-0 value to let Maya know that the
            // surface is transparent.

            if (!firstArgument)
            {
                emitString(Syntax::COMMA, pixelStage);
                emitLineEnd(pixelStage, false);
            }

            emitLineBegin(pixelStage);
            emitString("float ", pixelStage);
            emitString(OgsXmlGenerator::VP_TRANSPARENCY_NAME, pixelStage);
            emitLineEnd(pixelStage, false);
        }
    }
    emitScopeEnd(pixelStage);

    // Add function body
    emitScopeBegin(pixelStage);

    if (graph.hasClassification(ShaderNode::Classification::CLOSURE))
    {
        // Handle the case where the graph is a direct closure.
        // We don't support rendering closures without attaching 
        // to a surface shader, so just output black.
        emitLine("return vec3(0.0)", pixelStage);
    }
    else
    {
        // Copy vertex data values from the root function's arguments to a
        // global struct variable accessible from all other functions.
        //
        for (size_t i = 0; i < vertexData.size(); ++i)
        {
            const string& name = vertexData[i]->getVariable();

            emitLineBegin(pixelStage);
            emitString(HW::T_VERTEX_DATA_INSTANCE, pixelStage);
            emitString(".", pixelStage);
            emitString(name, pixelStage);
            emitString(" = ", pixelStage);
            emitString(name, pixelStage);
            emitLineEnd(pixelStage, true);
        }

        if (lighting) {
            // Store environment samples from light rig:
            emitLine("g_" + MAYA_ENV_IRRADIANCE_SAMPLE + " = " + MAYA_ENV_IRRADIANCE_SAMPLE, pixelStage);
            emitLine("g_" + MAYA_ENV_RADIANCE_SAMPLE + " = " + MAYA_ENV_RADIANCE_SAMPLE, pixelStage);
        }

        // Insert matrix converters
        for (const string& argument : convertMatrixStrings)
        {
            emitLine("mat3 " + argument + " = mat3(" + argument + GlslFragmentGenerator::MATRIX3_TO_MATRIX4_POSTFIX + ")", pixelStage, true);
        }

        // Add all function calls
        emitFunctionCalls(graph, context, pixelStage);

        // Emit final result
        //
        // TODO: Emit a mayaSurfaceShaderOutput result to plug into mayaComputeSurfaceFinal
        //
        if (const ShaderOutput* const outputConnection = outputSocket->getConnection())
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
                    emitLine("return vec4(" + finalOutput + ".color, clamp(1.0 - dot(" + finalOutput + ".transparency, vec3(0.3333)), 0.0, 1.0))", pixelStage);
                }
                else
                {
                    emitLine("return " + finalOutput + ".color", pixelStage);
                }
            }
            else
            {
                if (context.getOptions().hwTransparency && !outputSocket->getType()->isFloat4())
                {
                    toVec4(outputSocket->getType(), finalOutput);
                }
                else if (!context.getOptions().hwTransparency && !outputSocket->getType()->isFloat3())
                {
                    toVec3(outputSocket->getType(), finalOutput);
                }
                emitLine("return " + finalOutput, pixelStage);
            }
        }
        else
        {
            const string outputValue = outputSocket->getValue()
                ? _syntax->getValue(outputSocket->getType(), *outputSocket->getValue())
                : _syntax->getDefaultValue(outputSocket->getType());

            if (!context.getOptions().hwTransparency && !outputSocket->getType()->isFloat3())
            {
                string finalOutput = outputSocket->getVariable() + "_tmp";
                emitLine(_syntax->getTypeName(outputSocket->getType()) + " " + finalOutput + " = " + outputValue, pixelStage);
                toVec3(outputSocket->getType(), finalOutput);
                emitLine("return " + finalOutput, pixelStage);
            }
            else if (context.getOptions().hwTransparency && !outputSocket->getType()->isFloat4())
            {
                string finalOutput = outputSocket->getVariable() + "_tmp";
                emitLine(_syntax->getTypeName(outputSocket->getType()) + " " + finalOutput + " = " + outputValue, pixelStage);
                toVec4(outputSocket->getType(), finalOutput);
                emitLine("return " + finalOutput, pixelStage);
            }
            else
            {
                emitLine("return " + outputValue, pixelStage);
            }
        }
    }

    // End function
    emitScopeEnd(pixelStage);

    // Replace all tokens with real identifier names
    replaceTokens(_tokenSubstitutions, pixelStage);

    // Now emit uniform definitions to a special stage which is only
    // consumed by the HLSL cross-compiler.
    //
    ShaderStage& uniformsStage = shader->getStage(Stage::UNIFORMS);

    auto emitUniformBlock = [this, &uniformsStage, &context](
        const VariableBlock& uniformBlock
    ) -> void
    {
        for (size_t i = 0; i < uniformBlock.size(); ++i)
        {
            const ShaderPort* const shaderPort = uniformBlock[i];

            const string& originalName = shaderPort->getVariable();
            const string textureName =
                OgsXmlGenerator::samplerToTextureName(originalName);
            if (!textureName.empty())
            {
                // GLSL for OpenGL (which MaterialX generates) uses combined
                // samplers where the texture is implicit in the sampler.
                // HLSL shader model 5.0 required by DX11 uses separate samplers
                // and textures.
                // We use a naming convention compatible with both OGS and
                // SPIRV-Cross where we derive sampler names from texture names
                // with a prefix and a suffix.
                // Unfortunately, the cross-compiler toolchain converts GLSL
                // samplers to HLSL textures preserving the original names and
                // generates HLSL samplers with derived names. For this reason,
                // we rename GLSL samplers with macros here to follow the
                // texture naming convention which preserves the correct
                // mapping.

                emitLineBegin(uniformsStage);
                emitString("#define ", uniformsStage);
                emitString(originalName, uniformsStage);
                emitString(" ", uniformsStage);
                emitString(textureName, uniformsStage);
                emitLineEnd(uniformsStage, false);
            }

            emitLineBegin(uniformsStage);
            emitVariableDeclaration(
                shaderPort, _syntax->getUniformQualifier(), context, uniformsStage
            );
            emitString(Syntax::SEMICOLON, uniformsStage);
            emitLineEnd(uniformsStage, false);
        }

        if (!uniformBlock.empty())
            emitLineBreak(uniformsStage);
    };

    emitUniformBlock(pixelStage.getUniformBlock(HW::PRIVATE_UNIFORMS));
    emitUniformBlock(pixelStage.getUniformBlock(HW::PUBLIC_UNIFORMS));

    replaceTokens(_tokenSubstitutions, uniformsStage);

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
    // We change matrix3 to matrix4 input arguments
    if (variable->getType() == Type::MATRIX33)
    {
        string qualifierPrefix = qualifier.empty() ? EMPTY_STRING : qualifier + " ";
        emitString(qualifierPrefix + "mat4 " + variable->getVariable() + MATRIX3_TO_MATRIX4_POSTFIX, stage);
    }
    else
    {
        GlslShaderGenerator::emitVariableDeclaration(variable, qualifier, context, stage, assignValue);
    }
}

} // namespace MaterialX
