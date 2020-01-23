//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTBACKDROP_H
#define MATERIALX_PVTBACKDROP_H

#include <MaterialXRuntime/Private/PvtPrim.h>

#include <MaterialXRuntime/RtObject.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PvtBackdrop : public PvtPrim
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

    static const RtToken WIDTH;
    static const RtToken HEIGHT;
    static const RtToken CONTAINS;

private:
    static const RtObjType _typeId;
    static const RtToken _typeName;

protected:
    PvtBackdrop(const RtToken& name, PvtPrim* parent);
};

}

#endif
