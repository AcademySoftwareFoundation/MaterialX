//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtSchema.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

RtSchemaBase::RtSchemaBase(const RtPrim& prim) :
    _hnd(prim._hnd)
{
}

RtSchemaBase::RtSchemaBase(const RtSchemaBase& other) :
    _hnd(other._hnd)
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
    return RtPrim(_hnd);
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


const RtToken& RtTypedSchema::getTypeName() const
{
    return EMPTY_TOKEN;
}

bool RtTypedSchema::isSupported(const PvtDataHandle& hnd) const
{
    // TODO: Implement proper type information and checking
    // to handle schema class inheritance.
    return !hnd->isDisposed() && hnd->asA<PvtPrim>()->getTypeName() == getTypeName();
}

}
