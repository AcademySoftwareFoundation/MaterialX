//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/Nodes/BsdfNodes.h>
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


ShaderNodeImplPtr ConductorBsdfNode::create()
{
    return std::make_shared<ConductorBsdfNode>();
}

void ConductorBsdfNode::addInputs(ShaderNode& node, GenContext&) const
{
    // Add thin-film support.
    ThinFilmNode::addThinFilmSupport(node);
}

ShaderNodeImplPtr HwConductorBsdfNode::create()
{
    return std::make_shared<HwConductorBsdfNode>();
}

void HwConductorBsdfNode::addInputs(ShaderNode& node, GenContext&) const
{
    // Add thin-film support.
    ThinFilmNode::addThinFilmSupport(node);
}


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
