//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenHw/Nodes/HwMaterialCompoundNode.h>

#include <MaterialXGenShader/ShaderNode.h>

#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Node.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{

const string MATERIAL_TYPE = "material";
const string SURFACE_CATEGORY = "surface";

} // anonymous namespace

ShaderNodeImplPtr HwMaterialCompoundNode::create()
{
    return std::make_shared<HwMaterialCompoundNode>();
}

void HwMaterialCompoundNode::initialize(const InterfaceElement& element, GenContext& context)
{
    ShaderNodeImpl::initialize(element, context);

    if (!element.isA<NodeGraph>())
    {
        return;
    }

    const NodeGraph& graph = static_cast<const NodeGraph&>(element);

    // Find the material-type output and traverse upstream to discover the
    // surface node, reading its bsdf/edf/opacity interface name mappings.
    for (OutputPtr output : graph.getOutputs())
    {
        if (output->getType() == MATERIAL_TYPE)
        {
            for (Edge edge : output->traverseGraph())
            {
                ElementPtr upstream = edge.getUpstreamElement();
                NodePtr upstreamNode = upstream ? upstream->asA<Node>() : nullptr;
                if (upstreamNode && upstreamNode->getCategory() == SURFACE_CATEGORY)
                {
                    InputPtr bsdfIn = upstreamNode->getInput("bsdf");
                    if (bsdfIn && !bsdfIn->getInterfaceName().empty())
                    {
                        _bsdfInputName = bsdfIn->getInterfaceName();
                    }

                    InputPtr edfIn = upstreamNode->getInput("edf");
                    if (edfIn && !edfIn->getInterfaceName().empty())
                    {
                        _edfInputName = edfIn->getInterfaceName();
                    }

                    InputPtr opacityIn = upstreamNode->getInput("opacity");
                    if (opacityIn && !opacityIn->getInterfaceName().empty())
                    {
                        _opacityInputName = opacityIn->getInterfaceName();
                    }

                    return;
                }
            }
        }
    }
}

void HwMaterialCompoundNode::addClassification(ShaderNode& node) const
{
    // Add SHADER|SURFACE so the top-level graph triggers the
    // surface shader code path in the HW pixel shader generators.
    node.addClassification(ShaderNode::Classification::SHADER |
                           ShaderNode::Classification::SURFACE);
}

const string& HwMaterialCompoundNode::getBsdfInputName() const
{
    return _bsdfInputName;
}

const string& HwMaterialCompoundNode::getEdfInputName() const
{
    return _edfInputName;
}

const string& HwMaterialCompoundNode::getOpacityInputName() const
{
    return _opacityInputName;
}

MATERIALX_NAMESPACE_END
