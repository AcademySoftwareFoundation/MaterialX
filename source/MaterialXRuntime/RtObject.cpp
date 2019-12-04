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
        // RtApiType::ELEMENT
        {
            RtObjType::PORTDEF,
            RtObjType::NODEDEF,
            RtObjType::NODE,
            RtObjType::NODEGRAPH,
            RtObjType::STAGE
        },
        // RtApiType::PORTDEF
        {
            RtObjType::PORTDEF
        },
        // RtApiType::NODEDEF
        {
            RtObjType::NODEDEF
        },
        // RtApiType::NODE
        {
            RtObjType::NODE
        },
        // RtApiType::NODEGRAPH
        {
            RtObjType::NODEGRAPH
        },
        // RtApiType::STAGE
        {
            RtObjType::STAGE
        },
        // RtApiType::CORE_IO
        {
            RtObjType::STAGE
        },
        // RtApiType::STAGE_ITERATOR
        {
            RtObjType::STAGE
        },
        // RtApiType::TREE_ITERATOR
        {
            RtObjType::PORTDEF,
            RtObjType::NODEDEF,
            RtObjType::NODE,
            RtObjType::NODEGRAPH,
            RtObjType::STAGE
        },
        // RtApiType::GRAPH_ITERATOR
        {
        }
    };
}

const RtObject RtObject::NULL_OBJECT(nullptr);

RtObject::RtObject() :
    _data(PvtDataHandle())
{
}

RtObject::RtObject(const RtObject& other) :
    _data(other._data)
{
}

RtObject::RtObject(PvtDataHandle data) :
    _data(data)
{
}

RtObject::~RtObject()
{
}

bool RtObject::isValid() const
{
    return _data && _data->getObjType() != RtObjType::INVALID;
}

RtObjType RtObject::getObjType() const
{
    return _data ? _data->getObjType() : RtObjType::INVALID;
}

bool RtObject::hasApi(RtApiType type) const
{
    return _data && _data->hasApi(type);
}


RtApiBase::RtApiBase() :
    _data(PvtDataHandle())
{
}

RtApiBase::RtApiBase(PvtDataHandle data) :
    _data(data)
{
}

RtApiBase::RtApiBase(const RtObject& obj) :
    _data(obj.data())
{
}

RtApiBase::RtApiBase(const RtApiBase& other) :
    _data(other.data())
{
}

bool RtApiBase::isSupported(RtObjType type) const
{
    return API_TO_OBJ_RTTI[(int)getApiType()].count(type) != 0;
}

void RtApiBase::setObject(const RtObject& obj)
{
    setData(obj.data());
}

RtObject RtApiBase::getObject()
{
    return RtObject(_data);
}

bool RtApiBase::isValid() const
{
    return _data && isSupported(_data->getObjType());
}

void RtApiBase::setData(PvtDataHandle data)
{
    if (data && !isSupported(data->getObjType()))
    {
        throw ExceptionRuntimeError("Given object is not supported by this API");
    }
    _data = data;
}

}
