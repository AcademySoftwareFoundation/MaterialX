//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtPrim.h>
#include <MaterialXRuntime/Private/PvtPath.h>

#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtTraversal.h>

#include <MaterialXCore/Util.h>

namespace MaterialX
{

PvtPrim::PvtPrim(const RtToken& name, PvtPrim* parent) :
    PvtObject(name, parent)
{
    setTypeBit<PvtPrim>();
}

PvtDataHandle PvtPrim::createNew(const RtToken& name, PvtPrim* parent)
{
    // Make the name unique.
    const RtToken primName = parent->makeUniqueName(name);
    return PvtDataHandle(new PvtPrim(primName, parent));
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
    for (auto it = _relOrder.begin(); it != _relOrder.end(); ++it)
    {
        PvtRelationship* rel = (*it)->asA<PvtRelationship>();
        if (rel->getName() == name)
        {
            _relOrder.erase(it);
            break;
        }
    }
    _relMap.erase(name);
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
    if (getAttribute(name))
    {
        throw ExceptionRuntimeError("An input named '" + name.str() + "' already exists in prim '" + getName().str() + "'");
    }

    PvtDataHandle attrH(new PvtInput(name, type, flags, this));
    _attrOrder.push_back(attrH);
    _attrMap[name] = attrH;

    return attrH->asA<PvtInput>();
}

PvtOutput* PvtPrim::createOutput(const RtToken& name, const RtToken& type, uint32_t flags)
{
    if (getAttribute(name))
    {
        throw ExceptionRuntimeError("An output named '" + name.str() + "' already exists in prim '" + getName().str() + "'");
    }

    PvtDataHandle attrH(new PvtOutput(name, type, flags, this));
    _attrOrder.push_back(attrH);
    _attrMap[name] = attrH;

    return attrH->asA<PvtOutput>();
}

void PvtPrim::removeAttribute(const RtToken& name)
{
    for (auto it = _attrOrder.begin(); it != _attrOrder.end(); ++it)
    {
        PvtAttribute* attr = (*it)->asA<PvtAttribute>();
        if (attr->getName() == name)
        {
            _attrOrder.erase(it);
            break;
        }
    }
    _attrMap.erase(name);
}

RtAttrIterator PvtPrim::getAttributes(RtObjectPredicate predicate) const
{
    return RtAttrIterator(hnd(), predicate);
}

RtPrimIterator PvtPrim::getChildren(RtObjectPredicate predicate) const
{
    return RtPrimIterator(hnd(), predicate);
}

RtToken PvtPrim::makeUniqueName(const RtToken& name) const
{
    RtToken newName = name;

    // Check if there is another child with this name.
    const PvtPrim* otherChild = getChild(name);
    if (otherChild)
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
            newName = baseName + std::to_string(i++);
            otherChild = getChild(newName);
        } while (otherChild);
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
