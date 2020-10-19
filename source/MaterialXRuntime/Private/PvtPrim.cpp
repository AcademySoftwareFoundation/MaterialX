//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtPrim.h>
#include <MaterialXRuntime/Private/PvtPath.h>

#include <MaterialXRuntime/RtTraversal.h>

#include <MaterialXCore/Util.h>

namespace MaterialX
{

RT_DEFINE_RUNTIME_OBJECT(PvtPrim, RtObjType::PRIM, "PvtPrim")

PvtPrim::PvtPrim(const RtTypeInfo* typeInfo, const RtToken& name, PvtPrim* parent) :
    PvtObject(name, parent),
    _typeInfo(typeInfo)
{
    setTypeBit<PvtPrim>();
}

PvtDataHandle PvtPrim::createNew(const RtTypeInfo* type, const RtToken& name, PvtPrim* parent)
{
    // Make the name unique.
    const RtToken primName = parent->makeUniqueChildName(name);
    return PvtDataHandle(new PvtPrim(type, primName, parent));
}

void PvtPrim::dispose(bool state)
{
    for (const PvtDataHandle& hnd : _relOrder)
    {
        hnd->setDisposed(state);
    }
    for (const PvtDataHandle& hnd : _attrOrder)
    {
        hnd->setDisposed(state);
    }
    for (const PvtDataHandle& hnd : _primOrder)
    {
        hnd->asA<PvtPrim>()->dispose(state);
    }
    setDisposed(state);
}

void PvtPrim::destroy()
{
    // Disconnect and delete all relationships.
    for (PvtDataHandle& hnd : _relOrder)
    {
        PvtRelationship* rel = hnd->asA<PvtRelationship>();
        rel->clearTargets();
    }
    _relOrder.clear();
    _relMap.clear();

    // Disconnect and delete all attributes.
    for (PvtDataHandle& hnd : _attrOrder)
    {
        if (hnd->isA<PvtInput>())
        {
            hnd->asA<PvtInput>()->clearConnection();
        }
        else if (hnd->isA<PvtOutput>())
        {
            hnd->asA<PvtOutput>()->clearConnections();
        }
    }
    _attrOrder.clear();
    _attrMap.clear();

    // Destroy all child prims reqursively.
    for (const PvtDataHandle& hnd : _primOrder)
    {
        hnd->asA<PvtPrim>()->destroy();
    }
    _primOrder.clear();
    _primMap.clear();

    // Tag as disposed.
    dispose(true);
}

PvtRelationship* PvtPrim::createRelationship(const RtToken& name)
{
    if (getRelationship(name))
    {
        throw ExceptionRuntimeError("A relationship named '" + name.str() + "' already exists in prim '" + getName().str() + "'");
    }

    PvtDataHandle relH(new PvtRelationship(name, this));
    _relOrder.push_back(relH);
    _relMap[name] = relH;

    return relH->asA<PvtRelationship>();
}

void PvtPrim::removeRelationship(const RtToken& name)
{
    PvtRelationship* rel = getRelationship(name);
    if (rel)
    {
        for (auto it = _relOrder.begin(); it != _relOrder.end(); ++it)
        {
            PvtRelationship* r = (*it)->asA<PvtRelationship>();
            if (r == rel)
            {
                _relOrder.erase(it);
                break;
            }
        }
        rel->setDisposed(true);
        _relMap.erase(name);
    }
}

void PvtPrim::renameRelationship(const RtToken& name, const RtToken& newName)
{
    if (getRelationship(newName))
    {
        throw ExceptionRuntimeError("A relationship named '" + newName.str() + "' already exists in prim '" + getName().str() + "'");
    }
    PvtRelationship* rel = getRelationship(name);
    if (rel)
    {
        rel->setName(newName);
        _relMap[newName] = rel->hnd();
        _relMap.erase(name);
    }
}

PvtAttribute* PvtPrim::createAttribute(const RtToken& name, const RtToken& type, uint32_t flags)
{
    if (getAttribute(name))
    {
        throw ExceptionRuntimeError("An attribute named '" + name.str() + "' already exists in prim '" + getName().str() + "'");
    }

    PvtDataHandle attrH(new PvtAttribute(name, type, flags, this));
    _attrOrder.push_back(attrH);
    _attrMap[name] = attrH;

    return attrH->asA<PvtAttribute>();
}

PvtInput* PvtPrim::createInput(const RtToken& name, const RtToken& type, uint32_t flags)
{
    // Inputs with type filename, token or string must always be uniform.
    if (type == RtType::FILENAME || type == RtType::TOKEN || type == RtType::STRING)
    {
        flags |= RtAttrFlag::UNIFORM;
    }

    RtToken uniqueName = makeUniqueChildName(name);
    PvtDataHandle attrH(new PvtInput(uniqueName, type, flags, this));
    _attrOrder.push_back(attrH);
    _attrMap[uniqueName] = attrH;

    return attrH->asA<PvtInput>();
}

PvtOutput* PvtPrim::createOutput(const RtToken& name, const RtToken& type, uint32_t flags)
{
    RtToken uniqueName = makeUniqueChildName(name);
    PvtDataHandle attrH(new PvtOutput(uniqueName, type, flags, this));
    _attrOrder.push_back(attrH);
    _attrMap[uniqueName] = attrH;

    return attrH->asA<PvtOutput>();
}

void PvtPrim::removeAttribute(const RtToken& name)
{
    PvtAttribute* attr = getAttribute(name);
    if (attr)
    {
        for (auto it = _attrOrder.begin(); it != _attrOrder.end(); ++it)
        {
            PvtAttribute* a = (*it)->asA<PvtAttribute>();
            if (a == attr)
            {
                _attrOrder.erase(it);
                break;
            }
        }
        attr->setDisposed(true);
        _attrMap.erase(name);
    }
}

void PvtPrim::setAttributeName(const RtToken& name, const RtToken& newName)
{
    PvtAttribute* attr = getAttribute(name);
    if (attr)
    {
        attr->setName(newName);
        _attrMap[newName] = attr->hnd();
        _attrMap.erase(name);
    }
    else
    {
        throw ExceptionRuntimeError("Unable to set attribute name. Attribute named: " + name.str() + " does not exist.");
    }
}

RtToken PvtPrim::renameAttribute(const RtToken& name, const RtToken& newName)
{
    RtToken uniqueNewName = makeUniqueChildName(newName);
    setAttributeName(name, uniqueNewName);
    return uniqueNewName;
}

RtAttrIterator PvtPrim::getAttributes(RtObjectPredicate predicate) const
{
    return RtAttrIterator(hnd(), predicate);
}

RtPrimIterator PvtPrim::getChildren(RtObjectPredicate predicate) const
{
    return RtPrimIterator(hnd(), predicate);
}

RtToken PvtPrim::makeUniqueChildName(const RtToken& name) const
{
    RtToken newName = name;

    // Check if there is another child with this name.
    const PvtPrim* otherChild = getChild(name);
    const PvtAttribute* otherAttr = getAttribute(name);
    if (otherChild || otherAttr)
    {
        // Find a number to append to the name, incrementing
        // the counter until a unique name is found.
        string baseName = name.str();
        int i = 1;
        const size_t n = name.str().find_last_not_of("0123456789") + 1;
        if (n < name.str().size())
        {
            const string number = name.str().substr(n);
            i = std::stoi(number) + 1;
            baseName = baseName.substr(0, n);
        }
        // Iterate until there is no other child with the resulting name.
        do {
            newName = RtToken(baseName + std::to_string(i++));
            otherChild = getChild(newName);
	    otherAttr = getAttribute(newName);
        } while (otherChild || otherAttr);
    }
    return newName;
}

void PvtPrim::addChildPrim(const PvtPrim* prim)
{
    _primOrder.push_back(prim->hnd());
    _primMap[prim->getName()] = prim->hnd();
}

void PvtPrim::removeChildPrim(const PvtPrim* prim)
{
    for (auto it = _primOrder.begin(); it != _primOrder.end(); ++it)
    {
        if ((*it).get() == prim)
        {
            _primOrder.erase(it);
            break;
        }
    }
    _primMap.erase(prim->getName());
}

}
