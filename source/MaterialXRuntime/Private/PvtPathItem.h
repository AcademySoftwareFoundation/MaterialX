//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTPATHITEM_H
#define MATERIALX_PVTPATHITEM_H

#include <MaterialXRuntime/Private/PvtObject.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PvtPath;

class PvtPathItem : public PvtObject
{
public:
    const RtToken& getName() const
    {
        return _name;
    }

    PvtPath getPath() const;

    PvtPrim* getParent() const
    {
        return _parent;
    }

    PvtPrim* getRoot() const;

    PvtStage* getStage() const;

    RtTypedValue* addMetadata(const RtToken& name, const RtToken& type);

    void removeMetadata(const RtToken& name);

    const RtTypedValue* getMetadata(const RtToken& name) const
    {
        auto it = _metadataMap.find(name);
        return it != _metadataMap.end() ? &it->second : nullptr;
    }

    RtTypedValue* getMetadata(const RtToken& name)
    {
        auto it = _metadataMap.find(name);
        return it != _metadataMap.end() ? &it->second : nullptr;
    }

    // For serialization to file we need the order.
    const vector<RtToken>& getMetadataOrder() const
    {
        return _metadataOrder;
    }

protected:
    PvtPathItem(const RtToken& name, PvtPrim* parent);

    // Protected as arbitrary renaming is not supported.
    // Must be done from the owning stage.
    void setName(const RtToken& name)
    {
        _name = name;
    }

    // Protected as arbitrary reparenting is not supported.
    // Must be done from the owning stage.
    void setParent(PvtPrim* parent)
    {
        _parent = parent;
    }

    // TODO: Store a path instead of name token
    RtToken _name;
    PvtPrim* _parent;
    RtTokenMap<RtTypedValue> _metadataMap;
    vector<RtToken> _metadataOrder;

    friend class PvtPrim;
};

}

#endif
