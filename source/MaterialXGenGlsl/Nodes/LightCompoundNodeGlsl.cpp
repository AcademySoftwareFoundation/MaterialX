#include <MaterialXGenGlsl/Nodes/LightCompoundNodeGlsl.h>
#include <MaterialXGenGlsl/GlslShaderGenerator.h>

#include <MaterialXGenShader/Util.h>

namespace MaterialX
{

LightCompoundNodeGlsl::LightCompoundNodeGlsl()
    : _lightUniforms(HW::LIGHT_DATA, EMPTY_STRING)
{
}

ShaderNodeImplPtr LightCompoundNodeGlsl::create()
{
    return std::make_shared<LightCompoundNodeGlsl>();
}

const string& LightCompoundNodeGlsl::getLanguage() const
{
    return GlslShaderGenerator::LANGUAGE;
}

const string& LightCompoundNodeGlsl::getTarget() const
{
    return GlslShaderGenerator::TARGET;
}

void LightCompoundNodeGlsl::initialize(ElementPtr implementation, const ShaderGenerator& shadergen, GenContext& context)
{
    ShaderNodeImpl::initialize(implementation, shadergen, context);

    NodeGraphPtr graph = implementation->asA<NodeGraph>();
    if (!graph)
    {
        throw ExceptionShaderGenError("Element '" + implementation->getName() + "' is not a node graph implementation");
    }

    _functionName = graph->getName();

    // For compounds we do not want to publish all internal inputs
    // so always use the reduced interface for this graph.
    const int shaderInterfaceType = context.getOptions().shaderInterfaceType;
    context.getOptions().shaderInterfaceType = SHADER_INTERFACE_REDUCED;
    _rootGraph = ShaderGraph::create(nullptr, graph, shadergen, context);
    context.getOptions().shaderInterfaceType = shaderInterfaceType;

    // Store light uniforms for all inputs and parameters on the interface
    NodeDefPtr nodeDef = graph->getNodeDef();
    for (InputPtr input : nodeDef->getInputs())
    {
        _lightUniforms.add(TypeDesc::get(input->getType()), input->getName());
    }
    for (ParameterPtr param : nodeDef->getParameters())
    {
        _lightUniforms.add(TypeDesc::get(param->getType()), param->getName());
    }
}

void LightCompoundNodeGlsl::createVariables(Shader& shader, const ShaderNode&, const ShaderGenerator& shadergen, GenContext& context) const
{
    // Create variables for all child nodes
    for (ShaderNode* childNode : _rootGraph->getNodes())
    {
        childNode->getImplementation().createVariables(shader, *childNode, shadergen, context);
    }

    ShaderStage& ps = shader.getStage(HW::PIXEL_STAGE);
    VariableBlock& lightData = ps.getUniformBlock(HW::LIGHT_DATA);

    // Create all light uniforms
    for (size_t i = 0; i<_lightUniforms.size(); ++i)
    {
        ShaderPort* u = const_cast<ShaderPort*>(_lightUniforms[i]);
        lightData.add(u->getSelf());
    }

    // Create uniform for number of active light sources
    addStageUniform(ps, HW::PRIVATE_UNIFORMS, Type::INTEGER, "u_numActiveLightSources", 
                    Value::createValue<int>(0));
}

void LightCompoundNodeGlsl::emitFunctionDefinition(ShaderStage& stage, const ShaderNode& node, const ShaderGenerator& shadergen_, GenContext& context) const
{
BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)

    const GlslShaderGenerator& shadergen = static_cast<const GlslShaderGenerator&>(shadergen_);

    // Emit functions for all child nodes
    shadergen.emitFunctionDefinitions(stage, *_rootGraph, context);

    // Find any closure contexts used by this node
    // and emit the function for each context.
    vector<HwClosureContextPtr> ccxs;
    shadergen.getNodeClosureContexts(node, ccxs);
    if (ccxs.empty())
    {
        emitFunctionDefinition(stage, shadergen, context, nullptr);
    }
    else
    {
        for (HwClosureContextPtr ccx : ccxs)
        {
            emitFunctionDefinition(stage, shadergen, context, ccx);
        }
    }

END_SHADER_STAGE(shader, HW::PIXEL_STAGE)
}

void LightCompoundNodeGlsl::emitFunctionDefinition(ShaderStage& stage, const GlslShaderGenerator& shadergen,
                                                   GenContext& context, HwClosureContextPtr ccx) const
{
    // Emit function signature
    if (ccx)
    {
        shadergen.emitLine(stage, "void " + _functionName + ccx->getSuffix() + "(LightData light, vec3 position, out lightshader result)", false);
    }
    else
    {
        shadergen.emitLine(stage, "void " + _functionName + "(LightData light, vec3 position, out lightshader result)", false);
    }

    shadergen.emitScopeBegin(stage);

    // Handle all texturing nodes. These are inputs to any
    // closure/shader nodes and need to be emitted first.
    shadergen.emitTextureNodes(stage, *_rootGraph, context);

    if (ccx)
    {
        context.pushUserData(HW::USER_DATA_CLOSURE_CONTEXT, ccx);
    }

    // Emit function calls for all light shader nodes
    for (const ShaderNode* childNode : _rootGraph->getNodes())
    {
        if (childNode->hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::LIGHT))
        {
            shadergen.emitFunctionCall(stage, *childNode, context, false);
        }
    }

    if (ccx)
    {
        context.popUserData(HW::USER_DATA_CLOSURE_CONTEXT);
    }

    shadergen.emitScopeEnd(stage);
    shadergen.emitLineBreak(stage);
}


void LightCompoundNodeGlsl::emitFunctionCall(ShaderStage& stage, const ShaderNode&, const ShaderGenerator& shadergen, GenContext&) const
{
BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
    shadergen.emitLine(stage, _functionName + "(light, position, result)");
END_SHADER_STAGE(shader, HW::PIXEL_STAGE)
}

} // namespace MaterialX
