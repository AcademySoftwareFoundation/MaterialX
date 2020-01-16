//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTNODEGRAPH_H
#define MATERIALX_PVTNODEGRAPH_H

#include <MaterialXRuntime/Private/PvtNode.h>

#include <MaterialXRuntime/RtObject.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PvtNodeGraph : public PvtNode
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

    PvtAttribute* createAttribute(const RtToken& name, const RtToken& type, uint32_t flags = 0) override;

    PvtAttribute* getSocket(const RtToken& name) const
    {
        auto it = _socketMap.find(name);
        return it != _socketMap.end() ? it->second->asA<PvtAttribute>() : nullptr;
    }

    string asStringDot() const;

private:
    static const RtObjType _typeId;
    static const RtToken _typeName;

protected:
    PvtNodeGraph(const RtToken& name, PvtPrim* parent);

    PvtDataHandleMap _socketMap;
    PvtDataHandleVec _socketOrder;
};

}

#endif
