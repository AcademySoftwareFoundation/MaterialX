//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtSchema.h>
#include <MaterialXRuntime/RtTraversal.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

RT_DEFINE_RUNTIME_OBJECT(RtPrim, RtObjType::PRIM, "RtPrim")

RtPrim::RtPrim(PvtDataHandle hnd) :
    RtObject(hnd)
{
}

const RtTypeInfo* RtPrim::getTypeInfo() const
{
    return hnd()->asA<PvtPrim>()->getTypeInfo();
}

RtRelationship RtPrim::createRelationship(const RtToken& name)
{
    return hnd()->asA<PvtPrim>()->createRelationship(name)->hnd();
}

void RtPrim::removeRelationship(const RtToken& name)
{
    return hnd()->asA<PvtPrim>()->removeRelationship(name);
}

RtRelationship RtPrim::getRelationship(const RtToken& name) const
{
    PvtRelationship* rel = hnd()->asA<PvtPrim>()->getRelationship(name);
    return rel ? rel->hnd() : RtRelationship();
}

RtRelationshipIterator RtPrim::getRelationships() const
{
    return RtRelationshipIterator(*this);
}

RtAttribute RtPrim::createAttribute(const RtToken& name, const RtToken& type, uint32_t flags)
{
    return hnd()->asA<PvtPrim>()->createAttribute(name, type, flags)->hnd();
}

RtInput RtPrim::createInput(const RtToken& name, const RtToken& type, uint32_t flags)
{
    return hnd()->asA<PvtPrim>()->createInput(name, type, flags)->hnd();
}

RtOutput RtPrim::createOutput(const RtToken& name, const RtToken& type, uint32_t flags)
{
    return hnd()->asA<PvtPrim>()->createOutput(name, type, flags)->hnd();
}

void RtPrim::removeAttribute(const RtToken& name)
{
    return hnd()->asA<PvtPrim>()->removeAttribute(name);
}

RtAttribute RtPrim::getAttribute(const RtToken& name) const
{
    PvtAttribute* attr = hnd()->asA<PvtPrim>()->getAttribute(name);
    return attr ? attr->hnd() : RtAttribute();
}

RtInput RtPrim::getInput(const RtToken& name) const
{
    PvtInput* input = hnd()->asA<PvtPrim>()->getInput(name);
    return input ? input->hnd() : RtInput();
}

RtOutput RtPrim::getOutput(const RtToken& name) const
{
    PvtOutput* input = hnd()->asA<PvtPrim>()->getOutput(name);
    return input ? input->hnd() : RtOutput();
}

RtAttrIterator RtPrim::getAttributes(RtObjectPredicate filter) const
{
    return RtAttrIterator(*this, filter);
}

RtPrim RtPrim::getChild(const RtToken& name) const
{
    PvtPrim* child = hnd()->asA<PvtPrim>()->getChild(name);
    return child ? child->hnd() : RtPrim();
}

RtPrimIterator RtPrim::getChildren(RtObjectPredicate predicate) const
{
    return RtPrimIterator(*this, predicate);
}

}
