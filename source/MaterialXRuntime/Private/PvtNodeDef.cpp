//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtNodeDef.h>
#include <MaterialXRuntime/Private/PvtNode.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

const RtObjType PvtNodeDef::_typeId = RtObjType::NODEDEF;
const RtToken PvtNodeDef::_typeName = RtToken("nodedef");

PvtNodeDef::PvtNodeDef(const RtToken& name, PvtPrim* parent) :
    PvtPrim(name, parent)
{
}

PvtDataHandle PvtNodeDef::createNew(const RtToken& name, PvtPrim* parent)
{
    if (parent->getChild(name))
    {
        throw ExceptionRuntimeError("A child named '" + name.str() + "' already exists in prim '" + parent->getName().str() + "'");
    }
    return PvtDataHandle(new PvtNodeDef(name, parent));
}

void PvtNodeDef::setNodeTypeName(const RtToken& nodeTypeName)
{
    RtTypedValue* md = addMetadata(PvtNode::typeName(), RtType::TOKEN);
    md->getValue().asToken() = nodeTypeName;
}

const RtToken& PvtNodeDef::getNodeTypeName() const
{
    const RtTypedValue* md = getMetadata(PvtNode::typeName());
    return md ? md->getValue().asToken() : EMPTY_TOKEN;
}

}
