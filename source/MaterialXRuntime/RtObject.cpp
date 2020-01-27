//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtPath.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

RtObject::RtObject() :
    _hnd(nullptr)
{
}

RtObject::RtObject(const RtObject& other) :
    _hnd(other._hnd)
{
}

RtObject::RtObject(PvtDataHandle data) :
    _hnd(data)
{
}

RtObject::~RtObject()
{
}

RtObjType RtObject::getObjType() const
{
    return _hnd ? _hnd->getObjType() : RtObjType::NONE;
}

const RtToken& RtObject::getName() const
{
    return hnd()->asA<PvtObject>()->getName();
}

RtPath RtObject::getPath() const
{
    return RtPath(hnd()->obj());
}

RtPrim RtObject::getParent() const
{
    PvtPrim* parent = hnd()->asA<PvtObject>()->getParent();
    return parent ? parent->hnd() : RtPrim();
}

RtPrim RtObject::getRoot() const
{
    PvtPrim* root = hnd()->asA<PvtObject>()->getRoot();
    return root ? root->hnd() : RtPrim();
}

RtStageWeakPtr RtObject::getStage() const
{
    return hnd()->asA<PvtObject>()->getStage();
}

RtTypedValue* RtObject::addMetadata(const RtToken& name, const RtToken& type)
{
    return hnd()->asA<PvtObject>()->addMetadata(name, type);
}

void RtObject::removeMetadata(const RtToken& name)
{
    hnd()->asA<PvtObject>()->removeMetadata(name);
}

RtTypedValue* RtObject::getMetadata(const RtToken& name)
{
    return hnd()->asA<PvtObject>()->getMetadata(name);
}


RtSchemaBase::RtSchemaBase(const RtPrim& prim) :
    _hnd(prim.hnd())
{
}

RtSchemaBase::RtSchemaBase(const RtSchemaBase& other) :
    _hnd(other.hnd())
{
}

bool RtSchemaBase::isSupported(const RtPrim& prim) const
{
    return prim && isSupported(prim.hnd());
}

bool RtSchemaBase::isSupported(const PvtDataHandle&) const
{
    return true;
}

RtPrim RtSchemaBase::getPrim() const
{
    return hnd();
}

const RtToken& RtSchemaBase::getName() const
{
    return prim()->getName();
}

PvtPrim* RtSchemaBase::prim() const
{
    return hnd()->asA<PvtPrim>();
}

PvtAttribute* RtSchemaBase::attr(const RtToken& name) const
{
    return prim()->getAttribute(name);
}

PvtRelationship* RtSchemaBase::rel(const RtToken& name) const
{
    return prim()->getRelationship(name);
}


const string RtTypedSchema::TYPE_NAME_SEPARATOR(":");

const RtToken& RtTypedSchema::getTypeName() const
{
    return EMPTY_TOKEN;
}

bool RtTypedSchema::isSupported(const PvtDataHandle& hnd) const
{
    return hnd->asA<PvtPrim>()->getTypeName() == getTypeName();
}

}
