//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenMdl/Nodes/ClosureLayerNodeMdl.h>
#include <MaterialXGenMdl/MdlShaderGenerator.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/TypeDesc.h>

MATERIALX_NAMESPACE_BEGIN

const string StringConstantsMdl::TOP = "top";
const string StringConstantsMdl::BASE = "base";
const string StringConstantsMdl::FG = "fg";
const string StringConstantsMdl::BG = "bg";
const string StringConstantsMdl::MIX = "mix";
const string StringConstantsMdl::TOP_WEIGHT = "top_weight";

ShaderNodeImplPtr ClosureLayerNodeMdl::create()
{
    return std::make_shared<ClosureLayerNodeMdl>();
}

void ClosureLayerNodeMdl::emitFunctionCall(const ShaderNode& _node, GenContext& context, ShaderStage& stage) const
{
    const ShaderGenerator& shadergen = context.getShaderGenerator();

    ShaderNode& node = const_cast<ShaderNode&>(_node);

    ShaderInput* topInput = node.getInput(StringConstantsMdl::TOP);
    ShaderInput* baseInput = node.getInput(StringConstantsMdl::BASE);
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
        shadergen.emitLine("material " + output->getVariable() +
                               " = material(surface: " + t + ".surface, backface: " + t +
                               ".backface, ior: " + t + ".ior, volume: " + b + ".volume)",
                           stage);

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

    // Transport the base bsdf further than one layer
    ShaderNode* baseReceiverNode = top;
    ShaderNode* mixTopWeightNode = nullptr;
    while (true)
    {
        // If the top node is again a layer, we don't want to override the base
        // parameter but instead aim for the base parameter of layers base
        if (baseReceiverNode->hasClassification(ShaderNode::Classification::LAYER))
        {
            baseReceiverNode = top->getInput(StringConstantsMdl::BASE)->getConnection()->getNode();
        }
        else
        {
            // TODO is there a more efficient way to check if the node is a mix_bsdf?
            std::string name = top->getImplementation().getName();
            if (name == "IM_mix_bsdf_genmdl")
            {
                // handle one special case: the top node is a mix where either fg or bg is empty
                // so basically a scale factor
                ShaderOutput* fgOutput = top->getInput(StringConstantsMdl::FG)->getConnection();
                ShaderOutput* bgOutput = top->getInput(StringConstantsMdl::BG)->getConnection();
                ShaderOutput* mixOutput = top->getInput(StringConstantsMdl::MIX)->getConnection();
                ShaderNode* fg = fgOutput ? fgOutput->getNode() : nullptr;
                ShaderNode* bg = bgOutput ? bgOutput->getNode() : nullptr;
                ShaderNode* mix = mixOutput ? mixOutput->getNode() : nullptr;
                if ((bool) fg != (bool) bg)
                {
                    baseReceiverNode = fg ? fg : bg; // take the node that is valid
                    top = baseReceiverNode;
                    mixTopWeightNode = mix;
                }
                break;
            }
            // we stop at elemental bsdfs
            // TODO handle mix, add, and multiply
            break;
        }
    }

    // Only a subset of the MaterialX BSDF nodes can be layered vertically in MDL.
    // This is because MDL only supports layering through BSDF nesting with a base
    // input, and it's only possible to do this workaround on a subset of the BSDFs.
    // So if the top BSDF doesn't have a base input, we can only emit the top BSDF
    // without any base layering.
    ShaderInput* topNodeBaseInput = baseReceiverNode->getInput(StringConstantsMdl::BASE);
    if (!topNodeBaseInput)
    {
        shadergen.emitComment("Warning: MDL has no support for layering BSDF nodes without a base input. Only the top BSDF will used.", stage);

        // Change the state so we emit the top BSDF function
        // with output variable name from the layer node itself.
        ScopedSetVariableName setVariable(output->getVariable(), top->getOutput());

        // Make the call.
        if (top->getParent() == node.getParent())
        {
            top->getImplementation().emitFunctionCall(*top, context, stage);
        }
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
    if (mixTopWeightNode)
    {
        ShaderInput* topNodeTopWeightInput = baseReceiverNode->getInput(StringConstantsMdl::TOP_WEIGHT);
        topNodeTopWeightInput->makeConnection(mixTopWeightNode->getOutput());
    }
    ScopedSetVariableName setVariable(output->getVariable(), top->getOutput());

    // Make the call.
    if (top->getParent() == node.getParent())
    {
        top->getImplementation().emitFunctionCall(*top, context, stage);
    }

    // Restore state.
    topNodeBaseInput->breakConnection();
}

ShaderNodeImplPtr LayerableNodeMdl::create()
{
    return std::make_shared<LayerableNodeMdl>();
}

void LayerableNodeMdl::addInputs(ShaderNode& node, GenContext& /*context*/) const
{
    // Add the input to hold base layer BSDF.
    node.addInput(StringConstantsMdl::BASE, Type::BSDF);

    // Set the top level weight default to 1.0
    ShaderInput* topWeightNode = node.addInput(StringConstantsMdl::TOP_WEIGHT, Type::FLOAT);
    ValuePtr value = TypedValue<float>::createValue(1.0f);
    topWeightNode->setValue(value);
}

bool LayerableNodeMdl::isEditable(const ShaderInput& input) const
{
    if (input.getName() == StringConstantsMdl::BASE ||
        input.getName() == StringConstantsMdl::TOP_WEIGHT)
    {
        return false;
    }
    return BASE::isEditable(input);
}

MATERIALX_NAMESPACE_END
