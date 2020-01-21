//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtNode.h>
#include <MaterialXRuntime/Private/PvtNode.h>

#include <MaterialXRuntime/RtToken.h>
#include <MaterialXRuntime/RtTypeDef.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

const RtObjType PvtNode::_typeId = RtObjType::NODE;
const RtToken PvtNode::_typeName = RtToken("node");

// Construction with an interface is for nodes.
PvtNode::PvtNode(const RtToken& name, const PvtDataHandle& nodedef, PvtPrim* parent) :
    PvtPrim(name, parent),
    _nodedef(nodedef)
{
}

// Construction without an interface is only for nodegraphs.
PvtNode::PvtNode(const RtToken& name, PvtPrim* parent) :
    PvtPrim(name, parent),
    _nodedef(nullptr)
{
}

PvtDataHandle PvtNode::createNew(const RtToken& name, const PvtDataHandle& nodedefH, PvtPrim* parent)
{
    PvtNodeDef* nodedef = nodedefH->asA<PvtNodeDef>();

    // If a name is not given generate one.
    RtToken nodeName = name;
    if (nodeName == EMPTY_TOKEN)
    {
        nodeName = RtToken(nodedef->getNodeTypeName().str() + "1");
    }

    // Make the name unique.
    nodeName = parent->makeUniqueName(nodeName);

    PvtDataHandle nodeH(new PvtNode(nodeName, nodedefH, parent));
    PvtNode* node = nodeH->asA<PvtNode>();

    // The node type name is the prim type for nodes.
    node->setPrimTypeName(nodedef->getNodeTypeName());

    // Create all attributes according to the definition.
    for (const PvtDataHandle& attrH : nodedef->getAllAttributes())
    {
        const PvtAttribute* attr = attrH->asA<PvtAttribute>();
        const uint32_t flags = attr->getFlags() | RtAttrFlag::CONNECTABLE;
        PvtAttribute* nodeAttr = node->createAttribute(attr->getName(), attr->getType(), flags);
        RtValue::copy(attr->getType(), attr->getValue(), nodeAttr->getValue());
    }

    return nodeH;
}

}
