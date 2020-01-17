//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtPrim.h>
#include <MaterialXRuntime/Private/PvtPath.h>

#include <MaterialXCore/Util.h>

namespace MaterialX
{

namespace
{
    const RtToken _MD_PRIM("_prim");
}

const RtObjType PvtPrim::_typeId = RtObjType::PRIM;
const RtToken PvtPrim::_typeName = RtToken("prim");

PvtPrim::PvtPrim(const RtToken& name, PvtPrim* parent) :
    PvtPathItem(name, parent)
{
}

PvtDataHandle PvtPrim::createNew(const RtToken& name, PvtPrim* parent)
{
    // If a name is not given generate one.
    RtToken primName = name;
    if (primName == EMPTY_TOKEN)
    {
        primName = RtToken(_typeName.str() + "1");
    }

    // Make the name unique.
    primName = parent->makeUniqueName(primName);

    return PvtDataHandle(new PvtPrim(primName, parent));
}

const RtToken& PvtPrim::getPrimTypeName() const
{
    const RtTypedValue* md = getMetadata(_MD_PRIM);
    return md ? md->getValue().asToken() : EMPTY_TOKEN;
}

void PvtPrim::setPrimTypeName(const RtToken& primTypeName)
{
    RtTypedValue* md = addMetadata(_MD_PRIM, RtType::TOKEN);
    md->getValue().asToken() = primTypeName;
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

RtToken PvtPrim::makeUniqueName(const RtToken& name, const PvtPrim* child) const
{
    RtToken newName = name;

    // Check if there is another child with this name.
    const PvtPrim* otherChild = getChild(name);
    if (otherChild && otherChild != child)
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
        } while (otherChild && otherChild != child);
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
