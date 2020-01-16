//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtObject.h>
#include <MaterialXRuntime/Private/PvtPrim.h>
#include <MaterialXRuntime/Private/PvtPath.h>
#include <MaterialXRuntime/Private/PvtStage.h>

#include <set>

/// @file
/// TODO: Docs

namespace MaterialX
{

namespace
{
    static const std::set<RtApiType> OBJ_TO_API_RTTI[static_cast<int>(RtObjType::NUM_TYPES)] =
    {
        // NONE
        {
        },
        // PRIM
        {
            RtApiType::PRIM
        },
        // ATTRIBUTE
        {
            RtApiType::ATTRIBUTE,
            RtApiType::INPUT,
            RtApiType::OUTPUT
        },
        // NODEDEF
        {
            RtApiType::PRIM,
            RtApiType::NODEDEF
        },
        // NODE
        {
            RtApiType::PRIM,
            RtApiType::NODE
        },
        // NODEGRAPH
        {
            RtApiType::PRIM,
            RtApiType::NODE,
            RtApiType::NODEGRAPH
        },
        // STAGE
        {
            RtApiType::STAGE,
            RtApiType::FILE_IO
        },
    };
}

bool PvtObject::hasApi(RtApiType type) const
{
    return OBJ_TO_API_RTTI[int(getObjType())].count(type) != 0;
}

PvtPathItem::PvtPathItem(const RtToken& name, PvtPrim* parent) :
    _name(name),
    _parent(parent)
{
}

PvtPath PvtPathItem::getPath() const
{
    return PvtPath(this);
}

PvtPrim* PvtPathItem::getRoot() const
{
    PvtPrim* root = hasApi(RtApiType::PRIM) ? const_cast<PvtPrim*>(asA<PvtPrim>()) : _parent;
    while (root->_parent)
    {
        root = root->_parent;
    }
    return root;
}

PvtStage* PvtPathItem::getStage() const
{
    return getRoot()->asA<PvtStage::RootPrim>()->getStage();
}

RtTypedValue* PvtPathItem::addMetadata(const RtToken& name, const RtToken& type)
{
    auto it = _metadataMap.find(name);
    if (it != _metadataMap.end())
    {
        return &it->second;
    }

    _metadataMap[name] = RtTypedValue(type, RtValue::createNew(type, obj()));
    _metadataOrder.push_back(name);

    return &_metadataMap[name];
}

void PvtPathItem::removeMetadata(const RtToken& name)
{
    for (auto it = _metadataOrder.begin(); it != _metadataOrder.end(); ++it)
    {
        if (*it == name)
        {
            _metadataOrder.erase(it);
            break;
        }
    }
    _metadataMap.erase(name);
}

}
