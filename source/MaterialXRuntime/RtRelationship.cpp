//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtRelationship.h>

#include <MaterialXRuntime/Private/PvtRelationship.h>

namespace MaterialX
{

RT_DEFINE_RUNTIME_OBJECT(RtRelationship, RtObjType::RELATIONSHIP, "RtRelationship")

RtRelationship::RtRelationship(PvtDataHandle hnd) : 
    RtObject(hnd)
{
}

const RtToken& RtRelationship::getName() const
{
    return hnd()->asA<PvtRelationship>()->getName();
}

bool RtRelationship::hasTargets() const
{
    return hnd()->asA<PvtRelationship>()->hasTargets();
}

size_t RtRelationship::targetCount()  const
{
    return hnd()->asA<PvtRelationship>()->targetCount();
}

void RtRelationship::addTarget(const RtObject& target)
{
    if (!target)
    {
        throw ExceptionRuntimeError("Given target object is not valid");
    }
    return hnd()->asA<PvtRelationship>()->addTarget(PvtObject::ptr<PvtObject>(target));
}

void RtRelationship::removeTarget(const RtObject& target)
{
    if (!target)
    {
        throw ExceptionRuntimeError("Given target object is not valid");
    }
    return hnd()->asA<PvtRelationship>()->removeTarget(PvtObject::ptr<PvtObject>(target));
}

void RtRelationship::clearTargets()
{
    return hnd()->asA<PvtRelationship>()->clearTargets();
}

RtConnectionIterator RtRelationship::getTargets() const
{
    return RtConnectionIterator(*this);
}

string RtRelationship::getTargetsAsString(const string& sep) const
{
    RtConnectionIterator iter = getTargets();
    string str;
    while (!iter.isDone())
    {
        str += (*iter).getName().str();
        iter.operator++();
        if (!iter.isDone())
        {
            str += sep;
        }
    }
    return str;
}

}
