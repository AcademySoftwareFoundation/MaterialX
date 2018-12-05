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
        { "type", "getLightType" },
        { "position", "getLightPos" },
        { "direction", "getLightDir" },
        { "color", "getLightColor" },
        { "intensity", "getLightIntensity" },
        { "decayRate", "getLightDecayRate" },
        { "innerConeAngle", "getLightConeAngle" },
        { "outerConeAngle", "getLightPenumbraAngle" }
    };
}

OgsFxShader::OgsFxShader(const string& name) 
    : ParentClass(name)
{
    _stages.push_back(Stage("FinalFx"));

    // Create default uniform blocks for final fx stage
    createUniformBlock(FINAL_FX_STAGE, PRIVATE_UNIFORMS, "prvUniform");
    createUniformBlock(FINAL_FX_STAGE, PUBLIC_UNIFORMS, "pubUniform");
}

void OgsFxShader::createUniform(size_t stage, const string& block, const TypeDesc* type, const string& name, const string& path, const string& semantic, ValuePtr value)
{
    // If no semantic is given check if we have 
    // an OgsFx semantic that should be used
    if (semantic.empty())
    {
        auto it = OGSFX_DEFAULT_SEMANTICS_MAP.find(name);
        if (it != OGSFX_DEFAULT_SEMANTICS_MAP.end())
        {
            HwShader::createUniform(stage, block, type, name, path, it->second, value);
            return;
        }
    }
    ParentClass::createUniform(stage, block, type, name, path, semantic, value);
}

void OgsFxShader::createAppData(const TypeDesc* type, const string& name, const string& semantic)
{
    // If no semantic is given check if we have 
    // an OgsFx semantic that should be used
    if (semantic.empty())
    {
        auto it = OGSFX_DEFAULT_SEMANTICS_MAP.find(name);
        if (it != OGSFX_DEFAULT_SEMANTICS_MAP.end())
        {
            HwShader::createAppData(type, name, it->second);
            return;
        }
    }
    ParentClass::createAppData(type, name, semantic);
}

void OgsFxShader::createVertexData(const TypeDesc* type, const string& name, const string& semantic)
{
    // If no semantic is given check if we have 
    // an OgsFx semantic that should be used
    if (semantic.empty())
    {
        auto it = OGSFX_DEFAULT_SEMANTICS_MAP.find(name);
        if (it != OGSFX_DEFAULT_SEMANTICS_MAP.end())
        {
            HwShader::createVertexData(type, name, it->second);
            return;
        }
    }
    ParentClass::createVertexData(type, name, semantic);
}


const string OgsFxShaderGenerator::TARGET = "ogsfx";

OgsFxShaderGenerator::OgsFxShaderGenerator()
    : ParentClass()
{
    _syntax = OgsFxSyntax::create();
}

