//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtObject.h>

#include <set>

/// @file
/// TODO: Docs

namespace MaterialX
{

namespace
{
    static const std::set<RtApiType> OBJ_TO_API_RTTI[static_cast<int>(RtObjType::NUM_TYPES)] =
    {
        // NONE
        {
        },
        // PRIM
        {
            RtApiType::PATH_ITEM,
            RtApiType::PRIM
        },
        // RELATIONSHIP
        {
            RtApiType::PATH_ITEM,
            RtApiType::RELATIONSHIP
        },
        // ATTRIBUTE
        {
            RtApiType::PATH_ITEM,
            RtApiType::ATTRIBUTE,
            RtApiType::INPUT,
            RtApiType::OUTPUT
        },
        // NODEDEF
        {
            RtApiType::PATH_ITEM,
            RtApiType::PRIM,
            RtApiType::NODEDEF
        },
        // NODE
        {
            RtApiType::PATH_ITEM,
            RtApiType::PRIM,
            RtApiType::NODE
        },
        // NODEGRAPH
        {
            RtApiType::PATH_ITEM,
            RtApiType::PRIM,
            RtApiType::NODE,
            RtApiType::NODEGRAPH
        },
        // STAGE
        {
            RtApiType::STAGE,
            RtApiType::FILE_IO
        },
    };
}

bool PvtObject::hasApi(RtApiType type) const
{
    return OBJ_TO_API_RTTI[int(getObjType())].count(type) != 0;
}

}
