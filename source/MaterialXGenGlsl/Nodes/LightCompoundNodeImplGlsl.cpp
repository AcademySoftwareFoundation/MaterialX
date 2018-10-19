#include <MaterialXGenGlsl/Nodes/LightCompoundNodeImplGlsl.h>
#include <MaterialXGenGlsl/GlslShaderGenerator.h>

#include <MaterialXGenShader/Util.h>

namespace MaterialX
{

ShaderNodeImplPtr LightCompoundNodeImplGlsl::create()
{
    return std::make_shared<LightCompoundNodeImplGlsl>();
}

const string& LightCompoundNodeImplGlsl::getLanguage() const
{
    return GlslShaderGenerator::LANGUAGE;
}

const string& LightCompoundNodeImplGlsl::getTarget() const
{
    return GlslShaderGenerator::TARGET;
}

void LightCompoundNodeImplGlsl::initialize(ElementPtr implementation, ShaderGenerator& shadergen)
{
    ShaderNodeImpl::initialize(implementation, shadergen);

    NodeGraphPtr graph = implementation->asA<NodeGraph>();
    if (!graph)
    {
        throw ExceptionShaderGenError("Element '" + implementation->getName() + "' is not a node graph implementation");
    }

    _rootGraph = ShaderGraph::create(graph, shadergen);
    _functionName = graph->getName();

    // Store light uniforms for all inputs and parameters on the interface
    NodeDefPtr nodeDef = graph->getNodeDef();
    _lightUniforms.resize(nodeDef->getInputCount() + nodeDef->getParameterCount());
    size_t index = 0;
    for (InputPtr input : nodeDef->getInputs())
    {
        _lightUniforms[index++] = Shader::Variable(TypeDesc::get(input->getType()), input->getName());
    }
    for (ParameterPtr param : nodeDef->getParameters())
    {
        _lightUniforms[index++] = Shader::Variable(TypeDesc::get(param->getType()), param->getName());
    }
}

void LightCompoundNodeImplGlsl::createVariables(const ShaderNode& /*node*/, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    // Create variables for all child nodes
    for (ShaderNode* childNode : _rootGraph->getNodes())
    {
        ShaderNodeImpl* impl = childNode->getImplementation();
        impl->createVariables(*childNode, shadergen, shader);
    }

    // Create all light data uniforms
    for (const Shader::Variable& uniform : _lightUniforms)
    {
        shader.createUniform(HwShader::PIXEL_STAGE, HwShader::LIGHT_DATA_BLOCK, uniform.type, uniform.name);
    }

    // Create uniform for number of active light sources
    shader.createUniform(HwShader::PIXEL_STAGE, HwShader::PRIVATE_UNIFORMS, Type::INTEGER, "u_numActiveLightSources",
        EMPTY_STRING, Value::createValue<int>(0));
}

void LightCompoundNodeImplGlsl::emitFunctionDefinition(const ShaderNode& node, ShaderGenerator& shadergen_, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);
    GlslShaderGenerator shadergen = static_cast<GlslShaderGenerator&>(shadergen_);

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

    // Make the compound root graph the active graph
    shader.pushActiveGraph(_rootGraph.get());

    // Emit functions for all child nodes
    for (ShaderNode* childNode : _rootGraph->getNodes())
    {
        shader.addFunctionDefinition(childNode, shadergen);
    }

    // Emit function definitions for each context used by this compound node
    for (int id : node.getContextIDs())
    {
        const GenContext* context = shadergen.getNodeContext(id);
        if (!context)
        {
            throw ExceptionShaderGenError("Node '" + node.getName() + "' has an implementation context that is undefined for shader generator '" +
                shadergen.getLanguage() + "/" + shadergen.getTarget() + "'");
        }

        // Emit function signature
        shader.addLine("void " + _functionName + context->getFunctionSuffix() + "(LightData light, vec3 position, out lightshader result)", false);
        shader.beginScope();

        // Handle all texturing nodes. These are inputs to any
        // closure/shader nodes and need to be emitted first.
        shadergen.emitTextureNodes(shader);

        // Emit function calls for all light shader nodes
        for (ShaderNode* childNode : shader.getGraph()->getNodes())
        {
            if (childNode->hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::LIGHT))
            {
                shader.addFunctionCall(childNode, *context, shadergen);
            }
        }

        shader.endScope();
        shader.newLine();
    }

    // Restore active graph
    shader.popActiveGraph();

    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

void LightCompoundNodeImplGlsl::emitFunctionCall(const ShaderNode& /*node*/, GenContext& /*context*/, ShaderGenerator& /*shadergen*/, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

        shader.addLine(_functionName + "(light, position, result)");

    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
