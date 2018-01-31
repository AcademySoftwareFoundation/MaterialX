#include <MaterialXShaderGen/ShaderGenerators/Glsl/OgsFx/OgsFxShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/OgsFx/PositionOgsFx.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/OgsFx/NormalOgsFx.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/OgsFx/TangentOgsFx.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/OgsFx/BitangentOgsFx.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/OgsFx/TexCoordOgsFx.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/OgsFx/AdskSurfaceOgsFx.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/OgsFx/SurfaceOgsFx.h>
#include <MaterialXShaderGen/HwShader.h>
#include <MaterialXShaderGen/Syntax.h>

#include <sstream>

namespace MaterialX
{

const string OgsFxShaderGenerator::TARGET = "ogsfx";

OgsFxShaderGenerator::OgsFxShaderGenerator()
    : GlslShaderGenerator()
{
    // Add target specific implementations

    // <!-- <position> -->
    registerImplementation("IM_position__vector3__glsl", PositionOgsFx::creator);
    // <!-- <normal> -->
    registerImplementation("IM_normal__vector3__glsl", NormalOgsFx::creator);
    // <!-- <tangent> -->
    registerImplementation("IM_tangent__vector3__glsl", TangentOgsFx::creator);
    // <!-- <bitangent> -->
    registerImplementation("IM_bitangent__vector3__glsl", BitangentOgsFx::creator);
    // <!-- <texcoord> -->
    registerImplementation("IM_texcoord__vector2__glsl", TexCoordOgsFx::creator);
    registerImplementation("IM_texcoord__vector3__glsl", TexCoordOgsFx::creator);
    // <!-- <adskSurface> -->
    registerImplementation("IM_adskSurface__glsl", AdskSurfaceOgsFx::creator);
    // <!-- <surface> -->
    registerImplementation("IM_surface__glsl", SurfaceOgsFx::creator);
}

ShaderPtr OgsFxShaderGenerator::generate(const string& shaderName, ElementPtr element)
{
    OgsFxShaderPtr shaderPtr = std::make_shared<OgsFxShader>(shaderName);
    shaderPtr->initialize(element, *this);

    OgsFxShader& shader = *shaderPtr;

    // Register required variables for vertex stage
    shader.registerUniform(Shader::Variable("mat4", "gWorldXf", "World"), HwShader::VERTEX_STAGE);
    shader.registerUniform(Shader::Variable("mat4", "gWvpXf", "WorldViewProjection"), HwShader::VERTEX_STAGE);
    shader.registerUniform(Shader::Variable("mat4", "gViewIXf", "ViewInverse"), HwShader::VERTEX_STAGE);
    shader.registerInput(Shader::Variable("vec3", "inPosition", "POSITION"), HwShader::VERTEX_STAGE);
    shader.registerOutput(Shader::Variable("vec3", "ObjectPosition", "POSITION"), HwShader::VERTEX_STAGE);
    shader.registerOutput(Shader::Variable("vec3", "WorldPosition", "POSITION"), HwShader::VERTEX_STAGE);
    shader.registerOutput(Shader::Variable("vec3", "WorldView"), HwShader::VERTEX_STAGE);

    // Register outputs from pixel stage
    const SgOutputSocket* outputSocket = shader.getNodeGraph()->getOutputSocket();
    const string outputName = _syntax->getVariableName(outputSocket);
    shader.registerOutput(Shader::Variable("vec4", outputName), HwShader::PIXEL_STAGE);

    //
    // Emit code for vertex shader stage
    //

    shader.setActiveStage(OgsFxShader::VERTEX_STAGE);

    shader.addComment("---------------------------------- Vertex shader ----------------------------------------\n");
    shader.addLine("GLSLShader VS", false);
    shader.beginScope(Shader::Brackets::BRACES);

    emitFunctionDefinitions(shader);

    shader.addLine("void main()", false);
    shader.beginScope(Shader::Brackets::BRACES);

    // Calculate default vertex data
    shader.addLine("vec4 Po = vec4(inPosition.xyz, 1)");
    shader.addLine("vec4 Pw = gWorldXf * Po");
    shader.addLine("VS_OUT.ObjectPosition = Po.xyz");
    shader.setCalculated("ObjectPosition");
    shader.addLine("VS_OUT.WorldPosition = Pw.xyz");
    shader.setCalculated("WorldPosition");
    shader.addLine("VS_OUT.WorldView = normalize(gViewIXf[3].xyz - Pw.xyz)");
    shader.setCalculated("WorldView");

    emitFunctionCalls(shader);
    emitFinalOutput(shader);

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

    emitFunctionDefinitions(shader);

    shader.addLine("void main()", false);
    shader.beginScope(Shader::Brackets::BRACES);

    emitFunctionCalls(shader);
    emitFinalOutput(shader);

    shader.endScope();
    shader.endScope();
    shader.newLine();

    //
    // Assemble the final effects shader
    //

    shader.setActiveStage(size_t(OgsFxShader::FINAL_FX_STAGE));

    // Add global constants and type definitions
    shader.addInclude("sx/impl/shadergen/source/glsl/defines.glsl", *this);
    shader.newLine();
    emitTypeDefs(shader);

    shader.addComment("Data from application to vertex buffer");
    shader.addLine("attribute AppData", false);
    shader.beginScope(Shader::Brackets::BRACES);
    for (const Shader::Variable& input : shader.getInputs(HwShader::VERTEX_STAGE))
    {
        shader.addLine(input.type + " " + input.name + (!input.semantic.empty() ? " : " + input.semantic : ""));
    }
    shader.endScope(true);
    shader.newLine();

    shader.addComment("Data passed from vertex shader to pixel shader");
    shader.addLine("attribute VertexOutput", false);
    shader.beginScope(Shader::Brackets::BRACES);
    for (const Shader::Variable& output : shader.getOutputs(HwShader::VERTEX_STAGE))
    {
        shader.addLine(output.type + " " + output.name + (!output.semantic.empty() ? " : " + output.semantic : ""));
    }
    shader.endScope(true);
    shader.newLine();

    shader.addComment("Data output by the pixel shader");
    shader.addLine("attribute PixelOutput", false);
    shader.beginScope(Shader::Brackets::BRACES);
    for (const Shader::Variable& output : shader.getOutputs(HwShader::PIXEL_STAGE))
    {
        shader.addLine(output.type + " " + output.name + (!output.semantic.empty() ? " : " + output.semantic : ""));
    }
    shader.endScope(true);
    shader.newLine();

    // Emit all shader uniforms
    for (const Shader::Variable& uniform : shader.getUniforms(HwShader::VERTEX_STAGE))
    {
        emitUniform(uniform, shader);
    }
    for (const Shader::Variable& uniform : shader.getUniforms(HwShader::PIXEL_STAGE))
    {
        emitUniform(uniform, shader);
    }
    shader.newLine();

    // Emit common math functions
    shader.addInclude("sx/impl/shadergen/source/glsl/math.glsl", *this);
    shader.newLine();

    // Emit functions for lighting if needed
    if (!shader.hasClassification(SgNode::Classification::TEXTURE))
    {
        shader.addInclude("sx/impl/shadergen/source/glsl/ogsfx/lighting.glsl", *this);
        shader.newLine();
    }

    // Add code blocks from the vertex and pixel shader stages generated above
    shader.addBlock(shader.getSourceCode(OgsFxShader::VERTEX_STAGE));
    shader.addBlock(shader.getSourceCode(OgsFxShader::PIXEL_STAGE));

    if (shader.hasClassification(SgNode::Classification::TEXTURE))
    {
        shader.addInclude("sx/impl/shadergen/source/glsl/ogsfx/techniques_texturing.glsl", *this);
    }
    else
    {
        shader.addInclude("sx/impl/shadergen/source/glsl/ogsfx/techniques_lighting.glsl", *this);
    }
    shader.newLine();

    return shaderPtr;
}

void OgsFxShaderGenerator::emitFinalOutput(Shader& shader) const
{
    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)
        shader.addLine("gl_Position = gWvpXf * Po");
    END_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        const vector<Shader::Variable>& outputs = shader.getOutputs(HwShader::PIXEL_STAGE);
        if (outputs.empty())
        {
            throw ExceptionShaderGenError("Shader '" + shader.getName() + "' has no registered output");
        }

