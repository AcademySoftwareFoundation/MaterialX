//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtPrim.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

RtPrim::RtPrim(const RtObject& obj) :
    RtPathItem(obj)
{
}

const RtToken& RtPrim::typeName()
{
    return PvtPrim::typeName();
}

RtApiType RtPrim::getApiType() const
{
    return RtApiType::PRIM;
}

const RtToken& RtPrim::getPrimTypeName() const
{
    return hnd()->asA<PvtPrim>()->getPrimTypeName();
}

RtObject RtPrim::getAttribute(const RtToken& name) const
{
    PvtAttribute* attr = hnd()->asA<PvtPrim>()->getAttribute(name);
    return attr ? attr->obj() : RtObject();
}

RtAttrIterator RtPrim::getAttributes(RtObjectPredicate filter) const
{
    return RtAttrIterator(getObject(), filter);
}

RtObject RtPrim::getChild(const RtToken& name) const
{
    PvtPrim* child = hnd()->asA<PvtPrim>()->getChild(name);
    return child ? child->obj() : RtObject();
}

RtPrimIterator RtPrim::getChildren(RtObjectPredicate predicate) const
{
    return RtPrimIterator(getObject(), predicate);
}

}
