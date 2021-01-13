//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/Nodes/DielectricBsdfNode.h>
#include <MaterialXGenShader/Nodes/LayerNode.h>
#include <MaterialXGenShader/ShaderNode.h>

namespace MaterialX
{

ShaderNodeImplPtr DielectricBsdfNode::create()
{
    return std::make_shared<DielectricBsdfNode>();
}

void DielectricBsdfNode::addInputs(ShaderNode& node, GenContext& context) const
{
    // Add the input to hold base layer BSDF.
    node.addInput(LayerNode::BASE, Type::BSDF);

    // Add inputs from parent class.
    ThinFilmSupport::addInputs(node, context);
}


ShaderNodeImplPtr HwDielectricBsdfNode::create()
{
    return std::make_shared<HwDielectricBsdfNode>();
}

void HwDielectricBsdfNode::addInputs(ShaderNode& node, GenContext& context) const
{
    // Add the input for base layer BSDF.
    node.addInput(LayerNode::BASE, Type::BSDF);

    // Add any inputs according to parent class.
    HwThinFilmSupport::addInputs(node, context);
}

} // namespace MaterialX
