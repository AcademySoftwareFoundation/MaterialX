#include <MaterialXShaderGen/ShaderGenerators/OgsFxShaderGenerator.h>
#include <MaterialXShaderGen/Implementations/Position.h>
#include <MaterialXShaderGen/Implementations/Normal.h>
#include <MaterialXShaderGen/Implementations/Tangent.h>
#include <MaterialXShaderGen/Implementations/Bitangent.h>
#include <MaterialXShaderGen/Implementations/TexCoord.h>
#include <MaterialXShaderGen/Implementations/AdskSurface.h>
#include <MaterialXShaderGen/Implementations/Surface.h>
#include <MaterialXShaderGen/HwShader.h>

#include <sstream>

namespace MaterialX
{

namespace {
    void toVec4(const string& type, string& variable)
    {
        if (kScalars.count(type))
        {
            variable = "vec4(" + variable + ", " + variable + ", " + variable + ", 1.0)";
        }
        else if (kTuples.count(type))
        {
            variable = "vec4(" + variable + ", 0.0, 1.0)";
        }
        else if (kTriples.count(type))
        {
            variable = "vec4(" + variable + ", 1.0)";
        }
        else
        {
            // Can't understand other types. Just return black.
            variable = "vect4(0.0,0.0,0.0,1.0)";
        }
    }
}

DEFINE_SHADER_GENERATOR(OgsFxShaderGenerator, "glsl", "ogsfx")

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
    SgNodeGraph* graph = shader.getNodeGraph();

    // Register default shader inputs
    shader.registerUniform(Shader::Variable("mat4", "gWorldXf", "World"));
    shader.registerUniform(Shader::Variable("mat4", "gWvpXf", "WorldViewProjection"));
    shader.registerUniform(Shader::Variable("mat4", "gViewIXf", "ViewInverse"));
    shader.registerAttribute(Shader::Variable("vec3", "inPosition", "POSITION"));
    shader.registerVarying(Shader::Variable("vec3", "ObjectPosition", "POSITION"));
    shader.registerVarying(Shader::Variable("vec3", "WorldPosition", "POSITION"));
    shader.registerVarying(Shader::Variable("vec3", "WorldView"));

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
    shader.addInclude("sx/impl/shadergen/source/glsl/defines.glsl");
    shader.newLine();
    emitTypeDefs(shader);

    shader.addComment("Data from application to vertex buffer");
    shader.addLine("attribute AppData", false);
    shader.beginScope(Shader::Brackets::BRACES);
    for (const Shader::Variable& attr : shader.getAttributes())
    {
        shader.addLine(attr.type + " " + attr.name + (!attr.semantic.empty() ? " : " + attr.semantic : ""));
    }
    shader.endScope(true);
    shader.newLine();

    shader.addComment("Data passed from vertex shader to pixel shader");
    shader.addLine("attribute VertexOutput", false);
    shader.beginScope(Shader::Brackets::BRACES);
    for (const Shader::Variable& vary : shader.getVaryings())
    {
        shader.addLine(vary.type + " " + vary.name + (!vary.semantic.empty() ? " : " + vary.semantic : ""));
    }
    shader.endScope(true);
    shader.newLine();

    shader.addComment("Data output by the pixel shader");
    const SgOutputSocket* outputSocket = graph->getOutputSocket();
    const string variable = _syntax->getVariableName(outputSocket);
    shader.addLine("attribute PixelOutput", false);
    shader.beginScope(Shader::Brackets::BRACES);
    shader.addLine("vec4 " + variable);
    shader.endScope(true);
    shader.newLine();

    // Emit all shader uniforms
    for (const Shader::Variable& uniform : shader.getUniforms())
    {
        emitUniform(uniform, shader);
    }
    shader.newLine();

    // Emit common math functions
    shader.addInclude("sx/impl/shadergen/source/glsl/math.glsl");
    shader.newLine();

    // Emit functions for lighting if needed
    if (!shader.hasClassification(SgNode::Classification::TEXTURE))
    {
        shader.addInclude("sx/impl/shadergen/source/glsl/ogsfx/lighting.glsl");
        shader.newLine();
    }

    // Add code blocks from the vertex and pixel shader stages generated above
    shader.addBlock(shader.getSourceCode(OgsFxShader::VERTEX_STAGE));
    shader.addBlock(shader.getSourceCode(OgsFxShader::PIXEL_STAGE));

    if (shader.hasClassification(SgNode::Classification::TEXTURE))
    {
        shader.addInclude("sx/impl/shadergen/source/glsl/ogsfx/techniques_texturing.glsl");
    }
    else
    {
        shader.addInclude("sx/impl/shadergen/source/glsl/ogsfx/techniques_lighting.glsl");
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
        SgNodeGraph* graph = shader.getNodeGraph();
        const SgOutputSocket* outputSocket = graph->getOutputSocket();
        const string outputVariable = _syntax->getVariableName(outputSocket);

        // Early out for the rare case where the whole graph is just a single value
        if (!outputSocket->connection)
        {
            string outputValue = outputSocket->value ? _syntax->getValue(*outputSocket->value) : _syntax->getTypeDefault(outputSocket->type);
            if (!kQuadruples.count(outputSocket->type))
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
            if (!kQuadruples.count(outputSocket->type))
            {
                toVec4(outputSocket->type, finalOutput);
            }
            shader.addLine(outputVariable + " = " + finalOutput);
        }
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
