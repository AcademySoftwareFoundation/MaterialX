//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtStage.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtPath.h>

#include <MaterialXRuntime/Private/PvtStage.h>

namespace MaterialX
{

namespace
{
    // Syntactic sugar
    inline PvtStage* _cast(void* ptr)
    {
        return static_cast<PvtStage*>(ptr);
    }
}

RtStage::RtStage() :
    _ptr(nullptr)
{
}

RtStage::~RtStage()
{
    delete _cast(_ptr);
}

RtStagePtr RtStage::createNew(const RtToken& name)
{
    // Create the shared stage object.
    RtStagePtr stage(new RtStage());

    // Create the private stage implementation.
    stage->_ptr = new PvtStage(name, RtStageWeakPtr(stage));

    // Return the shared wrapper object.
    return stage;
}

const RtToken& RtStage::getName() const
{
    return _cast(_ptr)->getName();
}

RtPrim RtStage::createPrim(const RtToken& typeName)
{
    return createPrim(RtPath("/"), EMPTY_TOKEN, typeName);
}

RtPrim RtStage::createPrim(const RtPath& path, const RtToken& typeName)
{
    PvtPrim* prim = _cast(_ptr)->createPrim(*static_cast<PvtPath*>(path._ptr), typeName);
    return prim->hnd();
}

RtPrim RtStage::createPrim(const RtPath& parentPath, const RtToken& name, const RtToken& typeName)
{
    PvtPrim* prim = _cast(_ptr)->createPrim(*static_cast<PvtPath*>(parentPath._ptr), name, typeName);
    return prim->hnd();
}

void RtStage::removePrim(const RtPath& path)
{
    _cast(_ptr)->removePrim(*static_cast<PvtPath*>(path._ptr));
}

RtToken RtStage::renamePrim(const RtPath& path, const RtToken& newName)
{
    return _cast(_ptr)->renamePrim(*static_cast<PvtPath*>(path._ptr), newName);
}

RtToken RtStage::reparentPrim(const RtPath& path, const RtPath& newParentPath)
{
    return _cast(_ptr)->reparentPrim(
        *static_cast<PvtPath*>(path._ptr),
        *static_cast<PvtPath*>(newParentPath._ptr)
    );
}

RtPrim RtStage::getPrimAtPath(const RtPath& path)
{
    PvtPrim* prim = _cast(_ptr)->getPrimAtPath(*static_cast<PvtPath*>(path._ptr));
    return prim ? prim->hnd() : RtPrim();
}

RtPrim RtStage::getRootPrim()
{
    return _cast(_ptr)->getRootPrim()->hnd();
}

RtStageIterator RtStage::traverse(RtObjectPredicate predicate)
{
    return RtStageIterator(shared_from_this(), predicate);
}

void RtStage::addReference(RtStagePtr stage)
{
    _cast(_ptr)->addReference(stage);
}

RtStagePtr RtStage::getReference(const RtToken& name) const
{
    return _cast(_ptr)->getReference(name);
}

void RtStage::removeReference(const RtToken& name)
{
    _cast(_ptr)->removeReference(name);
}

void RtStage::removeReferences()
{
    _cast(_ptr)->removeReferences();
}

void RtStage::setName(const RtToken& name)
{
    _cast(_ptr)->setName(name);
}

}
