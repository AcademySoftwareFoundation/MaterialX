//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/Nodes/SheenBsdfNode.h>
#include <MaterialXGenShader/Nodes/LayerNode.h>
#include <MaterialXGenShader/ShaderNode.h>

namespace MaterialX
{

ShaderNodeImplPtr SheenBsdfNode::create()
{
    return std::make_shared<SheenBsdfNode>();
}

void SheenBsdfNode::addInputs(ShaderNode& node, GenContext&) const
{
    // Add layering support.
    LayerNode::addLayerSupport(node);
}


ShaderNodeImplPtr HwSheenBsdfNode::create()
{
    return std::make_shared<HwSheenBsdfNode>();
}

void HwSheenBsdfNode::addInputs(ShaderNode& node, GenContext&) const
{
    // Add layering support.
    LayerNode::addLayerSupport(node);
}

} // namespace MaterialX
