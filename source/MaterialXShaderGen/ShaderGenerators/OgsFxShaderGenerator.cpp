#include <MaterialXShaderGen/ShaderGenerators/OgsFxShaderGenerator.h>
#include <sstream>

namespace MaterialX
{

DEFINE_SHADER_GENERATOR(OgsFxShaderGenerator, "glsl", "ogsfx")

ShaderPtr OgsFxShaderGenerator::generate(const string& shaderName, ElementPtr element)
{
    ShaderPtr shaderPtr = std::make_shared<Shader>(shaderName);
    shaderPtr->initialize(element, getLanguage(), getTarget());

    Shader& shader = *shaderPtr;

    addExtraShaderUniforms(shader);

    shader.addInclude("sx/impl/shadergen/source/glsl/defines.glsl");
    shader.newLine();

    shader.addInclude("sx/impl/shadergen/source/glsl/ogsfx/default_uniforms.glsl");
    shader.newLine();

    // Emit all shader uniforms
    for (const Shader::Uniform& uniform : shader.getUniforms())
    {
        emitUniform(
            uniform.first, 
            uniform.second->getType(), 
            uniform.second->getValue(),
            shader
        );
    }
    // Emit all shader varyings
    for (const Shader::Varying& varying : shader.getVaryings())
    {
        emitUniform(
            varying.first,
            varying.second->getType(),
            varying.second->getValue(),
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
    const string variable = _syntax->getVariableName(*shader.getOutput());
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
    shader.addComment("Emit code for all texturing nodes");
    emitTextureNodes(shader);
    shader.newLine();

    // Emit function calls for all shader nodes
    for (const SgNode& node : shader.getNodes())
    {
        // Emit only unconditional nodes, since any node within a conditional 
        // branch is emitted by the conditional node itself
        if (node.hasClassification(SgNode::Classification::SHADER) && !node.referencedConditionally())
        {
            emitFunctionCall(node, shader);
        }
    }
    shader.newLine();

    emitFinalOutput(shader);
}

void OgsFxShaderGenerator::emitFinalOutput(Shader& shader) const
{
    const OutputPtr& output = shader.getOutput();
    const string outputVariable = _syntax->getVariableName(*output);

    const NodePtr connectedNode = output->getConnectedNode();
    string finalResult = _syntax->getVariableName(*connectedNode);

    if (shader.hasClassification(SgNode::Classification::SURFACE))
    {
        shader.addComment("TODO: How should we output transparency?");
        shader.addLine("float outAlpha = 1.0 - maxv(" + finalResult + ".transparency)");
        shader.addLine(outputVariable + " = vec4(" + finalResult + ".color, outAlpha)");
    }
    else
    {
        const string& outputType = output->getType();
        if (output->getChannels() != EMPTY_STRING)
        {
            finalResult = _syntax->getSwizzledVariable(finalResult, output->getType(), connectedNode->getType(), output->getChannels());
        }
        if (outputType != "color4" && outputType != "vector4")
        {
            // Remap to vec4 type for final output
            if (outputType == "float" || outputType == "boolean" || outputType == "integer")
            {
                finalResult = "vec4(" + finalResult + ", " + finalResult + ", " + finalResult + ", 1.0)";
            }
            else if (outputType == "vector2" || outputType == "color2")
            {
                finalResult = "vec4(" + finalResult + ", 0.0, 1.0)";
            }
            else if (outputType == "vector3" || outputType == "color3")
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

void OgsFxShaderGenerator::emitInput(const ValueElement& port, Shader& shader)
{
    if (port.isA<Parameter>() && 
        useAsShaderUniform(*port.asA<Parameter>()))
    {
        // This input is promoted to a shader input
        // So just use the name of that shader input
        shader.addStr(_syntax->getVariableName(port, true));
    }
    else
    {
        GlslShaderGenerator::emitInput(port, shader);
    }
}

void OgsFxShaderGenerator::addExtraShaderUniforms(Shader& shader)
{
    // Run over all node ports and check if any should be promoted to shader inputs
    // (this is the case for file texture filename inputs)
    for (const SgNode& node : shader.getNodes())
    {
        for (ParameterPtr param : node.getNode().getParameters())
        {
            if (useAsShaderUniform(*param))
            {
                const string name = _syntax->getVariableName(*param, true);
                shader.addUniform(Shader::Uniform(name, param));
            }
        }
    }
}

bool OgsFxShaderGenerator::useAsShaderUniform(const Parameter& param) const
{
    // Unconnected file texture inputs should be promoted to shader uniforms
    if (param.getType() == kFilename && param.getInterfaceName().empty() && param.getPublicName().empty())
    {
        ConstElementPtr parent = param.getParent();
        const string& nodeName = parent->isA<NodeDef>() ?
            parent->asA<NodeDef>()->getNode() : parent->asA<Node>()->getCategory();
        return nodeName == "image";
    }
    return false;
}

} // namespace MaterialX
