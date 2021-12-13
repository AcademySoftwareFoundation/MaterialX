//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenMdl/Nodes/ClosureLayerNodeMdl.h>
#include <MaterialXGenMdl/MdlShaderGenerator.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/TypeDesc.h>

MATERIALX_NAMESPACE_BEGIN

const string ClosureLayerNodeMdl::TOP = "top";
const string ClosureLayerNodeMdl::BASE = "base";

ShaderNodeImplPtr ClosureLayerNodeMdl::create()
{
    return std::make_shared<ClosureLayerNodeMdl>();
}

void ClosureLayerNodeMdl::emitFunctionCall(const ShaderNode& _node, GenContext& context, ShaderStage& stage) const
{
    const ShaderGenerator& shadergen = context.getShaderGenerator();

    ShaderNode& node = const_cast<ShaderNode&>(_node);

    ShaderInput* topInput = node.getInput(TOP);
    ShaderInput* baseInput = node.getInput(BASE);
    ShaderOutput* output = node.getOutput();

    //
    // 1. Handle the BSDF-over-VDF case
    //
    if (baseInput->getType() == Type::VDF)
    {
        // Make sure we have a top BSDF connected.
        if (!topInput->getConnection())
        {
            // No top BSDF so just emit an empty material.
            shadergen.emitLine("material " + output->getVariable() + " = material()", stage);
            return;
        }

        // Emit function call for top node if it's a sibling node
        // and not the graph interface.
        ShaderNode* top = topInput->getConnection()->getNode();
        if (top->getParent() == node.getParent())
        {
            shadergen.emitFunctionCall(*top, context, stage);
        }

        // Emit function call for base node if it's a sibling node
        // and not the graph interface.
        if (baseInput->getConnection())
        {
            ShaderNode* base = baseInput->getConnection()->getNode();
            if (base->getParent() == node.getParent())
            {
                shadergen.emitFunctionCall(*base, context, stage);
            }
        }

        const string t = shadergen.getUpstreamResult(topInput, context);
        const string b = shadergen.getUpstreamResult(baseInput, context);

        // Join the BSDF and VDF into a single material.
        shadergen.emitLine("material " + output->getVariable() 
            + " = material(surface: " + t + ".surface, backface: " + t + ".backface, ior: " + t + ".ior, volume: " + b + ".volume)", stage);

        return;
    }

    //
    // 2. Handle the BSDF-over-BSDF case
    //

    // Make sure the layer is fully connected.
    if (!(topInput->getConnection() && baseInput->getConnection()))
    {
        // Just emit an empty material.
        shadergen.emitLine("material " + output->getVariable() + " = material()", stage);
        return;
    }

    ShaderNode* top = topInput->getConnection()->getNode();
    ShaderNode* base = baseInput->getConnection()->getNode();

    // Make sure top BSDF is a sibling node and not the graph interface.
    if (top->getParent() != node.getParent())
    {
        shadergen.emitComment("Warning: MDL has no support for layering BSDFs through a graph interface. Only the top BSDF will used.", stage);
        shadergen.emitLine("material " + output->getVariable() + " = " + shadergen.getUpstreamResult(topInput, context), stage);
        return;
    }

    // Only a subset of the MaterialX BSDF nodes can be layered vertically in MDL.
    // This is because MDL only supports layering through BSDF nesting with a base
    // input, and it's only possible to do this workaround on a subset of the BSDFs.
    // So if the top BSDF doesn't have a base input, we can only emit the top BSDF 
    // without any base layering.
    //
    ShaderInput* topNodeBaseInput = top->getInput(BASE);
    if (!topNodeBaseInput)
    {
        shadergen.emitComment("Warning: MDL has no support for layering BSDF nodes without a base input. Only the top BSDF will used.", stage);

        // Change the state so we emit the top BSDF function 
        // with output variable name from the layer node itself.
        ScopedSetVariableName setVariable(output->getVariable(), top->getOutput());

        // Make the call.
        top->getImplementation().emitFunctionCall(*top, context, stage);

        return;
    }

    // Emit the base BSDF function call.
    // Make sure it's a sibling node and not the graph interface.
    if (base->getParent() == node.getParent())
    {
        shadergen.emitFunctionCall(*base, context, stage);
    }
    // Emit the layer operation with the top BSDF function call.
    // Change the state so we emit the top BSDF function with
    // base BSDF connection and output variable name from the
    // layer operator itself.
    topNodeBaseInput->makeConnection(base->getOutput());
    ScopedSetVariableName setVariable(output->getVariable(), top->getOutput());

    // Make the call.
    top->getImplementation().emitFunctionCall(*top, context, stage);

    // Restore state.
    topNodeBaseInput->breakConnection();
}


ShaderNodeImplPtr LayerableNodeMdl::create()
{
    return std::make_shared<LayerableNodeMdl>();
}

void LayerableNodeMdl::addInputs(ShaderNode& node, GenContext&) const
{
    // Add the input to hold base layer BSDF.
    node.addInput(ClosureLayerNodeMdl::BASE, Type::BSDF);
}

MATERIALX_NAMESPACE_END
