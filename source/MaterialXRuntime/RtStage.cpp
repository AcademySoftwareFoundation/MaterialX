//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtStage.h>

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
    // Create the shared wrapper stage object.
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

RtObject RtStage::createPrim(const RtToken& typeName, const RtObject def)
{
    return createPrim(RtPath("/"), EMPTY_TOKEN, typeName, def);
}

RtObject RtStage::createPrim(const RtPath& path, const RtToken& typeName, const RtObject def)
{
    PvtPrim* prim = _cast(_ptr)->createPrim(
        *static_cast<PvtPath*>(path._ptr),
        typeName,
        PvtObject::ptr<PvtObject>(def)
    );
    return prim->obj();
}

RtObject RtStage::createPrim(const RtPath& parentPath, const RtToken& name, const RtToken& typeName, const RtObject def)
{
    PvtPrim* prim = _cast(_ptr)->createPrim(
        *static_cast<PvtPath*>(parentPath._ptr),
        name,
        typeName,
        PvtObject::ptr<PvtObject>(def)
    );
    return prim->obj();
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

RtObject RtStage::getPrimAtPath(const RtPath& path)
{
    PvtPrim* prim = _cast(_ptr)->getPrimAtPath(*static_cast<PvtPath*>(path._ptr));
    return prim ? prim->obj() : RtObject();
}

RtObject RtStage::getRootPrim()
{
    return _cast(_ptr)->getRootPrim()->obj();
}

RtStageIterator RtStage::traverse(RtObjectPredicate predicate)
{
    return RtStageIterator(shared_from_this(), predicate);
}

void RtStage::addReference(RtStagePtr stage)
{
    _cast(_ptr)->addReference(stage);
}

void RtStage::removeReference(const RtToken& name)
{
    _cast(_ptr)->removeReference(name);
}

void RtStage::removeReferences()
{
    _cast(_ptr)->removeReferences();
}

}
