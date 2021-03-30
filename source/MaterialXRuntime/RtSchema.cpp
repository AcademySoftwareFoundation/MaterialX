//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtSchema.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

namespace
{
    static const RtIdentifierVec EMPTY_ATTR_NAMES;
}

RtSchemaBase::RtSchemaBase(const RtPrim& prim) :
    _hnd(prim._hnd)
{
}

RtSchemaBase::RtSchemaBase(const RtSchemaBase& other) :
    _hnd(other._hnd)
{
}

RtPrim RtSchemaBase::getPrim() const
{
    return RtPrim(_hnd);
}

PvtPrim* RtSchemaBase::prim() const
{
    return hnd()->asA<PvtPrim>();
}

PvtRelationship* RtSchemaBase::rel(const RtIdentifier& name) const
{
    return prim()->getRelationship(name);
}


bool RtTypedSchema::isCompatible(const RtPrim& prim) const
{
    return prim && prim.getTypeInfo()->isCompatible(getTypeInfo().getShortTypeName());
}

}
