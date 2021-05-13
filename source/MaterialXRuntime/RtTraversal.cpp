//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtTraversal.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtStage.h>

#include <MaterialXRuntime/Private/PvtPort.h>
#include <MaterialXRuntime/Private/PvtRelationship.h>
#include <MaterialXRuntime/Private/PvtPrim.h>
#include <MaterialXRuntime/Private/PvtStage.h>

namespace MaterialX
{

namespace
{

using StageIteratorStackFrame = std::tuple<PvtStage*, int, int>;

struct StageIteratorData
{
    PvtObjHandle current;
    RtObjectPredicate predicate;
    vector<StageIteratorStackFrame> stack;
};

struct AttrIteratorData
{
    PvtObjHandle obj;
    size_t index;
    AttrIteratorData() : obj(nullptr), index(0) {}
    explicit AttrIteratorData(const PvtObjHandle& o) : obj(o), index(0) {}
};

}

RtAttributeIterator::RtAttributeIterator() :
    _ptr(nullptr)
{
}

RtAttributeIterator::RtAttributeIterator(const RtObject& obj) :
    _ptr(new AttrIteratorData(PvtObject::hnd(obj)))
{
}

RtAttributeIterator::RtAttributeIterator(const RtAttributeIterator& other) :
    _ptr(nullptr)
{
    if (other._ptr)
    {
        _ptr = new AttrIteratorData();
        *static_cast<AttrIteratorData*>(_ptr) = *static_cast<AttrIteratorData*>(other._ptr);
    }
}

RtAttributeIterator& RtAttributeIterator::operator=(const RtAttributeIterator& other)
{
    if (other._ptr)
    {
        if (!_ptr)
        {
            _ptr = new AttrIteratorData();
        }
        *static_cast<AttrIteratorData*>(_ptr) = *static_cast<AttrIteratorData*>(other._ptr);
    }
    return *this;
}

RtAttributeIterator::~RtAttributeIterator()
{
    delete static_cast<AttrIteratorData*>(_ptr);
}

bool RtAttributeIterator::operator==(const RtAttributeIterator& other) const
{
    AttrIteratorData* data1 = static_cast<AttrIteratorData*>(_ptr);
    AttrIteratorData* data2 = static_cast<AttrIteratorData*>(other._ptr);
    return data1 && data2 ?
        data1->obj == data2->obj && data1->index == data2->index :
        data1 == data2;
}

RtAttribute RtAttributeIterator::operator*() const
{
    AttrIteratorData* data = static_cast<AttrIteratorData*>(_ptr);
    PvtObject* obj = data->obj->asA<PvtObject>();
    const RtString& name = obj->_attrNames[data->index];
    return RtAttribute(name, obj->getAttribute(name));
}

bool RtAttributeIterator::isDone() const
{
    return _ptr == nullptr;
}

const RtAttributeIterator& RtAttributeIterator::end()
{
    static const RtAttributeIterator NULL_ITERATOR;
    return NULL_ITERATOR;
}

RtAttributeIterator& RtAttributeIterator::operator++()
{
    AttrIteratorData* data = static_cast<AttrIteratorData*>(_ptr);
    PvtObject* obj = data->obj->asA<PvtObject>();
    if (++data->index >= obj->_attrNames.size())
    {
        abort();
    }
    return *this;
}

void RtAttributeIterator::abort()
{
    delete static_cast<AttrIteratorData*>(_ptr);
    _ptr = nullptr;
}


template<class T>
T RtObjectIterator<T>::operator*() const
{
    const PvtObjectVec& vec = *static_cast<PvtObjectVec*>(_ptr);
    return T(vec[_current]->hnd());
}

template<class T>
RtObjectIterator<T>& RtObjectIterator<T>::operator++()
{
    const PvtObjectVec& vec = *static_cast<PvtObjectVec*>(_ptr);
    while (_ptr && ++_current < int(vec.size()))
    {
        if (!_predicate || _predicate(vec[_current]->obj()))
        {
            return *this;
        }
    }
    abort();
    return *this;
}

template<class T>
bool RtObjectIterator<T>::isDone() const
{
    return !(_ptr && _current < int(static_cast<PvtObjectVec*>(_ptr)->size()));
}

template<class T>
const RtObjectIterator<T>& RtObjectIterator<T>::end()
{
    static const RtObjectIterator<T> NULL_ITERATOR;
    return NULL_ITERATOR;
}

template class RtObjectIterator<RtObject>;
template class RtObjectIterator<RtPrim>;
template class RtObjectIterator<RtInput>;
template class RtObjectIterator<RtOutput>;
template class RtObjectIterator<RtRelationship>;


RtPrimIterator::RtPrimIterator(const RtObject& obj, RtObjectPredicate predicate) :
    RtObjectIterator(predicate)
{
    if (obj.isA<RtPrim>())
    {
        PvtPrim* prim = PvtObject::cast<PvtPrim>(obj);
        _ptr = prim->_prims.empty() ? nullptr : &prim->_prims._vec;
    }
    ++*this;
}

RtInputIterator::RtInputIterator(const RtObject& obj) :
    RtObjectIterator()
{
    if (obj.isA<RtPrim>())
    {
        PvtPrim* prim = PvtObject::cast<PvtPrim>(obj);
        _ptr = prim->_inputs.empty() ? nullptr : &prim->_inputs._vec;
    }
    else if (obj.isA<RtOutput>())
    {
        PvtOutput* output = PvtObject::cast<PvtOutput>(obj);
        _ptr = output->_connections.empty() ? nullptr : &output->_connections;
    }
    ++*this;
}

RtOutputIterator::RtOutputIterator(const RtObject& obj) :
    RtObjectIterator()
{
    if (obj.isA<RtPrim>())
    {
        PvtPrim* prim = PvtObject::cast<PvtPrim>(obj);
        _ptr = prim->_outputs.empty() ? nullptr : &prim->_outputs._vec;
    }
    ++*this;
}

RtRelationshipIterator::RtRelationshipIterator(const RtObject& obj) :
    RtObjectIterator()
{
    if (obj.isA<RtPrim>())
    {
        PvtPrim* prim = PvtObject::cast<PvtPrim>(obj);
        _ptr = prim->_rel.empty() ? nullptr : &prim->_rel._vec;
    }
    ++*this;
}


RtConnectionIterator::RtConnectionIterator() :
    _ptr(nullptr),
    _current(-1)
{
}

RtConnectionIterator::RtConnectionIterator(const RtObject& obj) :
    _ptr(nullptr),
    _current(-1)
{
    if (obj.isA<RtOutput>())
    {
        PvtOutput* out = PvtObject::cast<PvtOutput>(obj);
        _ptr = out->_connections.empty() ? nullptr : &out->_connections;
    }
    else if (obj.isA<RtRelationship>())
    {
        PvtRelationship* rel = PvtObject::cast<PvtRelationship>(obj);
        _ptr = rel->_connections.empty() ? nullptr : &rel->_connections;
    }
    ++*this;
}

RtObject RtConnectionIterator::operator*() const
{
    const PvtObjHandleVec& vec = *static_cast<PvtObjHandleVec*>(_ptr);
    return vec[_current];
}

RtConnectionIterator& RtConnectionIterator::operator++()
{
    if (!(_ptr && ++_current < int(static_cast<PvtObjHandleVec*>(_ptr)->size())))
    {
        abort();
    }
    return *this;
}

bool RtConnectionIterator::isDone() const
{
    return !(_ptr && _current < int(static_cast<PvtObjHandleVec*>(_ptr)->size()));
}

const RtConnectionIterator& RtConnectionIterator::end()
{
    static const RtConnectionIterator NULL_ITERATOR;
    return NULL_ITERATOR;
}


RtStageIterator::RtStageIterator() :
    _ptr(nullptr)
{
}

RtStageIterator::RtStageIterator(const RtStagePtr& stage, RtObjectPredicate predicate) :
    _ptr(nullptr)
{
    // Initialize the stack and start iteration to the first element.
    StageIteratorData* data = new StageIteratorData();
    data->current = nullptr;
    data->predicate = predicate;
    data->stack.push_back(std::make_tuple(PvtStage::cast(stage), -1, -1));

    _ptr = data;
    ++*this;
}

RtStageIterator::RtStageIterator(const RtStageIterator& other) :
    _ptr(nullptr)
{
    if (other._ptr)
    {
        _ptr = new StageIteratorData();
        *static_cast<StageIteratorData*>(_ptr) = *static_cast<StageIteratorData*>(other._ptr);
    }
}

RtStageIterator& RtStageIterator::operator=(const RtStageIterator& other)
{
    if (other._ptr)
    {
        if (!_ptr)
        {
            _ptr = new StageIteratorData();
        }
        *static_cast<StageIteratorData*>(_ptr) = *static_cast<StageIteratorData*>(other._ptr);
    }
    return *this;
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

RtPrim RtStageIterator::operator*() const
{
    return static_cast<StageIteratorData*>(_ptr)->current;
}

bool RtStageIterator::isDone() const
{
    return _ptr == nullptr;
}

const RtStageIterator& RtStageIterator::end()
{
    static const RtStageIterator NULL_ITERATOR;
    return NULL_ITERATOR;
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

        StageIteratorStackFrame& frame = data->stack.back();
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
            PvtStage* refStage = PvtStage::cast(stage->getAllReferences()[++stageIndex]);
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
