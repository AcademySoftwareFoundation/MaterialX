//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtRelationship.h>

#include <MaterialXRuntime/Private/PvtRelationship.h>

namespace MaterialX
{

RtRelationship::RtRelationship(const RtObject& obj) : 
    RtPathItem(obj)
{
}

RtApiType RtRelationship::getApiType() const
{
    return RtApiType::RELATIONSHIP;
}

const RtToken& RtRelationship::getName() const
{
    return hnd()->asA<PvtRelationship>()->getName();
}

bool RtRelationship::hasTargets() const
{
    return hnd()->asA<PvtRelationship>()->hasTargets();
}

void RtRelationship::addTarget(const RtObject& target)
{
    if (!target)
    {
        throw ExceptionRuntimeError("Given target object is not valid");
    }
    return hnd()->asA<PvtRelationship>()->addTarget(PvtObject::ptr<PvtPathItem>(target));
}

void RtRelationship::removeTarget(const RtObject& target)
{
    if (!target)
    {
        throw ExceptionRuntimeError("Given target object is not valid");
    }
    return hnd()->asA<PvtRelationship>()->removeTarget(PvtObject::ptr<PvtPathItem>(target));
}

void RtRelationship::clearTargets()
{
    return hnd()->asA<PvtRelationship>()->clearTargets();
}

RtConnectionIterator RtRelationship::getTargets() const
{
    return hnd()->asA<PvtRelationship>()->getTargets();
}

}
