//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTNODE_H
#define MATERIALX_PVTNODE_H

#include <MaterialXRuntime/Private/PvtPrim.h>
#include <MaterialXRuntime/Private/PvtNodeDef.h>

#include <MaterialXRuntime/RtNode.h>
#include <MaterialXRuntime/RtValue.h>
#include <MaterialXRuntime/RtTypeDef.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PvtNode : public PvtPrim
{
public:
    static RtObjType typeId() { return _typeId; }

    static const RtToken& typeName() { return _typeName; }

    static PvtDataHandle createNew(const RtToken& name, const PvtDataHandle& nodedef, PvtPrim* parent);

    RtObjType getObjType() const override
    {
        return _typeId;
    }

    const RtToken& getObjTypeName() const override
    {
        return _typeName;
    }

    PvtDataHandle getNodeDef() const
    {
        return _nodedef;
    }

private:
    static const RtObjType _typeId;
    static const RtToken _typeName;

protected:
    // Constructor creating a node with a fixed interface
    // This is the constructor to use for ordinary nodes.
    PvtNode(const RtToken& name, const PvtDataHandle& nodedef, PvtPrim* parent);

    // Constructor creating a node without a fixed interface.
    // Used for constructing nodegraphs.
    PvtNode(const RtToken& name, PvtPrim* parent);

protected:
    PvtDataHandle _nodedef;
};

}

#endif
