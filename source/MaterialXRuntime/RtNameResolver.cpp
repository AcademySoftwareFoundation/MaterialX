//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNameResolver.h>

#include <MaterialXRuntime/Private/PvtNameResolver.h>

namespace MaterialX
{

RtNameResolverRegistryPtr RtNameResolverRegistry::createNew()
{
    RtNameResolverRegistryPtr result(new RtNameResolverRegistry());
    return result;
}

RtNameResolverRegistry::RtNameResolverRegistry()
    : _nameResolverRegistry(new PvtNameResolverRegistry())
{
}

void RtNameResolverRegistry::registerNameResolvers(RtNameResolverInfo& info)
{
    _nameResolverRegistry->registerNameResolvers(info);
}

void RtNameResolverRegistry::deregisterNameResolvers(const RtIdentifier& name)
{
    _nameResolverRegistry->deregisterNameResolvers(name);
}

RtIdentifier RtNameResolverRegistry::resolveIdentifier(const RtIdentifier& valueToResolve, const RtNameResolverInfo::ElementType elementType, bool toMaterialX) const
{
    return _nameResolverRegistry->resolveIdentifier(valueToResolve, elementType, toMaterialX);
}

}
