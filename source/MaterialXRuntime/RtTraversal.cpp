//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtTraversal.h>

#include <MaterialXRuntime/Private/PrvTraversal.h>

namespace MaterialX
{

RtStageIterator::RtStageIterator() :
    RtApiBase(),
    _ptr(new PrvStageIterator())
{
}

RtStageIterator::RtStageIterator(RtObject root, RtTraversalFilter filter) :
    RtApiBase(root),
    _ptr(new PrvStageIterator(root.data(), filter))
{
}

RtStageIterator::RtStageIterator(const RtStageIterator& other) :
    RtApiBase(other),
    _ptr(new PrvStageIterator(*static_cast<PrvStageIterator*>(other._ptr)))
{
}

RtStageIterator::~RtStageIterator()
{
    PrvStageIterator* it = static_cast<PrvStageIterator*>(_ptr);
    delete it;
}

RtApiType RtStageIterator::getApiType() const
{
    return RtApiType::STAGE_ITERATOR;
}

bool RtStageIterator::operator==(const RtStageIterator& other) const
{
    PrvStageIterator* lhs = static_cast<PrvStageIterator*>(_ptr);
    PrvStageIterator* rhs = static_cast<PrvStageIterator*>(other._ptr);
    return lhs->operator==(*rhs);
}
bool RtStageIterator::operator!=(const RtStageIterator& other) const
{
    return !(*this == other);
}

RtObject RtStageIterator::operator*() const
{
    PrvStageIterator* it = static_cast<PrvStageIterator*>(_ptr);
    return RtObject(it->operator*());
}

RtStageIterator& RtStageIterator::operator++()
{
    PrvStageIterator* it = static_cast<PrvStageIterator*>(_ptr);
    it->operator++();
    return *this;
}

bool RtStageIterator::isDone() const
{
    PrvStageIterator* it = static_cast<PrvStageIterator*>(_ptr);
    return it->isDone();
}

void RtStageIterator::abort()
{
    PrvStageIterator* it = static_cast<PrvStageIterator*>(_ptr);
    return it->abort();
}


RtTreeIterator::RtTreeIterator() :
    RtApiBase(),
    _ptr(new PrvTreeIterator())
{
}

RtTreeIterator::RtTreeIterator(RtObject root, RtTraversalFilter filter) :
    RtApiBase(root),
    _ptr(new PrvTreeIterator(root.data(), filter))
{
}

RtTreeIterator::RtTreeIterator(const RtTreeIterator& other) :
    RtApiBase(other),
    _ptr(new PrvTreeIterator(*static_cast<PrvTreeIterator*>(other._ptr)))
{
}

RtTreeIterator::~RtTreeIterator()
{
    PrvTreeIterator* it = static_cast<PrvTreeIterator*>(_ptr);
    delete it;
}

RtApiType RtTreeIterator::getApiType() const
{
    return RtApiType::TREE_ITERATOR;
}

bool RtTreeIterator::operator==(const RtTreeIterator& other) const
{
    PrvTreeIterator* lhs = static_cast<PrvTreeIterator*>(_ptr);
    PrvTreeIterator* rhs = static_cast<PrvTreeIterator*>(other._ptr);
    return lhs->operator==(*rhs);
}
bool RtTreeIterator::operator!=(const RtTreeIterator& other) const
{
    return !(*this==other);
}

RtObject RtTreeIterator::operator*() const
{
    PrvTreeIterator* it = static_cast<PrvTreeIterator*>(_ptr);
    return RtObject(it->operator*());
}

RtTreeIterator& RtTreeIterator::operator++()
{
    PrvTreeIterator* it = static_cast<PrvTreeIterator*>(_ptr);
    it->operator++();
    return *this;
}

bool RtTreeIterator::isDone() const
{
    PrvTreeIterator* it = static_cast<PrvTreeIterator*>(_ptr);
    return it->isDone();
}

void RtTreeIterator::abort()
{
    PrvTreeIterator* it = static_cast<PrvTreeIterator*>(_ptr);
    return it->abort();
}



RtGraphIterator::RtGraphIterator(RtPort root, RtTraversalFilter filter) :
    RtApiBase(root.data()),
    _ptr(new PrvGraphIterator(root, filter))
{
}

RtGraphIterator::RtGraphIterator(const RtGraphIterator& other) :
    RtApiBase(other),
    _ptr(new PrvGraphIterator(*static_cast<PrvGraphIterator*>(other._ptr)))
{
}

RtGraphIterator::~RtGraphIterator()
{
    PrvGraphIterator* it = static_cast<PrvGraphIterator*>(_ptr);
    delete it;
}

RtApiType RtGraphIterator::getApiType() const
{
    return RtApiType::GRAPH_ITERATOR;
}

bool RtGraphIterator::operator==(const RtGraphIterator& other) const
{
    PrvGraphIterator* lhs = static_cast<PrvGraphIterator*>(_ptr);
    PrvGraphIterator* rhs = static_cast<PrvGraphIterator*>(other._ptr);
    return lhs->operator==(*rhs);
}
bool RtGraphIterator::operator!=(const RtGraphIterator& other) const
{
    return !(*this == other);
}

RtEdge RtGraphIterator::operator*() const
{
    PrvGraphIterator* it = static_cast<PrvGraphIterator*>(_ptr);
    return it->operator*();
}

RtGraphIterator& RtGraphIterator::operator++()
{
    PrvGraphIterator* it = static_cast<PrvGraphIterator*>(_ptr);
    it->operator++();
    return *this;
}

bool RtGraphIterator::isDone() const
{
    PrvGraphIterator* it = static_cast<PrvGraphIterator*>(_ptr);
    return it->isDone();
}

void RtGraphIterator::abort()
{
    PrvGraphIterator* it = static_cast<PrvGraphIterator*>(_ptr);
    return it->abort();
}
}