ShaderPtr OgsFxShaderGenerator::generate(const string& shaderName, ElementPtr element, const GenOptions& options)
{
    OgsFxShaderPtr shaderPtr = createShader(shaderName);
    shaderPtr->initialize(element, *this, options);

    OgsFxShader& shader = *shaderPtr;

    bool lighting = shader.hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE) ||
        shader.hasClassification(ShaderNode::Classification::BSDF);

    // Turn on fixed formatting since OgsFx doesn't support scientific values
    Value::ScopedFloatFormatting fmt(Value::FloatFormatFixed);

    //
    // Emit code for vertex shader stage
    //

    shader.setActiveStage(OgsFxShader::VERTEX_STAGE);

    // Create required variables
    shader.createAppData(Type::VECTOR3, "i_position");
    shader.createUniform(HwShader::VERTEX_STAGE, HwShader::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldMatrix");
    shader.createUniform(HwShader::VERTEX_STAGE, HwShader::PRIVATE_UNIFORMS, Type::MATRIX44, "u_viewProjectionMatrix");

    shader.addComment("---------------------------------- Vertex shader ----------------------------------------\n");
    shader.addLine("GLSLShader VS", false);
    shader.beginScope(Shader::Brackets::BRACES);

    emitFunctionDefinitions(shader);

    // Add constants
    const Shader::VariableBlock& vsConstants = shader.getConstantBlock(HwShader::VERTEX_STAGE);
    if (!vsConstants.empty())
    {
        shader.addComment("Constant block: " + vsConstants.name);
        emitVariableBlock(vsConstants, _syntax->getConstantQualifier(), shader);
    }

    // Add main function
    shader.addLine("void main()", false);
    shader.beginScope(Shader::Brackets::BRACES);
    shader.addLine("vec4 hPositionWorld = u_worldMatrix * vec4(i_position, 1.0)");
    shader.addLine("gl_Position = u_viewProjectionMatrix * hPositionWorld");
    emitFunctionCalls(*_defaultContext, shader);
    shader.endScope();
    shader.endScope();
    shader.newLine();

    //
    // Emit code for pixel shader stage
    //

    shader.setActiveStage(OgsFxShader::PIXEL_STAGE);
    
    shader.addComment("---------------------------------- Pixel shader ----------------------------------------\n");
    shader.addStr("GLSLShader PS\n");
    shader.beginScope(Shader::Brackets::BRACES);

    if (lighting)
    {
        const Shader::VariableBlock& lightData = shader.getUniformBlock(HwShader::PIXEL_STAGE, HwShader::LIGHT_DATA_BLOCK);
        shader.addLine(lightData.name + " " + lightData.instance + "[MAX_LIGHT_SOURCES]");
        shader.newLine();
    }

    // Emit common math functions
    shader.addInclude("sxpbrlib/sx-glsl/lib/sx_math.glsl", *this);
    shader.newLine();

    // Emit sampling code if needed
    if (shader.hasClassification(ShaderNode::Classification::CONVOLUTION2D))
    {
        // Emit sampling functions
        shader.addInclude("stdlib/sx-glsl/lib/sx_sampling.glsl", *this);
        shader.newLine();
    }

    emitFunctionDefinitions(shader);

    // Add constants
    const Shader::VariableBlock& psConstants = shader.getConstantBlock(HwShader::PIXEL_STAGE);
    if (!psConstants.empty())
    {
        shader.addComment("Constant block: " + psConstants.name);
        emitVariableBlock(psConstants, _syntax->getConstantQualifier(), shader);
    }

    // Add main function
    shader.addLine("void main()", false);
    shader.beginScope(Shader::Brackets::BRACES);

    if (lighting)
    {
        // Add code for retreiving light data from OgsFx light uniforms
        shader.beginScope();
        shader.addLine("int numLights = numActiveLightSources()");
        shader.addLine("for (int i = 0; i<numLights; ++i)", false);
        shader.beginScope(Shader::Brackets::BRACES);
        const Shader::VariableBlock& lightData = shader.getUniformBlock(HwShader::PIXEL_STAGE, HwShader::LIGHT_DATA_BLOCK);
        for (const Shader::Variable* uniform : lightData.variableOrder)
        {
            auto it = OGSFX_GET_LIGHT_DATA_MAP.find(uniform->name);
            if (it != OGSFX_GET_LIGHT_DATA_MAP.end())
            {
                shader.addLine(lightData.instance + "[i]." + uniform->name + " = " + it->second + "(i)");
            }
        }
        shader.endScope();
        shader.endScope();
        shader.newLine();
    }

    emitFunctionCalls(*_defaultContext, shader);
    emitFinalOutput(shader);

    shader.endScope();
    shader.endScope();
    shader.newLine();

    //
    // Assemble the final effects shader
    //

    shader.setActiveStage(size_t(OgsFxShader::FINAL_FX_STAGE));

    // Add version directive
    shader.addLine("#version " + getVersion(), false);
    shader.newLine();

    // Add global constants and type definitions
    shader.addInclude("sxpbrlib/sx-glsl/lib/sx_defines.glsl", *this);
    shader.addLine("#define MAX_LIGHT_SOURCES " + std::to_string(getMaxActiveLightSources()), false);
    shader.newLine();
    emitTypeDefinitions(shader);

    shader.addComment("Data from application to vertex shader");
    shader.addLine("attribute AppData", false);
    shader.beginScope(Shader::Brackets::BRACES);
    const Shader::VariableBlock& appDataBlock = shader.getAppDataBlock();
    for (const Shader::Variable* input : appDataBlock.variableOrder)
    {
        const string& type = _syntax->getTypeName(input->type);
        shader.addLine(type + " " + input->name + (!input->semantic.empty() ? " : " + input->semantic : ""));
    }
    shader.endScope(true);
    shader.newLine();

    shader.addComment("Data passed from vertex shader to pixel shader");
    shader.addLine("attribute VertexData", false);
    shader.beginScope(Shader::Brackets::BRACES);
    const Shader::VariableBlock& vertexDataBlock = shader.getVertexDataBlock();
    for (const Shader::Variable* output : vertexDataBlock.variableOrder)
    {
        const string& type = _syntax->getTypeName(output->type);
        shader.addLine(type + " " + output->name + (!output->semantic.empty() ? " : " + output->semantic : ""));
    }
    shader.endScope(true);
    shader.newLine();

    // Add the pixel shader output. This needs to be a vec4 for rendering
    // and upstream connection will be converted to vec4 if needed in emitFinalOutput()
    shader.addComment("Data output by the pixel shader");
    const ShaderGraphOutputSocket* outputSocket = shader.getGraph()->getOutputSocket();
    shader.addLine("attribute PixelOutput", false);
    shader.beginScope(Shader::Brackets::BRACES);
    shader.addLine("vec4 " + outputSocket->variable);
    shader.endScope(true);
    shader.newLine();

    // Add all private vertex shader uniforms
    const Shader::VariableBlock& vsPrivateUniforms = shader.getUniformBlock(HwShader::VERTEX_STAGE, HwShader::PRIVATE_UNIFORMS);
    if (!vsPrivateUniforms.empty())
    {
        shader.addComment("Vertex stage uniform block: " + vsPrivateUniforms.name);
        for (const Shader::Variable* uniform : vsPrivateUniforms.variableOrder)
        {
            emitUniform(*uniform, shader);
        }
        shader.newLine();
    }

    // Add all public vertex shader uniforms
    const Shader::VariableBlock& vsPublicUniforms = shader.getUniformBlock(HwShader::VERTEX_STAGE, HwShader::PUBLIC_UNIFORMS);
    if (!vsPublicUniforms.empty())
    {
        shader.addComment("Vertex stage uniform block: " + vsPublicUniforms.name);
        for (const Shader::Variable* uniform : vsPublicUniforms.variableOrder)
        {
            emitUniform(*uniform, shader);
        }
        shader.newLine();
    }

    // Add all private pixel shader uniforms
    const Shader::VariableBlock& psPrivateUniforms = shader.getUniformBlock(HwShader::PIXEL_STAGE, HwShader::PRIVATE_UNIFORMS);
    if (!psPrivateUniforms.empty())
    {
        shader.addComment("Pixel stage uniform block: " + psPrivateUniforms.name);
        for (const Shader::Variable* uniform : psPrivateUniforms.variableOrder)
        {
            emitUniform(*uniform, shader);
        }
        shader.newLine();
    }

    // Add all public pixel shader uniforms
    const Shader::VariableBlock& psPublicUniforms = shader.getUniformBlock(HwShader::PIXEL_STAGE, HwShader::PUBLIC_UNIFORMS);
    if (!psPublicUniforms.empty())
    {
        shader.addComment("Pixel stage uniform block: " + psPublicUniforms.name);
        for (const Shader::Variable* uniform : psPublicUniforms.variableOrder)
        {
            emitUniform(*uniform, shader);
        }
        shader.newLine();
    }

    if (lighting)
    {
        // Add light struct declaration
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
    }

    if (lighting)
    {
        // Emit OGS lighting uniforms
        shader.addInclude("sxpbrlib/sx-glsl/ogsfx/lighting_uniforms.glsl", *this);
        shader.newLine();

        // Emit lighting functions
        shader.addLine("GLSLShader LightingFunctions", false);
        shader.beginScope(Shader::Brackets::BRACES);
        shader.addInclude("sxpbrlib/sx-glsl/ogsfx/lighting_functions.glsl", *this);
        shader.newLine();
        shader.addInclude("sxpbrlib/sx-glsl/lib/sx_lighting.glsl", *this);
        shader.endScope();
        shader.newLine();
    }

    // Add code blocks from the vertex and pixel shader stages generated above
    shader.addBlock(shader.getSourceCode(OgsFxShader::VERTEX_STAGE), *this);
    shader.addBlock(shader.getSourceCode(OgsFxShader::PIXEL_STAGE), *this);

    // Add Main technique block
    string techniqueParams;
    getTechniqueParams(shader, techniqueParams);
    if (techniqueParams.size())
    {
        shader.addLine("technique Main< " + techniqueParams + " >", false);
    }
    else
    {
        shader.addLine("technique Main", false);
    }
    shader.beginScope(Shader::Brackets::BRACES);
    shader.addLine("pass p0", false);
    shader.beginScope(Shader::Brackets::BRACES);
    shader.addLine("VertexShader(in AppData, out VertexData vd) = { VS }");
    shader.addLine(lighting ? 
        "PixelShader(in VertexData vd, out PixelOutput) = { LightingFunctions, PS }" :
        "PixelShader(in VertexData vd, out PixelOutput) = { PS }");
    shader.endScope();
    shader.endScope();
    shader.newLine();

    return shaderPtr;
}

