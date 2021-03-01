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
    static const RtTokenVec PUBLIC_EMPTY_METADATA_NAMES;
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

PvtAttribute* RtSchemaBase::attr(const RtToken& name) const
{
    return prim()->getAttribute(name);
}

PvtRelationship* RtSchemaBase::rel(const RtToken& name) const
{
    return prim()->getRelationship(name);
}


bool RtTypedSchema::isCompatible(const RtPrim& prim) const
{
    return prim && prim.getTypeInfo()->isCompatible(getTypeInfo().getShortTypeName());
}


const RtTokenVec& RtSchemaBase::getPublicMetadataNames() const
{
    return PUBLIC_EMPTY_METADATA_NAMES;
}

const RtTokenVec& RtSchemaBase::getPublicPortMetadataNames(const RtToken&) const
{
    return PUBLIC_EMPTY_METADATA_NAMES;
}

}

