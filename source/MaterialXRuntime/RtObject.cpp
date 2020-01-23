//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/Private/PvtObject.h>

namespace MaterialX
{

namespace
{
    static const std::set<RtObjType> API_TO_OBJ_RTTI[static_cast<int>(RtApiType::NUM_TYPES)] =
    {
        // RtApiType::PATH_ITEM
        {
            RtObjType::RELATIONSHIP,
            RtObjType::ATTRIBUTE,
            RtObjType::PRIM,
            RtObjType::NODEDEF,
            RtObjType::NODE,
            RtObjType::NODEGRAPH,
            RtObjType::BACKDROP
        },
        // RtApiType::PRIM
        {
            RtObjType::PRIM,
            RtObjType::NODEDEF,
            RtObjType::NODE,
            RtObjType::NODEGRAPH,
            RtObjType::BACKDROP
        },
        // RtApiType::RELATIONSHIP
        {
            RtObjType::RELATIONSHIP
        },
        // RtApiType::ATTRIBUTE
        {
            RtObjType::ATTRIBUTE
        },
        // RtApiType::INPUT
        {
            RtObjType::ATTRIBUTE
        },
        // RtApiType::OUTPUT
        {
            RtObjType::ATTRIBUTE
        },
        // RtApiType::NODEDEF
        {
            RtObjType::NODEDEF
        },
        // RtApiType::NODE
        {
            RtObjType::NODE,
            RtObjType::NODEGRAPH
        },
        // RtApiType::NODEGRAPH
        {
            RtObjType::NODEGRAPH
        },
        // RtApiType::BACKDROP
        {
            RtObjType::BACKDROP
        }
    };

    static RtToken NONE_TYPE_NAME("none");
}

RtObject::RtObject() :
    _hnd(nullptr)
{
}

RtObject::RtObject(const RtObject& other) :
    _hnd(other._hnd)
{
}

RtObject::RtObject(PvtDataHandle data) :
    _hnd(data)
{
}

RtObject::~RtObject()
{
}

bool RtObject::isValid() const
{
    return _hnd != nullptr;
}

RtObjType RtObject::getObjType() const
{
    return _hnd ? _hnd->getObjType() : RtObjType::NONE;
}

const RtToken& RtObject::getObjTypeName() const
{
    return _hnd ? _hnd->getObjTypeName() : NONE_TYPE_NAME;
}

bool RtObject::hasApi(RtApiType type) const
{
    return _hnd && _hnd->hasApi(type);
}


RtApiBase::RtApiBase(PvtDataHandle data) :
    _hnd(data)
{
}

RtApiBase::RtApiBase(const RtObject& obj) :
    _hnd(obj.hnd())
{
}

RtApiBase::RtApiBase(const RtApiBase& other) :
    _hnd(other.hnd())
{
}

bool RtApiBase::isSupported(RtObjType type) const
{
    return API_TO_OBJ_RTTI[(int)getApiType()].count(type) != 0;
}

void RtApiBase::setObject(const RtObject& obj)
{
    setHnd(obj.hnd());
}

RtObject RtApiBase::getObject() const
{
    return RtObject(_hnd);
}

bool RtApiBase::isValid() const
{
    return _hnd && isSupported(_hnd->getObjType());
}

void RtApiBase::setHnd(PvtDataHandle hnd)
{
    if (hnd && !isSupported(hnd->getObjType()))
    {
        throw ExceptionRuntimeError("Given object is not supported by this API");
    }
    _hnd = hnd;
}

}
