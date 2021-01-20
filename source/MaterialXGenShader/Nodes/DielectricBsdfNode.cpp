//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/Nodes/DielectricBsdfNode.h>
#include <MaterialXGenShader/Nodes/LayerNode.h>
#include <MaterialXGenShader/Nodes/ThinFilmNode.h>
#include <MaterialXGenShader/ShaderNode.h>

namespace MaterialX
{

ShaderNodeImplPtr DielectricBsdfNode::create()
{
    return std::make_shared<DielectricBsdfNode>();
}

void DielectricBsdfNode::addInputs(ShaderNode& node, GenContext&) const
{
    // Add layering support.
    LayerNode::addLayerSupport(node);

    // Add thin-film support.
    ThinFilmNode::addThinFilmSupport(node);
}


ShaderNodeImplPtr HwDielectricBsdfNode::create()
{
    return std::make_shared<HwDielectricBsdfNode>();
}

void HwDielectricBsdfNode::addInputs(ShaderNode& node, GenContext&) const
{
    // Add layering support.
    LayerNode::addLayerSupport(node);

    // Add thin-film support.
    ThinFilmNode::addThinFilmSupport(node);
}

} // namespace MaterialX
