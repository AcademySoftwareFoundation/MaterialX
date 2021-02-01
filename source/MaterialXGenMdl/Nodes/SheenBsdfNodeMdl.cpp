//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenMdl/Nodes/SheenBsdfNodeMdl.h>
#include <MaterialXGenShader/Nodes/LayerNode.h>

namespace MaterialX
{

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
