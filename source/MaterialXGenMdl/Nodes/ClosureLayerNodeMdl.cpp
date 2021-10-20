//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenMdl/Nodes/ClosureLayerNodeMdl.h>
#include <MaterialXGenMdl/MdlShaderGenerator.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/TypeDesc.h>

namespace MaterialX
{

const string ClosureLayerNodeMdl::TOP = "top";
const string ClosureLayerNodeMdl::BASE = "base";

ShaderNodeImplPtr ClosureLayerNodeMdl::create()
{
    return std::make_shared<ClosureLayerNodeMdl>();
}

void ClosureLayerNodeMdl::emitFunctionCall(const ShaderNode& _node, GenContext& context, ShaderStage& stage) const
{
BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
    const ShaderGenerator& shadergen = context.getShaderGenerator();

    ShaderNode& node = const_cast<ShaderNode&>(_node);

    ShaderInput* topInput = node.getInput(TOP);
    ShaderInput* baseInput = node.getInput(BASE);
    ShaderOutput* output = node.getOutput();

    // [TODO]
    // Make sure the layer is fully connected.
    if (!(topInput->getConnection() && baseInput->getConnection()))
    {
        // Just declare the output variable with default value.
        emitOutputVariables(node, context, stage);
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
    // TODO: For some cases we could work around this by transforming the graph.
    //
    //                        bsdf1              
    // bsdf1                       \             
    //      \                       layer1       
    //       mix                   /      \      
    //      /   \             bsdf3        \     
    // bsdf2     layer->  ==                mix->
    //          /             bsdf2        /     
    //     bsdf3                   \      /      
    //                              layer2       
    //                             /             
    //                        bsdf3              
    //
    ShaderInput* topNodeBaseInput = top->getInput(BASE);
    if (!topNodeBaseInput)
    {
        shadergen.emitComment("Warning: MDL has no support for layering BSDF nodes without a base input. Only the top BSDF will used.", stage);

        // Change the state so we emit the top BSDF function 
        // with output variable name from the layer node itself.
        ShaderOutput* topOutput = top->getOutput();
        const string topOutputOldVariable = topOutput->getVariable();
        topOutput->setVariable(output->getVariable());

        // Make the call.
        top->getImplementation().emitFunctionCall(*top, context, stage);

        // Restore state.
        topOutput->setVariable(topOutputOldVariable);

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
    ShaderOutput* topOutput = top->getOutput();
    const string topOutputOldVariable = topOutput->getVariable();
    topOutput->setVariable(output->getVariable());

    // Make the call.
    top->getImplementation().emitFunctionCall(*top, context, stage);

    // Restore state.
    topOutput->setVariable(topOutputOldVariable);
    topNodeBaseInput->breakConnection();

END_SHADER_STAGE(stage, Stage::PIXEL)
}


ShaderNodeImplPtr LayarableNodeMdl::create()
{
    return std::make_shared<LayarableNodeMdl>();
}

void LayarableNodeMdl::addInputs(ShaderNode& node, GenContext&) const
{
    // Add the input to hold base layer BSDF.
    node.addInput(ClosureLayerNodeMdl::BASE, Type::BSDF);
}

} // namespace MaterialX
