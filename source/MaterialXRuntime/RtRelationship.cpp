//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtRelationship.h>

#include <MaterialXRuntime/Private/PvtRelationship.h>

namespace MaterialX
{

RT_DEFINE_RUNTIME_OBJECT(RtRelationship, RtObjType::RELATIONSHIP, "RtRelationship")

RtRelationship::RtRelationship(PvtObjHandle hnd) : 
    RtObject(hnd)
{
}

const RtToken& RtRelationship::getName() const
{
    return hnd()->asA<PvtRelationship>()->getName();
}

void RtRelationship::connect(const RtObject& obj)
{
    if (!obj)
    {
        throw ExceptionRuntimeError("Given object is not valid");
    }
    return hnd()->asA<PvtRelationship>()->connect(PvtObject::ptr(obj));
}

void RtRelationship::disconnect(const RtObject& obj)
{
    if (!obj)
    {
        throw ExceptionRuntimeError("Given object is not valid");
    }
    return hnd()->asA<PvtRelationship>()->disconnect(PvtObject::ptr(obj));
}

bool RtRelationship::hasConnections() const
{
    return hnd()->asA<PvtRelationship>()->hasConnections();
}

size_t RtRelationship::numConnections()  const
{
    return hnd()->asA<PvtRelationship>()->numConnections();
}

RtObject RtRelationship::getConnection(size_t index) const
{
    return hnd()->asA<PvtRelationship>()->getConnection(index);
}

void RtRelationship::clearConnections()
{
    return hnd()->asA<PvtRelationship>()->clearConnections();
}

RtConnectionIterator RtRelationship::getConnections() const
{
    return RtConnectionIterator(*this);
}

string RtRelationship::getObjectNames() const
{
    string result;
    const string seperator = ",";
    const string* sep = &EMPTY_STRING;
    for (RtObject obj : getConnections())
    {
        result += *sep + obj.getName().str();
        sep = &seperator;
    }
    return result;
}

}
