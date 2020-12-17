//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtObject.h>
#include <MaterialXRuntime/Private/PvtPath.h>
#include <MaterialXRuntime/Private/PvtStage.h>

#include <set>

/// @file
/// TODO: Docs

namespace MaterialX
{

RT_DEFINE_RUNTIME_OBJECT(PvtObject, RtObjType::OBJECT, "PvtObject")
RT_DEFINE_REF_PTR_FUNCTIONS(PvtObject)

PvtObject::PvtObject(const RtToken& name, PvtPrim* parent) :
    _typeBits(0),
    _name(name),
    _parent(parent)
{
    setTypeBit<PvtObject>();
}

PvtPath PvtObject::getPath() const
{
    return PvtPath(this);
}

PvtPrim* PvtObject::getRoot() const
{
    PvtPrim* root = isA<PvtPrim>() ? const_cast<PvtPrim*>(asA<PvtPrim>()) : _parent;
    while (root->_parent)
    {
        root = root->_parent;
    }
    return root;
}

RtStageWeakPtr PvtObject::getStage() const
{
    return getRoot()->asA<PvtStage::RootPrim>()->getStage();
}

RtTypedValue* PvtObject::addMetadata(const RtToken& name, const RtToken& type)
{
    auto it = _metadataMap.find(name);
    if (it != _metadataMap.end())
    {
        // Check if the data type is matching.
        if (it->second.getType() != type)
        {
            throw ExceptionRuntimeError("Metadata '" + name.str() + "' found with an unmatching datatype on object '" + getName().str() +"'");
        }
        return &it->second;
    }

    PvtPrim* prim = isA<PvtPrim>() ? asA<PvtPrim>() : _parent;
    _metadataMap[name] = RtTypedValue(type, RtValue::createNew(type, prim->prim()));
    _metadataOrder.push_back(name);

    return &_metadataMap[name];
}

void PvtObject::removeMetadata(const RtToken& name)
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

RtTypedValue* PvtObject::getMetadata(const RtToken& name, const RtToken& type)
{
    auto it = _metadataMap.find(name);
    if (it != _metadataMap.end())
    {
        // Check if the data type is matching.
        if (it->second.getType() != type)
        {
            throw ExceptionRuntimeError("Metadata '" + name.str() + "' found with an unmatching datatype on object '" + getName().str() + "'");
        }
        return &it->second;
    }
    return nullptr;
}

}
