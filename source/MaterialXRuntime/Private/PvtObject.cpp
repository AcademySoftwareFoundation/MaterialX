//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtObject.h>
#include <MaterialXRuntime/Private/PvtPath.h>
#include <MaterialXRuntime/Private/PvtStage.h>

#include <set>
#include <algorithm>

/// @file
/// TODO: Docs

namespace MaterialX
{

RT_DEFINE_RUNTIME_OBJECT(PvtObject, RtObjType::OBJECT, "PvtObject")
RT_DEFINE_REF_PTR_FUNCTIONS(PvtObject)

PvtObject::PvtObject(const RtIdentifier& name, PvtPrim* parent) :
    _typeBits(0),
    _name(name),
    _parent(parent)
{
    setTypeBit<PvtObject>();
}

PvtObject::~PvtObject()
{
    for (auto it : _attr)
    {
        delete it.second;
    }
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

RtTypedValue* PvtObject::createAttribute(const RtIdentifier& name, const RtIdentifier& type)
{
    RtTypedValue* value = getAttribute(name, type);
    if (value)
    {
        return value;
    }

    PvtPrim* owner = isA<PvtPrim>() ? asA<PvtPrim>() : _parent;
    RtTypedValue* attr = new RtTypedValue(type, RtValue::createNew(type, owner->prim()));
    _attr[name] = attr;
    _attrNames.push_back(name);

    return attr;
}

void PvtObject::removeAttribute(const RtIdentifier& name)
{
    auto it = _attr.find(name);
    if (it != _attr.end())
    {
        RtTypedValue* attr = it->second;
        for (auto it2 = _attrNames.begin(); it2 != _attrNames.end(); ++it2)
        {
            if (*it2 == it->first)
            {
                _attrNames.erase(it2);
                break;
            }
        }
        _attr.erase(it);
        delete attr;
    }
}

RtTypedValue* PvtObject::getAttribute(const RtIdentifier& name, const RtIdentifier& type)
{
    RtTypedValue* value = getAttribute(name);
    if (value && value->getType() != type)
    {
        throw ExceptionRuntimeError("Attribute '" + name.str() + "' found with an unmatching datatype on object '" + getName().str() + "'");
    }
    return value;
}


PvtObjHandle PvtObjectList::remove(const RtIdentifier& name)
{
    auto it = _map.find(name);
    if (it != _map.end())
    {
        PvtObjHandle hnd = it->second;
        _map.erase(it);
        _vec.erase(std::find(_vec.begin(), _vec.end(), hnd.get()));

        // Return the handle to keep the object alive
        // in case the intent was to only remove it
        // from the list but to keep its ref count.
        return hnd;
    }
    return PvtObjHandle();
}

RtIdentifier PvtObjectList::rename(const RtIdentifier& name, const RtIdentifier& newName, const PvtPrim* parent)
{
    auto it = _map.find(name);
    if (it == _map.end())
    {
        throw ExceptionRuntimeError("No object named '" + name.str() + "' exists, unable to rename.");
    }

    // Remove it from the name map first.
    // Make sure to hold on to the ref pointer
    // to keep the object alive.
    PvtObjHandle hnd = it->second;
    _map.erase(it);

    // Make sure the new name is unique within the parent.
    const RtIdentifier finalName = parent->makeUniqueChildName(newName);
    hnd->setName(finalName);
    _map[finalName] = hnd;

    return finalName;
}

}
