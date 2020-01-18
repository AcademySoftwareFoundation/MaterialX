//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtTraversal.h>
#include <MaterialXRuntime/RtNode.h>

#include <MaterialXRuntime/Private/PvtPrim.h>
#include <MaterialXRuntime/Private/PvtStage.h>

namespace MaterialX
{

namespace
{
    static const RtAttrIterator NULL_ATTR_ITERATOR;
    static const RtPrimIterator NULL_PRIM_ITERATOR;
    static const RtConnectionIterator NULL_CONNECTION_ITERATOR;
    static const RtStageIterator NULL_STAGE_ITERATOR;
}

RtAttrIterator::RtAttrIterator(const RtObject& prim, RtObjectPredicate predicate) :
    _prim(nullptr),
    _current(0),
    _predicate(predicate)
{
    if (prim.hasApi(RtApiType::PRIM))
    {
        _prim = PvtObject::ptr<PvtPrim>(prim);
        _prim = _prim->getAllAttributes().empty() ? nullptr : _prim;
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
        if (!_predicate || _predicate(_prim->getAllAttributes()[_current]->obj()))
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

RtPrimIterator::RtPrimIterator(const RtObject& prim, RtObjectPredicate predicate) :
    _prim(nullptr),
    _current(0),
    _predicate(predicate)
{
    if (prim.hasApi(RtApiType::PRIM))
    {
        _prim = PvtObject::ptr<PvtPrim>(prim);
        _prim = _prim->getAllChildren().empty() ? nullptr : _prim;
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
        if (!_predicate || _predicate(_prim->getAllChildren()[_current]->obj()))
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
        _ptr = attr->_connections.empty() ? nullptr : &attr->_connections;
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


namespace
{
    using StackFrame = std::tuple<PvtStage*, int, int>;

    struct StageIteratorData
    {
        PvtDataHandle current;
        RtObjectPredicate predicate;
        vector<StackFrame> stack;
    };
}


RtStageIterator::RtStageIterator() :
    _ptr(nullptr)
{
}

RtStageIterator::RtStageIterator(const RtObject& stage, RtObjectPredicate predicate) :
    _ptr(nullptr)
{
    if (stage.hasApi(RtApiType::STAGE))
    {
        PvtStage* s = PvtObject::ptr<PvtStage>(stage);

        // Initialize the stack and start iteration to the first element.
        StageIteratorData* data = new StageIteratorData();
        data->current = nullptr;
        data->predicate = predicate;
        data->stack.push_back(std::make_tuple(s, -1, -1));

        _ptr = data;
        ++*this;
    }
}

RtStageIterator::RtStageIterator(const RtStageIterator& other) :
    _ptr(nullptr)
{
    if (other._ptr)
    {
        StageIteratorData* data = new StageIteratorData();
        *data = *static_cast<StageIteratorData*>(other._ptr);
        _ptr = data;
    }
}

RtStageIterator::~RtStageIterator()
{
    delete static_cast<StageIteratorData*>(_ptr);
}

bool RtStageIterator::operator==(const RtStageIterator& other) const
{
    return _ptr && other._ptr ?
        static_cast<StageIteratorData*>(_ptr)->current == static_cast<StageIteratorData*>(other._ptr)->current :
        _ptr == other._ptr;
}

bool RtStageIterator::operator!=(const RtStageIterator& other) const
{
    return _ptr && other._ptr ?
        static_cast<StageIteratorData*>(_ptr)->current != static_cast<StageIteratorData*>(other._ptr)->current :
        _ptr != other._ptr;
}

RtObject RtStageIterator::operator*() const
{
    PvtDataHandle hnd = static_cast<StageIteratorData*>(_ptr)->current;
    return hnd ? hnd->obj() : RtObject();
}

bool RtStageIterator::isDone() const
{
    return _ptr == nullptr;
}

const RtStageIterator& RtStageIterator::end()
{
    return NULL_STAGE_ITERATOR;
}

RtStageIterator& RtStageIterator::operator++()
{
    while (_ptr)
    {
        StageIteratorData* data = static_cast<StageIteratorData*>(_ptr);

        if (data->stack.empty())
        {
            // Traversal is complete.
            abort();
            return *this;
        }

        StackFrame& frame = data->stack.back();
        PvtStage* stage = std::get<0>(frame);
        int& primIndex = std::get<1>(frame);
        int& stageIndex = std::get<2>(frame);

        bool pop = true;

        if (primIndex + 1 < int(stage->getRootPrim()->getAllChildren().size()))
        {
            data->current = stage->getRootPrim()->getAllChildren()[++primIndex];
            if (!data->predicate || data->predicate(data->current->obj()))
            {
                return *this;
            }
            pop = false;
        }
        else if (stageIndex + 1 < int(stage->getAllReferences().size()))
        {
            PvtStage* refStage = stage->getAllReferences()[++stageIndex]->asA<PvtStage>();
            if (!refStage->getRootPrim()->getAllChildren().empty())
            {
                data->stack.push_back(std::make_tuple(refStage, 0, stageIndex));
                data->current = refStage->getRootPrim()->getAllChildren()[0];
                if (!data->predicate || data->predicate(data->current->obj()))
                {
                    return *this;
                }
                pop = false;
            }
        }

        if (pop)
        {
            data->stack.pop_back();
        }
    }
    return *this;
}

void RtStageIterator::abort()
{
    delete static_cast<StageIteratorData*>(_ptr);
    _ptr = nullptr;
}

}
