//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenHw/Nodes/HwLightCompoundNode.h>

#include <MaterialXGenHw/HwConstants.h>
#include <MaterialXGenHw/HwShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

HwLightCompoundNode::HwLightCompoundNode() :
    _lightUniforms(HW::LIGHT_DATA, EMPTY_STRING)
{
}

ShaderNodeImplPtr HwLightCompoundNode::create()
{
    return std::make_shared<HwLightCompoundNode>();
}

void HwLightCompoundNode::initialize(const InterfaceElement& element, GenContext& context)
{
    CompoundNode::initialize(element, context);

    // Store light uniforms for all inputs on the interface
    const NodeGraph& graph = static_cast<const NodeGraph&>(element);
    NodeDefPtr nodeDef = graph.getNodeDef();
    for (InputPtr input : nodeDef->getActiveInputs())
    {
        const TypeDesc type = context.getTypeDesc(input->getType());
        _lightUniforms.add(type, input->getName());
    }
}

void HwLightCompoundNode::createVariables(const ShaderNode&, GenContext& context, Shader& shader) const
{
    // Create variables for all child nodes
    for (ShaderNode* childNode : _rootGraph->getNodes())
    {
        childNode->getImplementation().createVariables(*childNode, context, shader);
    }

    ShaderStage& ps = shader.getStage(Stage::PIXEL);
    VariableBlock& lightData = ps.getUniformBlock(HW::LIGHT_DATA);

    // Create all light uniforms
    for (size_t i = 0; i < _lightUniforms.size(); ++i)
    {
        ShaderPort* u = const_cast<ShaderPort*>(_lightUniforms[i]);
        lightData.add(u->getSelf());
    }

    const HwShaderGenerator& shadergen = static_cast<const HwShaderGenerator&>(context.getShaderGenerator());
    shadergen.addStageLightingUniforms(context, ps);
}

void HwLightCompoundNode::emitFunctionDefinition(const ShaderNode& /*node*/, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const HwShaderGenerator& shadergen = static_cast<const HwShaderGenerator&>(context.getShaderGenerator());
        const Syntax& syntax = shadergen.getSyntax();

        const string& vec3 = syntax.getTypeName(Type::VECTOR3);
        const string& out_lightshader = syntax.getOutputTypeName(Type::LIGHTSHADER);
        const string vec3_zero = syntax.getValue(Type::VECTOR3, HW::VEC3_ZERO);

        // Emit functions for all child nodes
        shadergen.emitFunctionDefinitions(*_rootGraph, context, stage);

        shadergen.emitLine("void " + _functionName + "(LightData light, "+vec3+" position, "+out_lightshader+" result)", stage, false);

        shadergen.emitFunctionBodyBegin(*_rootGraph, context, stage);

        // Emit all texturing nodes. These are inputs to any
        // closure/shader nodes and need to be emitted first.
        shadergen.emitFunctionCalls(*_rootGraph, context, stage, ShaderNode::Classification::TEXTURE);

        shadergen.emitLine("ClosureData closureData = makeClosureData(CLOSURE_TYPE_EMISSION, "+vec3_zero+", -L, light.direction, "+vec3_zero+", 0)", stage);
        shadergen.emitFunctionCalls(*_rootGraph, context, stage, ShaderNode::Classification::SHADER | ShaderNode::Classification::LIGHT);

        shadergen.emitFunctionBodyEnd(*_rootGraph, context, stage);
    }
}

void HwLightCompoundNode::emitFunctionCall(const ShaderNode&, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        shadergen.emitLine(_functionName + "(light, position, result)", stage);
    }
}

MATERIALX_NAMESPACE_END
