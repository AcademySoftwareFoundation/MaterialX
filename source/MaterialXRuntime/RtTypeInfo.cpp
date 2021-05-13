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
    RtString shortName;
    RtString longName;
    RtStringVec baseNames;
    RtStringSet allNames;
};

RtTypeInfo::RtTypeInfo(const char* typeNameHierachy) :
    _ptr(nullptr)
{
    PvtTypeNameInfo* info = new PvtTypeNameInfo();

    const RtString longName(typeNameHierachy);
    StringVec names = splitString(longName.str(), ":");
    info->shortName = RtString(names.back());
    info->longName = longName;
    for (size_t i = 0; i < names.size() - 1; ++i)
    {
        const RtString name(names[i]);
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

const RtString& RtTypeInfo::getShortTypeName() const
{
    return static_cast<PvtTypeNameInfo*>(_ptr)->shortName;
}

const RtString& RtTypeInfo::getLongTypeName() const
{
    return static_cast<PvtTypeNameInfo*>(_ptr)->longName;
}

size_t RtTypeInfo::numBaseClasses() const
{
    return static_cast<PvtTypeNameInfo*>(_ptr)->baseNames.size();
}

const RtString& RtTypeInfo::getBaseClassType(size_t index) const
{
    return static_cast<PvtTypeNameInfo*>(_ptr)->baseNames[index];
}

bool RtTypeInfo::isCompatible(const RtString& typeName) const
{
    return static_cast<PvtTypeNameInfo*>(_ptr)->allNames.count(typeName) > 0;
}

}
