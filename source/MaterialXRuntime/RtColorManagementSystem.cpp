//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtColorManagementSystem.h>

namespace MaterialX
{

namespace
{
    static const RtTokenVec DEFAULT_COLOR_SPACES_NAMES {
        RtToken("gamma18"),
        RtToken("gamma22"),
        RtToken("gamma24"),
        RtToken("acescg"),
        RtToken("lin_rec709"),
        RtToken("srgb_texture"),
        RtToken("g22_ap1")
    };
}

RtDefaultColorManagementSystem::RtDefaultColorManagementSystem()
    : RtColorManagementSystem()
{
}

const RtTokenVec& RtDefaultColorManagementSystem::getColorSpaceNames() const
{
    return DEFAULT_COLOR_SPACES_NAMES;
}

}
