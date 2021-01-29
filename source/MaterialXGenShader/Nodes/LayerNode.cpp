//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/Nodes/LayerNode.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/TypeDesc.h>

namespace MaterialX
{

const string LayerNode::TOP = "top";
const string LayerNode::BASE = "base";

ShaderNodeImplPtr LayerNode::create()
{
    return std::make_shared<LayerNode>();
}

void LayerNode::emitFunctionCall(const ShaderNode& _node, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        const ShaderGenerator& shadergen = context.getShaderGenerator();

        ShaderNode& node = const_cast<ShaderNode&>(_node);

        ShaderInput* top = node.getInput(TOP);
        ShaderInput* base = node.getInput(BASE);
        ShaderOutput* output = node.getOutput();
        if (!(top && base && output))
        {
            throw ExceptionShaderGenError("Node '" + node.getName() + "' is not a valid layer node");
        }
        if (!top->getConnection())
        {
            // No top layer, just emit default value.
            shadergen.emitLineBegin(stage);
            shadergen.emitOutput(output, true, true, context, stage);
            shadergen.emitLineEnd(stage);
            shadergen.emitLineEnd(stage);
            return;
        }
        
        ShaderNode* topBsdf = top->getConnection()->getNode();

        // Layerable nodes require a BSDF input named "base"
        ShaderInput* topBsdfInput = topBsdf->getInput(BASE);
        if (!topBsdfInput || topBsdfInput->getType() != Type::BSDF)
        {
            throw ExceptionShaderGenError("Node connected as top layer '" + topBsdf->getName() + "' is not a layerable BSDF");
        }

        ShaderOutput* topBsdfOutput = topBsdf->getOutput();

        // Save state so we can restore it below
        ShaderOutput* topBsdfInputOldConnection = topBsdfInput->getConnection();
        const string topBsdfOutputOldVariable = topBsdfOutput->getVariable();

        // Change the state so we emit the top layer function 
        // with base layer connection and output variable name
        // from the layer node itself.
        ShaderOutput* baseBsdfOutput = base->getConnection();
        if (baseBsdfOutput)
        {
            topBsdfInput->makeConnection(baseBsdfOutput);
        }
        else
        {
            topBsdfInput->breakConnection();
        }
        topBsdfOutput->setVariable(output->getVariable());

        // Emit the function call.
        topBsdf->getImplementation().emitFunctionCall(*topBsdf, context, stage);

        // Restore state.
        if (topBsdfInputOldConnection)
        {
            topBsdfInput->makeConnection(topBsdfInputOldConnection);
        }
        topBsdfOutput->setVariable(topBsdfOutputOldVariable);

    END_SHADER_STAGE(stage, Stage::PIXEL)
}

void LayerNode::addLayerSupport(ShaderNode& node)
{
    // Add the input to hold base layer BSDF.
    node.addInput(LayerNode::BASE, Type::BSDF);
}

} // namespace MaterialX
