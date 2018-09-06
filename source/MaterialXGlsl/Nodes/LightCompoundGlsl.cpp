#include <MaterialXGlsl/Nodes/LightCompoundGlsl.h>
#include <MaterialXGlsl/GlslShaderGenerator.h>

#include <MaterialXShaderGen/Util.h>

namespace MaterialX
{

SgImplementationPtr LightCompoundGlsl::create()
{
    return std::make_shared<LightCompoundGlsl>();
}

const string& LightCompoundGlsl::getLanguage() const
{
    return GlslShaderGenerator::LANGUAGE;
}

const string& LightCompoundGlsl::getTarget() const
{
    return GlslShaderGenerator::TARGET;
}

void LightCompoundGlsl::initialize(ElementPtr implementation, ShaderGenerator& shadergen)
{
    SgImplementation::initialize(implementation, shadergen);

    NodeGraphPtr graph = implementation->asA<NodeGraph>();
    if (!graph)
    {
        throw ExceptionShaderGenError("Element '" + implementation->getName() + "' is not a node graph implementation");
    }

    _rootGraph = SgNodeGraph::create(graph, shadergen);
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

void LightCompoundGlsl::createVariables(const SgNode& /*node*/, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    // Create variables for all child nodes
    for (SgNode* childNode : _rootGraph->getNodes())
    {
        SgImplementation* impl = childNode->getImplementation();
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

void LightCompoundGlsl::emitFunctionDefinition(const SgNode& node, ShaderGenerator& shadergen_, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);
    GlslShaderGenerator shadergen = static_cast<GlslShaderGenerator&>(shadergen_);

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

    // Make the compound root graph the active graph
    shader.pushActiveGraph(_rootGraph.get());

    // Emit functions for all child nodes
    for (SgNode* childNode : _rootGraph->getNodes())
    {
        shader.addFunctionDefinition(childNode, shadergen);
    }

    // Emit function definitions for each context used by this compound node
    for (int id : node.getContextIDs())
    {
        const SgNodeContext* context = shadergen.getNodeContext(id);
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
        for (SgNode* childNode : shader.getNodeGraph()->getNodes())
        {
            if (childNode->hasClassification(SgNode::Classification::SHADER | SgNode::Classification::LIGHT))
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

void LightCompoundGlsl::emitFunctionCall(const SgNode& /*node*/, const SgNodeContext& /*context*/, ShaderGenerator& /*shadergen*/, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

        shader.addLine(_functionName + "(light, position, result)");

    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
