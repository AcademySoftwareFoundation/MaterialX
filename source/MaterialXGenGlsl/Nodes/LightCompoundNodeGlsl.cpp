//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/Nodes/LightCompoundNodeGlsl.h>
#include <MaterialXGenGlsl/GlslShaderGenerator.h>

#include <MaterialXGenShader/Util.h>

namespace MaterialX
{

LightCompoundNodeGlsl::LightCompoundNodeGlsl() :
    _lightUniforms(HW::LIGHT_DATA, EMPTY_STRING)
{
}

ShaderNodeImplPtr LightCompoundNodeGlsl::create()
{
    return std::make_shared<LightCompoundNodeGlsl>();
}

const string& LightCompoundNodeGlsl::getTarget() const
{
    return GlslShaderGenerator::TARGET;
}

void LightCompoundNodeGlsl::initialize(const InterfaceElement& element, GenContext& context)
{
    CompoundNode::initialize(element, context);

    // Store light uniforms for all inputs on the interface
    const NodeGraph& graph = static_cast<const NodeGraph&>(element);
    NodeDefPtr nodeDef = graph.getNodeDef();
    for (InputPtr input : nodeDef->getActiveInputs())
    {
        _lightUniforms.add(TypeDesc::get(input->getType()), input->getName());
    }
}

void LightCompoundNodeGlsl::createVariables(const ShaderNode&, GenContext& context, Shader& shader) const
{
    // Create variables for all child nodes
    for (ShaderNode* childNode : _rootGraph->getNodes())
    {
        childNode->getImplementation().createVariables(*childNode, context, shader);
    }

    ShaderStage& ps = shader.getStage(Stage::PIXEL);
    VariableBlock& lightData = ps.getUniformBlock(HW::LIGHT_DATA);

    // Create all light uniforms
    for (size_t i = 0; i<_lightUniforms.size(); ++i)
    {
        ShaderPort* u = const_cast<ShaderPort*>(_lightUniforms[i]);
        lightData.add(u->getSelf());
    }

    const GlslShaderGenerator& shadergen = static_cast<const GlslShaderGenerator&>(context.getShaderGenerator());
    shadergen.addStageLightingUniforms(context, ps);
}

void LightCompoundNodeGlsl::emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        const GlslShaderGenerator& shadergen = static_cast<const GlslShaderGenerator&>(context.getShaderGenerator());

        // Emit functions for all child nodes
        shadergen.emitFunctionDefinitions(*_rootGraph, context, stage);

        // Find any closure contexts used by this node
        // and emit the function for each context.
        vector<ClosureContext*> ccts;
        shadergen.getNodeClosureContexts(node, ccts);
        if (ccts.empty())
        {
            emitFunctionDefinition(nullptr, context, stage);
        }
        else
        {
            for (ClosureContext* cct : ccts)
            {
                emitFunctionDefinition(cct, context, stage);
            }
        }
    END_SHADER_STAGE(shader, Stage::PIXEL)
}

void LightCompoundNodeGlsl::emitFunctionDefinition(ClosureContext* cct, GenContext& context, ShaderStage& stage) const
{
    const GlslShaderGenerator& shadergen = static_cast<const GlslShaderGenerator&>(context.getShaderGenerator());

    // Emit function signature
    if (cct)
    {
        // Use the first output for classifying node type for the closure context.
        // This is only relevent for closures, and they only have a single output.
        const TypeDesc* nodeType = _rootGraph->getOutputSocket()->getType();
        shadergen.emitLine("void " + _functionName + cct->getSuffix(nodeType) + "(LightData light, vec3 position, out lightshader result)", stage, false);
    }
    else
    {
        shadergen.emitLine("void " + _functionName + "(LightData light, vec3 position, out lightshader result)", stage, false);
    }

    shadergen.emitScopeBegin(stage);

    // Handle all texturing nodes. These are inputs to any
    // closure/shader nodes and need to be emitted first.
    shadergen.emitTextureNodes(*_rootGraph, context, stage);

    if (cct)
    {
        context.pushClosureContext(cct);
    }

    // Emit function calls for all light shader nodes
    for (const ShaderNode* childNode : _rootGraph->getNodes())
    {
        if (childNode->hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::LIGHT))
        {
            shadergen.emitFunctionCall(*childNode, context, stage, false);
        }
    }

    if (cct)
    {
        context.popClosureContext();
    }

    shadergen.emitScopeEnd(stage);
    shadergen.emitLineBreak(stage);
}


void LightCompoundNodeGlsl::emitFunctionCall(const ShaderNode&, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        shadergen.emitLine(_functionName + "(light, position, result)", stage);
    END_SHADER_STAGE(shader, Stage::PIXEL)
}

} // namespace MaterialX
