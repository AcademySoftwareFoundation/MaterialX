//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtTypeInfo.h>

#include <MaterialXCore/Util.h>

namespace MaterialX
{

struct PvtTypeNameInfo
{
    RtToken shortName;
    RtToken longName;
    RtTokenVec baseNames;
    RtTokenSet allNames;
};

RtTypeInfo::RtTypeInfo(const char* typeNameHierachy) :
    _ptr(nullptr)
{
    PvtTypeNameInfo* info = new PvtTypeNameInfo();

    const RtToken longName(typeNameHierachy);
    StringVec names = splitString(longName.str(), ":");
    info->shortName = RtToken(names.back());
    info->longName = longName;
    for (size_t i = 0; i < names.size() - 1; ++i)
    {
        const RtToken name(names[i]);
        info->baseNames.push_back(name);
        info->allNames.insert(name);
    }
    info->allNames.insert(info->shortName);

    _ptr = info;
}

RtTypeInfo::~RtTypeInfo()
{
    delete static_cast<PvtTypeNameInfo*>(_ptr);
}

const RtToken& RtTypeInfo::getShortTypeName() const
{
    return static_cast<PvtTypeNameInfo*>(_ptr)->shortName;
}

const RtToken& RtTypeInfo::getLongTypeName() const
{
    return static_cast<PvtTypeNameInfo*>(_ptr)->longName;
}

size_t RtTypeInfo::numBaseClasses() const
{
    return static_cast<PvtTypeNameInfo*>(_ptr)->baseNames.size();
}

const RtToken& RtTypeInfo::getBaseClassType(size_t index) const
{
    return static_cast<PvtTypeNameInfo*>(_ptr)->baseNames[index];
}

bool RtTypeInfo::isCompatible(const RtToken& typeName) const
{
    return static_cast<PvtTypeNameInfo*>(_ptr)->allNames.count(typeName) > 0;
}

}
