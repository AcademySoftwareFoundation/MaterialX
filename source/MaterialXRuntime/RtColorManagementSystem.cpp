//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtColorManagementSystem.h>

namespace MaterialX
{

namespace
{
    static const RtIdentifierVec DEFAULT_COLOR_SPACES_NAMES {
        RtIdentifier("gamma18"),
        RtIdentifier("gamma22"),
        RtIdentifier("gamma24"),
        RtIdentifier("acescg"),
        RtIdentifier("lin_rec709"),
        RtIdentifier("srgb_texture"),
        RtIdentifier("g22_ap1")
    };
}

RtDefaultColorManagementSystem::RtDefaultColorManagementSystem()
    : RtColorManagementSystem()
{
}

const RtIdentifierVec& RtDefaultColorManagementSystem::getColorSpaceNames() const
{
    return DEFAULT_COLOR_SPACES_NAMES;
}

}
