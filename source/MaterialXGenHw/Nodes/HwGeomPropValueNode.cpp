//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenHw/Nodes/HwGeomPropValueNode.h>

#include <MaterialXGenHw/HwConstants.h>
#include <MaterialXGenHw/HwShaderGenerator.h>
#include <MaterialXGenShader/Exception.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>

MATERIALX_NAMESPACE_BEGIN

static const string GEOMPROP = "geomprop";

ShaderNodeImplPtr HwGeomPropValueNode::create()
{
    return std::make_shared<HwGeomPropValueNode>();
}

// Find the ShaderInput that contains the actual name of the geomprop to read
static const ShaderInput* getGeomPropValueInput(const ShaderNode& node, const GenContext& context)
{
    // We should always have the "geomprop" input locally on this node
    const ShaderInput* geomPropInput = node.getInput(GEOMPROP);
    if (!geomPropInput)
    {
        throw ExceptionShaderGenError("No 'geomprop' input found on geompropvalue node '" + node.getName() + "'.");
    }

    // Depth into the compound instance stack, measured from the innermost
    // instance. Incremented each time we cross a graph boundary so that nested
    // compounds resolve to the correct enclosing instance.
    size_t depth = 0;

    while (geomPropInput)
    {
        // If it has a local value set, then we've found the ShaderInput and can return that
        ConstValuePtr value = geomPropInput->getValue();
        if (value)
        {
            return geomPropInput;
        }

        // If we don't have a local value, then we may have an incoming connection from a containing NodeGraph
        // Note : the "geomprop" input on the base node is "uniform" so it cannot be driven by the output from
        // another node.
        const ShaderOutput* upstreamPort = geomPropInput->getConnection();
        if (!upstreamPort)
        {
            return nullptr;
        }

        const ShaderNode* upstreamNode = upstreamPort->getNode();
        if (!upstreamNode || !upstreamNode->isAGraph())
        {
            return nullptr;
        }

        // We've reached a graph input socket. Step out to the matching compound
        // instance, walking the instance stack outward in tandem with the graph
        // boundary we just crossed, so that nested compounds resolve to the
        // correct instance rather than always the innermost one.
        const ShaderNode* compoundInstance = context.getCompoundInstanceNode(depth++);
        if (!compoundInstance)
        {
            return nullptr;
        }

        // Consider the input on the compound instance to be the new potential
        // source for the value of "geomprop". The next iteration either finds a
        // value here or continues climbing.
        geomPropInput = compoundInstance->getInput(upstreamPort->getName());
    }

    return nullptr;
}

void HwGeomPropValueNode::createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const
{
    const ShaderInput* geomPropInput = getGeomPropValueInput(node, context);
    if (!geomPropInput || !geomPropInput->getValue())
    {
        throw ExceptionShaderGenError("No 'geomprop' value found for geompropvalue node '" + node.getName() + "'. Don't know what property to bind");
    }
    const string geomProp = geomPropInput->getValue()->getValueString();
    const ShaderOutput* output = node.getOutput();

    ShaderStage& vs = shader.getStage(Stage::VERTEX);
    ShaderStage& ps = shader.getStage(Stage::PIXEL);

    addStageInput(HW::VERTEX_INPUTS, output->getType(), HW::T_IN_GEOMPROP + "_" + geomProp, vs);
    addStageConnector(HW::VERTEX_DATA, output->getType(), HW::T_IN_GEOMPROP + "_" + geomProp, vs, ps);
}

void HwGeomPropValueNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const HwShaderGenerator& shadergen = static_cast<const HwShaderGenerator&>(context.getShaderGenerator());

    const ShaderInput* geomPropInput = getGeomPropValueInput(node, context);
    if (!geomPropInput)
    {
        throw ExceptionShaderGenError("No 'geomprop' value found for geompropvalue node '" + node.getName() + "'. Don't know what property to bind");
    }
    const string geomname = geomPropInput->getValue()->getValueString();
    const string variable = HW::T_IN_GEOMPROP + "_" + geomname;

    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        const string prefix = shadergen.getVertexDataPrefix(vertexData);
        ShaderPort* geomprop = vertexData[variable];
        if (!geomprop->isEmitted())
        {
            shadergen.emitLine(prefix + geomprop->getVariable() + " = " + HW::T_IN_GEOMPROP + "_" + geomname, stage);
            geomprop->setEmitted();
        }
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
        const string prefix = shadergen.getVertexDataPrefix(vertexData);
        ShaderPort* geomprop = vertexData[variable];
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        shadergen.emitString(" = " + prefix + geomprop->getVariable(), stage);
        shadergen.emitLineEnd(stage);
    }
}

ShaderNodeImplPtr HwGeomPropValueNodeAsUniform::create()
{
    return std::make_shared<HwGeomPropValueNodeAsUniform>();
}

void HwGeomPropValueNodeAsUniform::createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const
{
    const ShaderInput* geomPropInput = getGeomPropValueInput(node, context);
    if (!geomPropInput || !geomPropInput->getValue())
    {
        throw ExceptionShaderGenError("No 'geomprop' value found for geompropvalue node '" + node.getName() + "'. Don't know what property to bind");
    }
    const string geomProp = geomPropInput->getValue()->getValueString();
    ShaderStage& ps = shader.getStage(Stage::PIXEL);
    ShaderPort* uniform = addStageUniform(HW::PRIVATE_UNIFORMS, node.getOutput()->getType(), HW::T_GEOMPROP + "_" + geomProp, ps);
    uniform->setPath(geomPropInput->getPath());
}

void HwGeomPropValueNodeAsUniform::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        const ShaderInput* geomPropInput = getGeomPropValueInput(node, context);
        if (!geomPropInput)
        {
            throw ExceptionShaderGenError("No 'geomprop' value found for geompropvalue node '" + node.getName() + "'. Don't know what property to bind");
        }
        const string attrName = geomPropInput->getValue()->getValueString();
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        shadergen.emitString(" = " + HW::T_GEOMPROP + "_" + attrName, stage);
        shadergen.emitLineEnd(stage);
    }
}

MATERIALX_NAMESPACE_END
