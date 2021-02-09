//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenMdl/Nodes/BsdfNodesMdl.h>
#include <MaterialXGenShader/Nodes/LayerNode.h>

namespace MaterialX
{

ShaderNodeImplPtr DielectricBsdfNodeMdl::create()
{
    return std::make_shared<DielectricBsdfNodeMdl>();
}

void DielectricBsdfNodeMdl::addInputs(ShaderNode& node, GenContext&) const
{
    // Add layering support.
    LayerNode::addLayerSupport(node);
}


ShaderNodeImplPtr SheenBsdfNodeMdl::create()
{
    return std::make_shared<SheenBsdfNodeMdl>();
}

void SheenBsdfNodeMdl::addInputs(ShaderNode& node, GenContext&) const
{
    // Add layering support.
    LayerNode::addLayerSupport(node);
}

} // namespace MaterialX
