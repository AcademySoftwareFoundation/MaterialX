#include <MaterialXShaderGen/ShaderGenerators/OgsFxShaderGenerator.h>
#include <MaterialXShaderGen/Implementations/AdskSurface.h>
#include <MaterialXShaderGen/Implementations/Surface.h>

#include <sstream>

namespace MaterialX
{

DEFINE_SHADER_GENERATOR(OgsFxShaderGenerator, "glsl", "ogsfx")

OgsFxShaderGenerator::OgsFxShaderGenerator()
    : GlslShaderGenerator()
{
    // Add target specific implementations

    // <!-- <adskSurface> -->
    registerImplementation("IM_adskSurface__glsl", AdskSurfaceOgsFx::creator);
    // <!-- <surface> -->
    registerImplementation("IM_surface__glsl", SurfaceOgsFx::creator);
}

ShaderPtr OgsFxShaderGenerator::generate(const string& shaderName, ElementPtr element)
{
    ShaderPtr shaderPtr = std::make_shared<Shader>(shaderName);
    shaderPtr->initialize(element, *this);

    Shader& shader = *shaderPtr;

    shader.addInclude("sx/impl/shadergen/source/glsl/defines.glsl", *this);
    shader.newLine();

    shader.addInclude("sx/impl/shadergen/source/glsl/ogsfx/default_uniforms.glsl", *this);
    shader.newLine();

    // Emit all shader uniforms
    for (const Shader::Uniform& uniform : shader.getUniforms())
    {
        emitUniform(
            uniform.first,
            uniform.second->type,
            uniform.second->value,
            shader
        );
    }

    shader.newLine();

    // Emit functions for lighting if needed
    if (!shader.hasClassification(SgNode::Classification::TEXTURE))
    {
        shader.addInclude("sx/impl/shadergen/source/glsl/ogsfx/lighting.glsl", *this);
        shader.newLine();
    }

    shader.addInclude("sx/impl/shadergen/source/glsl/ogsfx/vertexshader.glsl", *this);
    shader.newLine();

    shader.addComment("---------------------------------- Pixel shader ----------------------------------------\n");

    emitTypeDefs(shader);

    // Emit shader output
    SgNodeGraph* graph = shader.getNodeGraph();
    const SgOutputSocket* outputSocket = graph->getOutputSocket();
    const string variable = _syntax->getVariableName(outputSocket);
    shader.addComment("Data output by the pixel shader");
    shader.addLine("attribute PixelOutput", false);
    shader.beginScope(Shader::Brackets::BRACES);
    shader.addLine("vec4 " + variable);
    shader.endScope(true);
    shader.newLine();

    shader.addComment("Pixel shader");
    shader.addStr("GLSLShader PS\n");
    shader.beginScope(Shader::Brackets::BRACES);

    shader.addInclude("sx/impl/shadergen/source/glsl/math.glsl", *this);
    shader.newLine();

    shader.addComment("-------------------------------- Node Functions ------------------------------------\n");
    emitFunctions(shader);

    shader.addLine("void main()", false);
    shader.beginScope(Shader::Brackets::BRACES);
    emitShaderBody(shader);
    emitFinalOutput(shader);
    shader.endScope();

    shader.endScope();
    shader.newLine();

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

void OgsFxShaderGenerator::emitShaderBody(Shader &shader)
{
    // Shaders need special handling to get closures right
    if (shader.getNodeGraph()->hasClassification(SgNode::Classification::SHADER))
    {
        // Handle all texturing nodes. These are inputs to any
        // closure/shader nodes and need to be emitted first.
        //
        emitTextureNodes(shader);
        shader.newLine();

        // Emit function calls for all shader nodes
        for (SgNode* node : shader.getNodeGraph()->getNodes())
        {
            // Emit only unconditional nodes, since any node within a conditional 
            // branch is emitted by the conditional node itself
            if (node->hasClassification(SgNode::Classification::SHADER) && !node->referencedConditionally())
            {
                shader.addFunctionCall(node, *this);
            }
        }
    }
    else
    {
        // No shader, just call parent method
        GlslShaderGenerator::emitShaderBody(shader);
    }
}

void OgsFxShaderGenerator::emitFinalOutput(Shader& shader) const
{
    SgNodeGraph* graph = shader.getNodeGraph();
    const SgOutputSocket* outputSocket = graph->getOutputSocket();
    const string outputVariable = _syntax->getVariableName(outputSocket);

    if (!outputSocket->connection)
    {
        // Early out for the rare case where the whole graph is just a single value
        shader.addLine(outputVariable + " = " + (outputSocket->value ? _syntax->getValue(*outputSocket->value) : _syntax->getTypeDefault(outputSocket->type)));
        return;
    }

    string finalResult = _syntax->getVariableName(outputSocket->connection);

    if (shader.hasClassification(SgNode::Classification::SURFACE))
    {
        shader.addComment("TODO: How should we output transparency?");
        shader.addLine("float outAlpha = 1.0 - maxv(" + finalResult + ".transparency)");
        shader.addLine(outputVariable + " = vec4(" + finalResult + ".color, outAlpha)");
    }
    else
    {
        if (outputSocket->channels != EMPTY_STRING)
        {
            finalResult = _syntax->getSwizzledVariable(finalResult, outputSocket->type, outputSocket->connection->type, outputSocket->channels);
        }
        if (outputSocket->type != "color4" && outputSocket->type != "vector4")
        {
            // Remap to vec4 type for final output
            if (outputSocket->type == "float" || outputSocket->type == "boolean" || outputSocket->type == "integer")
            {
                finalResult = "vec4(" + finalResult + ", " + finalResult + ", " + finalResult + ", 1.0)";
            }
            else if (outputSocket->type == "vector2" || outputSocket->type == "color2")
            {
                finalResult = "vec4(" + finalResult + ", 0.0, 1.0)";
            }
            else if (outputSocket->type == "vector3" || outputSocket->type == "color3")
            {
                finalResult = "vec4(" + finalResult + ", 1.0)";
            }
            else
            {
                // Can't understand other types. Just output black
                finalResult = "vect4(0.0,0.0,0.0,1.0)";
            }
        }
        shader.addLine(outputVariable + " = " + finalResult);
    }
}

void OgsFxShaderGenerator::emitUniform(const string& name, const string& type, const ValuePtr& value, Shader& shader)
{
    // A file texture input needs special handling on GLSL
    if (type == kFilename)
    {
        std::stringstream str;
        str << "uniform texture2D " << name << "_texture : SourceTexture;\n";
        str << "uniform sampler2D " << name << " = sampler_state\n";
        str << "{\n    Texture = <" << name << "_texture>;\n};\n";
        shader.addBlock(str.str());
    }
    else
    {
        shader.beginLine();
        shader.addStr("uniform ");
        ShaderGenerator::emitUniform(name, type, value, shader);
        shader.endLine();
    }
}

bool OgsFxShaderGenerator::shouldPublish(const ValueElement* port, string& publicName) const
{
    if (!ShaderGenerator::shouldPublish(port, publicName))
    {
        // File texture inputs must be published in GLSL
        static const string kImage = "image";
        if (port->getParent()->getCategory() == kImage &&
            port->getType() == kFilename)
        {
            publicName = port->getParent()->getName() + "_" + port->getName();
            return true;
        }
        else
        {
            return false;
        }
    }
    return true;
}

} // namespace MaterialX
