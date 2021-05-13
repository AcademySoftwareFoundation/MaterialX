//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtColorManagementSystem.h>

namespace MaterialX
{

namespace
{
    static const RtStringVec DEFAULT_COLOR_SPACES_NAMES {
        RtString("gamma18"),
        RtString("gamma22"),
        RtString("gamma24"),
        RtString("acescg"),
        RtString("lin_rec709"),
        RtString("srgb_texture"),
        RtString("g22_ap1")
    };
}

RtDefaultColorManagementSystem::RtDefaultColorManagementSystem()
    : RtColorManagementSystem()
{
}

const RtStringVec& RtDefaultColorManagementSystem::getColorSpaceNames() const
{
    return DEFAULT_COLOR_SPACES_NAMES;
}

}