        const string& outputVariable = outputs[0].name;
        const SgOutputSocket* outputSocket = shader.getNodeGraph()->getOutputSocket();

        // Early out for the rare case where the whole graph is just a single value
        if (!outputSocket->connection)
        {
            string outputValue = outputSocket->value ? _syntax->getValue(*outputSocket->value) : _syntax->getTypeDefault(outputSocket->type);
            if (!DataType::isQuadruple(outputSocket->type))
            {
                string finalOutput = outputVariable + "_tmp";
                shader.addLine(_syntax->getTypeName(outputSocket->type) + " " + finalOutput + " = " + outputValue);
                toVec4(outputSocket->type, finalOutput);
                shader.addLine(outputVariable + " = " + finalOutput);
            }
            else
            {
                shader.addLine(outputVariable + " = " + outputValue);
            }
            return;
        }

        string finalOutput = _syntax->getVariableName(outputSocket->connection);

        if (shader.hasClassification(SgNode::Classification::SURFACE))
        {
            shader.addComment("TODO: How should we output transparency?");
            shader.addLine("float outAlpha = 1.0 - maxv(" + finalOutput + ".transparency)");
            shader.addLine(outputVariable + " = vec4(" + finalOutput + ".color, outAlpha)");
        }
        else
        {
            if (outputSocket->channels != EMPTY_STRING)
            {
                finalOutput = _syntax->getSwizzledVariable(finalOutput, outputSocket->type, outputSocket->connection->type, outputSocket->channels);
            }
            if (!DataType::isQuadruple(outputSocket->type))
            {
                toVec4(outputSocket->type, finalOutput);
            }
            shader.addLine(outputVariable + " = " + finalOutput);
        }
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

void OgsFxShaderGenerator::emitUniform(const Shader::Variable& uniform, Shader& shader)
{
    // A file texture input needs special handling on GLSL
    if (uniform.type == DataType::FILENAME)
    {
        std::stringstream str;
        str << "uniform texture2D " << uniform.name << "_texture : SourceTexture;\n";
        str << "uniform sampler2D " << uniform.name << " = sampler_state\n";
        str << "{\n    Texture = <" << uniform.name << "_texture>;\n};\n";
        shader.addBlock(str.str());
    }
    else if (!uniform.semantic.empty())
    {
        shader.addLine("uniform " + _syntax->getTypeName(uniform.type) + " " + uniform.name + " : " + uniform.semantic);
    }
    else
    {
        const string initStr = (uniform.value ? _syntax->getValue(*uniform.value, true) : _syntax->getTypeDefault(uniform.type, true));
        shader.addLine("uniform " + _syntax->getTypeName(uniform.type) + " " + uniform.name + (initStr.empty() ? "" : " = " + initStr));
    }
}

const string OgsFxImplementation::SPACE = "space";
const string OgsFxImplementation::WORLD = "world";
const string OgsFxImplementation::OBJECT = "object";
const string OgsFxImplementation::MODEL = "model";
const string OgsFxImplementation::INDEX = "index";

const string& OgsFxImplementation::getLanguage() const
{
    return OgsFxShaderGenerator::LANGUAGE;
}

const string& OgsFxImplementation::getTarget() const
{
    return OgsFxShaderGenerator::TARGET;
}

} // namespace MaterialX
