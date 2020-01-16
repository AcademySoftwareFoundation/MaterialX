//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtTraversal.h>
#include <MaterialXRuntime/RtNode.h>

#include <MaterialXRuntime/Private/PvtTraversal.h>
#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

const RtAttrIterator NULL_ATTR_ITERATOR;
const RtPrimIterator NULL_PRIM_ITERATOR;
const RtConnectionIterator NULL_CONNECTION_ITERATOR;

RtAttrIterator::RtAttrIterator(const RtObject& prim, RtObjectPredicate filter) :
    _prim(nullptr),
    _current(0),
    _filter(filter)
{
    if (prim.hasApi(RtApiType::PRIM))
    {
        _prim = PvtObject::ptr<PvtPrim>(prim);
    }
}

RtObject RtAttrIterator::operator*() const
{
    return _prim->getAllAttributes()[_current]->obj();
}

RtAttrIterator& RtAttrIterator::operator++()
{
    while (_prim && ++_current < _prim->getAllAttributes().size())
    {
        if (!_filter || _filter(_prim->getAllAttributes()[_current]->obj()))
        {
            return *this;
        }
    }
    abort();
    return *this;
}

bool RtAttrIterator::isDone() const
{
    return !(_prim && _current < _prim->getAllAttributes().size());
}

const RtAttrIterator& RtAttrIterator::end()
{
    return NULL_ATTR_ITERATOR;
}


RtPrimIterator::RtPrimIterator(const RtObject& prim, RtObjectPredicate filter) :
    _prim(nullptr),
    _current(0),
    _filter(filter)
{
    if (prim.hasApi(RtApiType::PRIM))
    {
        _prim = PvtObject::ptr<PvtPrim>(prim);
    }
}

RtObject RtPrimIterator::operator*() const
{
    return _prim->getAllChildren()[_current]->obj();
}

RtPrimIterator& RtPrimIterator::operator++()
{
    while (_prim && ++_current < _prim->getAllChildren().size())
    {
        if (!_filter || _filter(_prim->getAllChildren()[_current]->obj()))
        {
            return *this;
        }
    }
    abort();
    return *this;
}

bool RtPrimIterator::isDone() const
{
    return !(_prim && _current < _prim->getAllChildren().size());
}

const RtPrimIterator& RtPrimIterator::end()
{
    return NULL_PRIM_ITERATOR;
}

RtConnectionIterator::RtConnectionIterator(const RtObject& attrObj) :
    _ptr(nullptr),
    _current(0)
{
    if (attrObj.hasApi(RtApiType::ATTRIBUTE))
    {
        PvtAttribute* attr = PvtObject::ptr<PvtAttribute>(attrObj);
        _ptr = &attr->_connections;
    }
}

RtInput RtConnectionIterator::operator*() const
{
    PvtConnectionData& data = *static_cast<PvtConnectionData*>(_ptr);
    return RtInput(data[_current]->obj());
}

RtConnectionIterator& RtConnectionIterator::operator++()
{
    if (_ptr && ++_current < static_cast<PvtConnectionData*>(_ptr)->size())
    {
        return *this;
    }
    abort();
    return *this;
}

bool RtConnectionIterator::isDone() const
{
    return !(_ptr && _current < static_cast<PvtConnectionData*>(_ptr)->size());
}

const RtConnectionIterator& RtConnectionIterator::end()
{
    return NULL_CONNECTION_ITERATOR;
}

}
