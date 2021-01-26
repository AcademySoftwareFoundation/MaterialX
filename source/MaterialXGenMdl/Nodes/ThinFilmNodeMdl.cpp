//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenMdl/Nodes/ThinFilmNodeMdl.h>

#include <MaterialXGenShader/Nodes/LayerNode.h>
#include <MaterialXGenShader/ShaderNode.h>

namespace MaterialX
{

ShaderNodeImplPtr ThinFilmNodeMdl::create()
{
    return std::make_shared<ThinFilmNodeMdl>();
}

void ThinFilmNodeMdl::addInputs(ShaderNode& node, GenContext&) const
{
    // Add layering support.
    LayerNode::addLayerSupport(node);
}

} // namespace MaterialX
