//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/Nodes/ClosureLayerNode.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/TypeDesc.h>

MATERIALX_NAMESPACE_BEGIN

const string ClosureLayerNode::TOP = "top";
const string ClosureLayerNode::BASE = "base";

ShaderNodeImplPtr ClosureLayerNode::create()
{
    return std::make_shared<ClosureLayerNode>();
}

void ClosureLayerNode::emitFunctionCall(const ShaderNode& _node, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        ClosureContext* cct = context.getClosureContext();

        ShaderNode& node = const_cast<ShaderNode&>(_node);

        ShaderInput* topInput = node.getInput(TOP);
        ShaderInput* baseInput = node.getInput(BASE);
        ShaderOutput* output = node.getOutput();

        // Make sure the layer is fully connected.
        if (!(topInput->getConnection() && baseInput->getConnection()))
        {
            // Just declare the output variable with default value.
            emitOutputVariables(node, context, stage);
            return;
        }

        ShaderNode* top = topInput->getConnection()->getNode();
        ShaderNode* base = baseInput->getConnection()->getNode();

        // Evaluate top and base nodes and combine their result
        // according to throughput.
        //
        // TODO: In the BSDF over BSDF case should we emit code
        //       to check the top throughput amount before calling
        //       the base BSDF?

        // Make sure the connections are sibling nodes and not the graph interface.
        if (top->getParent() == node.getParent())
        {
            // If this layer node has closure parameters set,
            // we pass this on to the top component only.
            ScopedSetClosureParams setParams(&node, top, cct);
            shadergen.emitFunctionCall(*top, context, stage);
        }
        if (base->getParent() == node.getParent())
        {
            shadergen.emitFunctionCall(*base, context, stage);
        }

        // Get the result variables.
        const string& topResult = topInput->getConnection()->getVariable();
        const string& baseResult = baseInput->getConnection()->getVariable();

        // Calculate the layering result.
        emitOutputVariables(node, context, stage);
        if (base->getOutput()->getType() == Type::VDF)
        {
            shadergen.emitLine(output->getVariable() + ".response = " + topResult + ".response * " + baseResult + ".throughput", stage);
            shadergen.emitLine(output->getVariable() + ".throughput = " + topResult + ".throughput * " + baseResult + ".throughput", stage);
        }
        else
        {
            shadergen.emitLine(output->getVariable() + ".response = " + topResult + ".response + " + baseResult + ".response * " + topResult + ".throughput", stage);
            shadergen.emitLine(output->getVariable() + ".throughput = " + topResult + ".throughput * " + baseResult + ".throughput", stage);
        }
    }
}

MATERIALX_NAMESPACE_END
