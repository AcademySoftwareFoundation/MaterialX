//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTNAMERESOLVER_H
#define MATERIALX_RTNAMERESOLVER_H

#include <MaterialXRuntime/RtToken.h>

#include <MaterialXCore/Element.h>

#include <memory>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PvtNameResolverRegistry;

/// Shared pointer to an RtNameResolverRegistry
using RtNameResolverRegistryPtr = std::shared_ptr<class RtNameResolverRegistry>;

/// Function for resolving a given token of a given type
typedef RtToken (*RtNameResolverFunction)(const RtToken& str, const RtToken& type);

/// @class RtNameResolverInfo
///
/// \brief A structure containing information for custom name resolving
/// to MaterialX or from MaterialX.
///
/// In the structure it is possible to specify a function to call to perform
/// token changes. Additionally it is possible to supply a set of token
/// replacements. This is for both to and from MaterialX resolution.
///
struct RtNameResolverInfo
{
    /// Element type enumeration
    enum ElementType {
        FILENAME_TYPE, /// Filename element type
        GEOMNAME_TYPE  /// Geometry element type
    };

    RtNameResolverInfo() = default;
    RtToken identifier {""}; /// Unique token identifier for this information
    ElementType elementType; /// Type of element this resolver applies to
    RtNameResolverFunction toFunction; /// Resolver function to resolve to MaterialX. Can be null.
    RtNameResolverFunction fromFunction; /// Resolver function to resolve from MaterialX. Can be null.
    RtTokenMap<RtToken> toSubstitutions; /// Custom token substitutions to MaterialX. May be empty.
    RtTokenMap<RtToken> fromSubstitutions; /// Custom token substitutions from MaterialX. May be empty.
};

/// @class RtNameResolverRegistry
class RtNameResolverRegistry
{
public:
    /// Constructor
    static RtNameResolverRegistryPtr createNew();

    /// \brief Registers a pair of string resolvers for resolving scene identifiers to/from MaterialX
    /// \param nameResolverContext Name resolution information
    void registerNameResolvers(RtNameResolverInfo& info);

    /// \brief Deregisters a named pair of string resolvers
    /// \param name The name given to the pair of identifiers to deregister
    void deregisterNameResolvers(const RtToken& name);

    /// \brief Resolves the provided string. Any registered custom resolvers are applied to determine
    /// the final resolved value. The resolvers are applied in the order that they are registered.
    ///
    /// \param valueToResolve The value to resolve
    /// \param elementType The type of element to resolve
    /// \param toMaterialX Whether to convert to/from MaterialX
    RtToken resolveIdentifier(const RtToken& valueToResolve, const RtNameResolverInfo::ElementType elementType, bool toMaterialX = true) const;

private:
    RtNameResolverRegistry();
    
    friend class PvtNameResolverRegistry;
    std::unique_ptr<PvtNameResolverRegistry> _nameResolverRegistry;
};

}

#endif
