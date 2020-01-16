//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtPathItem.h>

#include <MaterialXRuntime/Private/PvtObject.h>
#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

RtPathItem::RtPathItem(const RtObject& obj):
    RtApiBase(obj)
{
}

const RtToken& RtPathItem::getName() const
{
    return hnd()->asA<PvtPathItem>()->getName();
}

RtPath RtPathItem::getPath() const
{
    return RtPath(hnd()->obj());
}

RtObject RtPathItem::getParent() const
{
    PvtPrim* parent = hnd()->asA<PvtPathItem>()->getParent();
    return parent ? parent->obj() : RtObject();
}

RtTypedValue* RtPathItem::addMetadata(const RtToken& name, const RtToken& type)
{
    return hnd()->asA<PvtPathItem>()->addMetadata(name, type);
}

void RtPathItem::removeMetadata(const RtToken& name)
{
    hnd()->asA<PvtPathItem>()->removeMetadata(name);
}

RtTypedValue* RtPathItem::getMetadata(const RtToken& name)
{
    return hnd()->asA<PvtPathItem>()->getMetadata(name);
}

}
