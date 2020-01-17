//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtStage.h>

#include <MaterialXRuntime/Private/PvtStage.h>

namespace MaterialX
{

RtStage::RtStage(const RtObject& obj) :
    RtApiBase(obj)
{
}

RtApiType RtStage::getApiType() const
{
    return RtApiType::STAGE;
}

const RtToken& RtStage::getName() const
{
    return hnd()->asA<PvtStage>()->getName();
}

RtObject RtStage::createNew(const RtToken& name)
{
    return PvtStage::createNew(name)->obj();
}

RtObject RtStage::createPrim(const RtToken& typeName, const RtObject def)
{
    return createPrim(RtPath("/"), EMPTY_TOKEN, typeName, def);
}

RtObject RtStage::createPrim(const RtPath& path, const RtToken& typeName, const RtObject def)
{
    PvtPrim* prim = hnd()->asA<PvtStage>()->createPrim(
        *static_cast<PvtPath*>(path._ptr),
        typeName,
        PvtObject::ptr<PvtObject>(def)
    );
    return prim->obj();
}

RtObject RtStage::createPrim(const RtPath& parentPath, const RtToken& name, const RtToken& typeName, const RtObject def)
{
    PvtPrim* prim = hnd()->asA<PvtStage>()->createPrim(
        *static_cast<PvtPath*>(parentPath._ptr),
        name,
        typeName,
        PvtObject::ptr<PvtObject>(def)
    );
    return prim->obj();
}

void RtStage::removePrim(const RtPath& path)
{
    hnd()->asA<PvtStage>()->removePrim(*static_cast<PvtPath*>(path._ptr));
}

RtToken RtStage::renamePrim(const RtPath& path, const RtToken& newName)
{
    return hnd()->asA<PvtStage>()->renamePrim(*static_cast<PvtPath*>(path._ptr), newName);
}

RtObject RtStage::getPrimAtPath(const RtPath& path)
{
    PvtPrim* prim = hnd()->asA<PvtStage>()->getPrimAtPath(*static_cast<PvtPath*>(path._ptr));
    return prim ? prim->obj() : RtObject();
}

RtObject RtStage::getRootPrim()
{
    return hnd()->asA<PvtStage>()->getRootPrim()->obj();
}

RtPrimIterator RtStage::traverse(RtObjectPredicate predicate)
{
    return hnd()->asA<PvtStage>()->traverse(predicate);
}

void RtStage::addReference(const RtObject& stage)
{
    hnd()->asA<PvtStage>()->addReference(PvtObject::hnd(stage));
}

void RtStage::removeReference(const RtToken& name)
{
    hnd()->asA<PvtStage>()->removeReference(name);
}

void RtStage::removeReferences()
{
    hnd()->asA<PvtStage>()->removeReferences();
}

size_t RtStage::numReferences() const
{
    return hnd()->asA<PvtStage>()->numReferences();
}

RtObject RtStage::getReference(size_t index) const
{
    return hnd()->asA<PvtStage>()->getReference(index)->obj();
}

RtObject RtStage::findReference(const RtToken& name) const
{
    PvtStage* ref = hnd()->asA<PvtStage>()->findReference(name);
    return ref ? ref->obj() : RtObject();
}

}
