//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtTraversal.h>

#include <MaterialXRuntime/Private/PvtTraversal.h>

namespace MaterialX
{

RtStageIterator::RtStageIterator() :
    RtApiBase(),
    _ptr(new PvtStageIterator())
{
}

RtStageIterator::RtStageIterator(RtObject root, RtTraversalFilter filter) :
    RtApiBase(root),
    _ptr(new PvtStageIterator(PvtObject::data(root), filter))
{
}

RtStageIterator::RtStageIterator(const RtStageIterator& other) :
    RtApiBase(other),
    _ptr(new PvtStageIterator(*static_cast<PvtStageIterator*>(other._ptr)))
{
}

RtStageIterator::~RtStageIterator()
{
    PvtStageIterator* it = static_cast<PvtStageIterator*>(_ptr);
    delete it;
}

RtApiType RtStageIterator::getApiType() const
{
    return RtApiType::STAGE_ITERATOR;
}

bool RtStageIterator::operator==(const RtStageIterator& other) const
{
    PvtStageIterator* lhs = static_cast<PvtStageIterator*>(_ptr);
    PvtStageIterator* rhs = static_cast<PvtStageIterator*>(other._ptr);
    return lhs->operator==(*rhs);
}
bool RtStageIterator::operator!=(const RtStageIterator& other) const
{
    return !(*this == other);
}

RtObject RtStageIterator::operator*() const
{
    PvtStageIterator* it = static_cast<PvtStageIterator*>(_ptr);
    return PvtObject::object(it->operator*());
}

RtStageIterator& RtStageIterator::operator++()
{
    PvtStageIterator* it = static_cast<PvtStageIterator*>(_ptr);
    it->operator++();
    return *this;
}

bool RtStageIterator::isDone() const
{
    PvtStageIterator* it = static_cast<PvtStageIterator*>(_ptr);
    return it->isDone();
}

void RtStageIterator::abort()
{
    PvtStageIterator* it = static_cast<PvtStageIterator*>(_ptr);
    return it->abort();
}


RtTreeIterator::RtTreeIterator() :
    RtApiBase(),
    _ptr(new PvtTreeIterator())
{
}

RtTreeIterator::RtTreeIterator(RtObject root, RtTraversalFilter filter) :
    RtApiBase(root),
    _ptr(new PvtTreeIterator(PvtObject::data(root), filter))
{
}

RtTreeIterator::RtTreeIterator(const RtTreeIterator& other) :
    RtApiBase(other),
    _ptr(new PvtTreeIterator(*static_cast<PvtTreeIterator*>(other._ptr)))
{
}

RtTreeIterator::~RtTreeIterator()
{
    PvtTreeIterator* it = static_cast<PvtTreeIterator*>(_ptr);
    delete it;
}

RtApiType RtTreeIterator::getApiType() const
{
    return RtApiType::TREE_ITERATOR;
}

bool RtTreeIterator::operator==(const RtTreeIterator& other) const
{
    PvtTreeIterator* lhs = static_cast<PvtTreeIterator*>(_ptr);
    PvtTreeIterator* rhs = static_cast<PvtTreeIterator*>(other._ptr);
    return lhs->operator==(*rhs);
}
bool RtTreeIterator::operator!=(const RtTreeIterator& other) const
{
    return !(*this==other);
}

RtObject RtTreeIterator::operator*() const
{
    PvtTreeIterator* it = static_cast<PvtTreeIterator*>(_ptr);
    return PvtObject::object(it->operator*());
}

RtTreeIterator& RtTreeIterator::operator++()
{
    PvtTreeIterator* it = static_cast<PvtTreeIterator*>(_ptr);
    it->operator++();
    return *this;
}

bool RtTreeIterator::isDone() const
{
    PvtTreeIterator* it = static_cast<PvtTreeIterator*>(_ptr);
    return it->isDone();
}

void RtTreeIterator::abort()
{
    PvtTreeIterator* it = static_cast<PvtTreeIterator*>(_ptr);
    return it->abort();
}



RtGraphIterator::RtGraphIterator(RtPort root, RtTraversalFilter filter) :
    RtApiBase(root.data()),
    _ptr(new PvtGraphIterator(root, filter))
{
}

RtGraphIterator::RtGraphIterator(const RtGraphIterator& other) :
    RtApiBase(other),
    _ptr(new PvtGraphIterator(*static_cast<PvtGraphIterator*>(other._ptr)))
{
}

RtGraphIterator::~RtGraphIterator()
{
    PvtGraphIterator* it = static_cast<PvtGraphIterator*>(_ptr);
    delete it;
}

RtApiType RtGraphIterator::getApiType() const
{
    return RtApiType::GRAPH_ITERATOR;
}

bool RtGraphIterator::operator==(const RtGraphIterator& other) const
{
    PvtGraphIterator* lhs = static_cast<PvtGraphIterator*>(_ptr);
    PvtGraphIterator* rhs = static_cast<PvtGraphIterator*>(other._ptr);
    return lhs->operator==(*rhs);
}
bool RtGraphIterator::operator!=(const RtGraphIterator& other) const
{
    return !(*this == other);
}

RtEdge RtGraphIterator::operator*() const
{
    PvtGraphIterator* it = static_cast<PvtGraphIterator*>(_ptr);
    return it->operator*();
}

RtGraphIterator& RtGraphIterator::operator++()
{
    PvtGraphIterator* it = static_cast<PvtGraphIterator*>(_ptr);
    it->operator++();
    return *this;
}

bool RtGraphIterator::isDone() const
{
    PvtGraphIterator* it = static_cast<PvtGraphIterator*>(_ptr);
    return it->isDone();
}

void RtGraphIterator::abort()
{
    PvtGraphIterator* it = static_cast<PvtGraphIterator*>(_ptr);
    return it->abort();
}
}