void OgsFxShaderGenerator::emitVariable(const Shader::Variable& uniform, const string& qualifier, Shader& shader)
{
    // A file texture input needs special handling on GLSL
    if (uniform.type == Type::FILENAME)
    {
        std::stringstream str;
        str << "uniform texture2D " << uniform.name << "_texture : SourceTexture;\n";
        str << "uniform sampler2D " << uniform.name << " = sampler_state\n";
        str << "{\n    Texture = <" << uniform.name << "_texture>;\n};\n";
        shader.addBlock(str.str(), *this);
    }
    else if (!uniform.semantic.empty())
    {
        const string& type = _syntax->getTypeName(uniform.type);
        shader.addLine(qualifier + " " + type + " " + uniform.name + " : " + uniform.semantic);
    }
    else
    {
        const string& type = _syntax->getTypeName(uniform.type);
        const string initStr = (uniform.value ? _syntax->getValue(uniform.type, *uniform.value, true) : _syntax->getDefaultValue(uniform.type, true));

        string line = qualifier + " " + type + " " + uniform.name;

        // If an arrays we need an array qualifier (suffix) for the variable name
        string arraySuffix;
        uniform.getArraySuffix(arraySuffix);
        line += arraySuffix;

        line += initStr.empty() ? "" : " = " + initStr;

        shader.addLine(line);
    }
}

OgsFxShaderPtr OgsFxShaderGenerator::createShader(const string& name)
{
    return std::make_shared<OgsFxShader>(name);
}

void OgsFxShaderGenerator::getTechniqueParams(const Shader& /*shader*/, string& /*params*/)
{
    // Default implementation doesn't use any technique parameters
}

} // namespace MaterialX
