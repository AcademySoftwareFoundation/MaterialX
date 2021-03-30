//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtNameResolver.h>

#include <MaterialXRuntime/Library.h>

namespace MaterialX
{

/// Shared pointer to an PvtUserStringResolver
using PvtUserStringResolverPtr = std::shared_ptr<class PvtUserStringResolver>;

// Internal class to hold on to user specified resolving information
class PvtUserStringResolver : public StringResolver
{
public:
    static PvtUserStringResolverPtr createNew(RtNameResolverFunction f)
    {
        PvtUserStringResolverPtr result(new PvtUserStringResolver(f));
        return result;
    }

    // Override resolve method
    std::string resolve(const std::string& str, const std::string& type) const override
    {
        std::string result = str;
        // Use resolve function if defined
        if (_resolveFunction)
        {
            result = _resolveFunction(RtIdentifier(str.c_str()), RtIdentifier(type.c_str())).str();
        }

        // Call the parent class to perform any additional resolving
        // including identifier replacement.
        result = StringResolver::resolve(result, type);
        return result;
    }

private:
    RtNameResolverFunction _resolveFunction;

    explicit PvtUserStringResolver(RtNameResolverFunction f) :
        StringResolver(),
        _resolveFunction(f)
    {
    }
};

PvtStringResolverPair::PvtStringResolverPair(const RtNameResolverInfo::ElementType elementType,
                                             StringResolverPtr toMaterialXResolver,
                                             StringResolverPtr fromMaterialXResolver)
    : _elementType(elementType)
    , _toMaterialXResolver(toMaterialXResolver)
    , _fromMaterialXResolver(fromMaterialXResolver)
{
}

PvtStringResolverPairPtr PvtStringResolverPair::createNew(const RtNameResolverInfo::ElementType elementType,
                                                          StringResolverPtr toMaterialXResolver,
                                                          StringResolverPtr fromMaterialXResolver)
{
    PvtStringResolverPairPtr result(new PvtStringResolverPair(elementType, toMaterialXResolver, fromMaterialXResolver));
    return result;
}

const RtNameResolverInfo::ElementType PvtStringResolverPair::getType() const
{
    return _elementType;
}

StringResolverPtr PvtStringResolverPair::getToMaterialXResolver()
{
    return _toMaterialXResolver;
}

StringResolverPtr PvtStringResolverPair::getFromMaterialXResolver()
{
    return _fromMaterialXResolver;
}

PvtNameResolverRegistry::PvtNameResolverRegistry()
{
}

void PvtNameResolverRegistry::registerNameResolvers(RtNameResolverInfo& info)
{
    // Resolvers with the same identifier are not allowed. Fail
    // if this is attempted.
    if (_resolvers.find(info.identifier) != _resolvers.end())
    {
        throw ExceptionRuntimeError("Name resolver for identifier: '" + info.identifier.str() + "' already exists");
    }

    PvtStringResolverPairPtr resolverPair = nullptr;
    PvtUserStringResolverPtr toResolver = nullptr;
    PvtUserStringResolverPtr fromResolver = nullptr;
    RtIdentifier elementTypeString("");

    if (info.toFunction || info.toSubstitutions.size())
    {
        // Set custom function
        toResolver = PvtUserStringResolver::createNew(info.toFunction);

        // Set custom token substitutions
        for (auto& toSubsitution : info.toSubstitutions)
        {
            if (info.elementType == RtNameResolverInfo::FILENAME_TYPE)
            {
                toResolver->setFilenameSubstitution(toSubsitution.first.str(), toSubsitution.second.str());
            }
            else if (info.elementType == RtNameResolverInfo::GEOMNAME_TYPE)
            {
                toResolver->setGeomNameSubstitution(toSubsitution.first.str(), toSubsitution.second.str());
            }
        }
    }
    if (info.fromFunction || info.fromSubstitutions.size())
    {
        // Set custom function
        fromResolver = PvtUserStringResolver::createNew(info.fromFunction);

        // Set custom token substitutions
        for (auto& fromSubstitution : info.fromSubstitutions)
        {
            if (info.elementType == RtNameResolverInfo::FILENAME_TYPE)
            {
                fromResolver->setFilenameSubstitution(fromSubstitution.first.str(), fromSubstitution.second.str());
            }
            else if (info.elementType == RtNameResolverInfo::GEOMNAME_TYPE)
            {
                fromResolver->setGeomNameSubstitution(fromSubstitution.first.str(), fromSubstitution.second.str());
            }
        }
    }

    resolverPair = PvtStringResolverPair::createNew(info.elementType, toResolver, fromResolver);
    _resolvers[info.identifier] = resolverPair;
}

void PvtNameResolverRegistry::deregisterNameResolvers(const RtIdentifier& name)
{
    _resolvers.erase(name);
}

RtIdentifier PvtNameResolverRegistry::resolveIdentifier(const RtIdentifier& valueToResolve, const RtNameResolverInfo::ElementType elementType, bool toMaterialX) const
{
    RtIdentifier type(GEOMNAME_TYPE_STRING);
    if (elementType == RtNameResolverInfo::ElementType::FILENAME_TYPE)
    {
        type = RtIdentifier(FILENAME_TYPE_STRING);
    }
    
    RtIdentifier result = valueToResolve;
    for (const auto resolverPair : _resolvers)
    {
        if (resolverPair.second && resolverPair.second->getType() == elementType)
        {
            if (toMaterialX)
            {
                if (resolverPair.second->getToMaterialXResolver())
                {
                    PvtUserStringResolverPtr resolverPtr = std::dynamic_pointer_cast<PvtUserStringResolver>(resolverPair.second->getToMaterialXResolver());
                    if (resolverPtr)
                    {
                        result = RtIdentifier(resolverPtr->resolve(result.str(), type.str()));
                    }
                }
            }
            else
            {
                if (resolverPair.second->getFromMaterialXResolver())
                {
                    PvtUserStringResolverPtr resolverPtr = std::dynamic_pointer_cast<PvtUserStringResolver>(resolverPair.second->getFromMaterialXResolver());
                    if (resolverPtr)
                    {
                        result = RtIdentifier(resolverPtr->resolve(result.str(), type.str()));
                    }
                }
            }
        }
    }
    return result;
}

}
