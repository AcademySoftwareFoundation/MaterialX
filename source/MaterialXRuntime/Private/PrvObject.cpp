//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PrvObject.h>

#include <set>

/// @file
/// TODO: Docs

namespace MaterialX
{

namespace
{
    static const std::set<RtApiType> OBJ_TO_API_RTTI[static_cast<int>(RtObjType::NUM_TYPES)] =
    {
        // INVALID
        {
        },
        // PORTDEF
        {
            RtApiType::ELEMENT,
            RtApiType::PORTDEF,
            RtApiType::TREE_ITERATOR
        },
        // NODEDEF
        {
            RtApiType::ELEMENT,
            RtApiType::NODEDEF,
            RtApiType::TREE_ITERATOR
        },
        // NODE
        {
            RtApiType::ELEMENT,
            RtApiType::NODE,
            RtApiType::TREE_ITERATOR
        },
        // NODEGRAPH
        {
            RtApiType::ELEMENT,
            RtApiType::NODE,
            RtApiType::NODEGRAPH,
            RtApiType::TREE_ITERATOR
        },
        // STAGE
        {
            RtApiType::ELEMENT,
            RtApiType::STAGE,
            RtApiType::CORE_IO,
            RtApiType::TREE_ITERATOR,
            RtApiType::STAGE_ITERATOR
        },
        // UNKNOWN
        {
            RtApiType::ELEMENT
        },
    };
}

PrvObject::PrvObject(RtObjType type) :
    _objType(type)
{
}

PrvObject::~PrvObject()
{
}

bool PrvObject::hasApi(RtApiType type) const
{
    return OBJ_TO_API_RTTI[int(_objType)].count(type) != 0;
}

}
