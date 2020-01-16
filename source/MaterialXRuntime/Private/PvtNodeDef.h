//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTNODEDEF_H
#define MATERIALX_PVTNODEDEF_H

#include <MaterialXRuntime/Private/PvtPrim.h>

#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtValue.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PvtNodeDef : public PvtPrim
{
public:
    static RtObjType typeId() { return _typeId; }

    static const RtToken& typeName() { return _typeName; }

    static PvtDataHandle createNew(const RtToken& name, PvtPrim* parent);

    RtObjType getObjType() const override
    {
        return _typeId;
    }

    const RtToken& getObjTypeName() const override
    {
        return _typeName;
    }

    void setNodeTypeName(const RtToken& nodeTypeName);

    const RtToken& getNodeTypeName() const;

private:
    static const RtObjType _typeId;
    static const RtToken _typeName;

protected:
    PvtNodeDef(const RtToken& name, PvtPrim* parent);
};

}

#endif
