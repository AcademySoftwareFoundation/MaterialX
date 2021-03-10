//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTCOLORMANAGEMENTSYSTEM_H
#define MATERIALX_RTCOLORMANAGEMENTSYSTEM_H

#include <MaterialXRuntime/RtToken.h>

namespace MaterialX
{

class RtColorManagementSystem
{
public:
    RtColorManagementSystem() = default;
    virtual ~RtColorManagementSystem() = default;

    virtual const RtTokenVec& getColorSpaceNames() const = 0;
};

class RtDefaultColorManagementSystem : public RtColorManagementSystem
{
public:
    RtDefaultColorManagementSystem();
    ~RtDefaultColorManagementSystem() override = default;

    const RtTokenVec& getColorSpaceNames() const override;
};

}

#endif

