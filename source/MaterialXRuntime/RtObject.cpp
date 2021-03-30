//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtPath.h>
#include <MaterialXRuntime/RtTraversal.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

RT_DEFINE_RUNTIME_OBJECT(RtObject, RtObjType::OBJECT, "object")

RtObject::RtObject() :
    _hnd(nullptr)
{
}

RtObject::RtObject(const RtObject& other) :
    _hnd(other._hnd)
{
}

RtObject::RtObject(PvtObjHandle data) :
    _hnd(data)
{
}

RtObject::~RtObject()
{
}

bool RtObject::isValid() const
{
    return _hnd && !_hnd->isDisposed();
}

const RtIdentifier& RtObject::getName() const
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

RtTypedValue* RtObject::createAttribute(const RtIdentifier& name, const RtIdentifier& type)
{
    return hnd()->asA<PvtObject>()->createAttribute(name, type);
}

void RtObject::removeAttribute(const RtIdentifier& name)
{
    hnd()->asA<PvtObject>()->removeAttribute(name);
}

RtTypedValue* RtObject::getAttribute(const RtIdentifier& name)
{
    return hnd()->asA<PvtObject>()->getAttribute(name);
}

const RtTypedValue* RtObject::getAttribute(const RtIdentifier& name) const
{
    return hnd()->asA<PvtObject>()->getAttribute(name);
}

RtTypedValue* RtObject::getAttribute(const RtIdentifier& name, const RtIdentifier& type)
{
    return hnd()->asA<PvtObject>()->getAttribute(name, type);
}

const RtTypedValue* RtObject::getAttribute(const RtIdentifier& name, const RtIdentifier& type) const
{
    return hnd()->asA<PvtObject>()->getAttribute(name, type);
}

RtAttributeIterator RtObject::getAttributes() const
{
    return RtAttributeIterator(*this);
}

bool RtObject::isCompatible(RtObjType typeId) const
{
    return hnd()->asA<PvtObject>()->isCompatible(typeId);
}

#ifndef NDEBUG
const PvtObjHandle& RtObject::hnd() const
{
    // In debug mode we check for object validity
    // to report any invalid access.
    if (!isValid())
    {
        throw ExceptionRuntimeError("Trying to access an invalid or disposed object");
    }
    return _hnd;
}
#endif

}
