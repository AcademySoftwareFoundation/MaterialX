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

    // Find the first surface node in the compound and trace its
    // bsdf/edf/opacity inputs back through interfacename to the
    // nodedef's external input names.
    for (NodePtr node : graph.getNodes())
    {
        if (node->getCategory() == SURFACE_CATEGORY)
        {
            InputPtr bsdfIn = node->getInput("bsdf");
            if (bsdfIn && !bsdfIn->getInterfaceName().empty())
            {
                _bsdfInputName = bsdfIn->getInterfaceName();
            }

            InputPtr edfIn = node->getInput("edf");
            if (edfIn && !edfIn->getInterfaceName().empty())
            {
                _edfInputName = edfIn->getInterfaceName();
            }

            InputPtr opacityIn = node->getInput("opacity");
            if (opacityIn && !opacityIn->getInterfaceName().empty())
            {
                _opacityInputName = opacityIn->getInterfaceName();
            }

            break;
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
