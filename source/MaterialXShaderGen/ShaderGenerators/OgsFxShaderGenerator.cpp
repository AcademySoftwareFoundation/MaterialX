#include <MaterialXShaderGen/ShaderGenerators/OgsFxShaderGenerator.h>
#include <MaterialXShaderGen/Implementations/Surface.h>

#include <sstream>

namespace MaterialX
{

DEFINE_SHADER_GENERATOR(OgsFxShaderGenerator, "glsl", "ogsfx")

OgsFxShaderGenerator::OgsFxShaderGenerator()
    : GlslShaderGenerator()
{
    // Add target specific implementations

    // <!-- <surface> -->
    registerImplementation("IM_surface__glsl", SurfaceOgsFx::creator);
}

ShaderPtr OgsFxShaderGenerator::generate(const string& shaderName, ElementPtr element)
{
    ShaderPtr shaderPtr = std::make_shared<Shader>(shaderName);
    shaderPtr->initialize(element, *this);

    Shader& shader = *shaderPtr;

    addExtraShaderUniforms(shader);

    shader.addInclude("sx/impl/shadergen/source/glsl/defines.glsl");
    shader.newLine();

    shader.addInclude("sx/impl/shadergen/source/glsl/ogsfx/default_uniforms.glsl");
    shader.newLine();

    // Emit all shader uniforms
    for (const auto& uniform : shader.getUniforms())
    {
        emitUniform(
            uniform.first,
            uniform.second->type, 
            uniform.second->value,
            shader
        );
    }
    // Emit all shader varyings
    for (const auto& varying : shader.getVaryings())
    {
        emitUniform(
            varying.first,
            varying.second->type,
            varying.second->value,
            shader
        );
    }
    shader.newLine();

    // Emit functions for lighting if needed
    if (!shader.hasClassification(SgNode::Classification::TEXTURE))
    {
        shader.addInclude("sx/impl/shadergen/source/glsl/ogsfx/lighting.glsl");
        shader.newLine();
    }

    shader.addInclude("sx/impl/shadergen/source/glsl/ogsfx/vertexshader.glsl");
    shader.newLine();

    shader.addComment("---------------------------------- Pixel shader ----------------------------------------\n");

    emitTypeDefs(shader);

    // Emit shader output
    SgNodeGraph* graph = shader.getNodeGraph();
    const SgOutput* output = graph->getOutput();
    const string variable = _syntax->getVariableName(output);
    shader.addComment("Data output by the pixel shader");
    shader.addLine("attribute PixelOutput", false);
    shader.beginScope(Shader::Brackets::BRACES);
    shader.addLine("vec4 " + variable);
    shader.endScope(true);
    shader.newLine();

    shader.addComment("Pixel shader");
    shader.addStr("GLSLShader PS\n");
    shader.beginScope(Shader::Brackets::BRACES);

    shader.addInclude("sx/impl/shadergen/source/glsl/math.glsl");
    shader.newLine();

    shader.addComment("-------------------------------- Node Functions ------------------------------------\n");
    emitFunctions(shader);

    shader.addLine("void main()", false);
    shader.beginScope(Shader::Brackets::BRACES);
    emitShaderBody(shader);
    shader.endScope();

    shader.endScope();
    shader.newLine();

    if (shader.hasClassification(SgNode::Classification::TEXTURE))
    {
        shader.addInclude("sx/impl/shadergen/source/glsl/ogsfx/techniques_texturing.glsl");
    }
    else
    {
        shader.addInclude("sx/impl/shadergen/source/glsl/ogsfx/techniques_lighting.glsl");
    }
    shader.newLine();

    // Release resources used by shader gen
    shaderPtr->finalize();

    return shaderPtr;
}

void OgsFxShaderGenerator::emitShaderBody(Shader &shader)
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
            node->getImplementation()->emitFunctionCall(*node, *this, shader);
        }
    }

    emitFinalOutput(shader);
}

void OgsFxShaderGenerator::emitFinalOutput(Shader& shader) const
{
    SgNodeGraph* graph = shader.getNodeGraph();
    const SgOutput* output = graph->getOutput();
    const string outputVariable = _syntax->getVariableName(output);

    SgInput* outputSocket = graph->getOutputSocket(output->name);
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
            finalResult = _syntax->getSwizzledVariable(finalResult, output->type, outputSocket->connection->type, outputSocket->channels);
        }
        if (output->type != "color4" && output->type != "vector4")
        {
            // Remap to vec4 type for final output
            if (output->type == "float" || output->type == "boolean" || output->type == "integer")
            {
                finalResult = "vec4(" + finalResult + ", " + finalResult + ", " + finalResult + ", 1.0)";
            }
            else if (output->type == "vector2" || output->type == "color2")
            {
                finalResult = "vec4(" + finalResult + ", 0.0, 1.0)";
            }
            else if (output->type == "vector3" || output->type == "color3")
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

void OgsFxShaderGenerator::emitInput(const SgInput* input, Shader& shader)
{
    if (useAsShaderUniform(input))
    {
        // This input is promoted to a shader input
        // So just use the name of that shader input
        shader.addStr(_syntax->getVariableName(input));
    }
    else
    {
        GlslShaderGenerator::emitInput(input, shader);
    }
}

void OgsFxShaderGenerator::addExtraShaderUniforms(Shader& shader)
{
    // Run over all node ports and check if any should be promoted to shader inputs
    // (this is the case for file texture filename inputs)
    for (SgNode* node : shader.getNodeGraph()->getNodes())
    {
        for (SgInput* input : node->getInputs())
        {
            if (useAsShaderUniform(input))
            {
                const string name = _syntax->getVariableName(input);
                shader.addUniform(name, input);
            }
        }
    }
}

bool OgsFxShaderGenerator::useAsShaderUniform(const SgInput* input) const
{
    // Unconnected file texture inputs should be promoted to shader uniforms
    if (!input->connection && input->node->hasClassification(SgNode::Classification::FILETEXTURE) && input->type == kFilename)
    {
        return true;
    }
    return false;
}

} // namespace MaterialX
