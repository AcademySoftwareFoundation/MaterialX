//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTNAMERESOLVER_H
#define MATERIALX_PVTNAMERESOLVER_H

#include <MaterialXRuntime/RtNameResolver.h>

#include <MaterialXCore/Element.h>

#include <unordered_map>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PvtStringResolverPair;

/// Shared pointer to a PvtStringResolverPair
using PvtStringResolverPairPtr = std::shared_ptr<class PvtStringResolverPair>;

/// Map of identifier to RtStringResolverPairPtr
using PvtStringResolverMap = RtIdentifierMap<PvtStringResolverPairPtr>;

/// @class PvtStringResolverPair
///
/// Class that keeps track of a pair of String Resolvers that are used to
/// convert string identifiers to and from MaterialX.
///
class PvtStringResolverPair
{
  public:
    /// \brief Creates an PvtStringResolverPair shared_ptr
    /// \param type The type of element the resolver pair resolves
    /// \param toMaterialXResolver Resolves string identifiers to MaterialX document format
    /// \param fromMaterialXResolver Resolves string identifiers from MaterialX document format
    /// \return Returns a PvtStringResolverPair shared_ptr
    static PvtStringResolverPairPtr createNew(const RtNameResolverInfo::ElementType elementType,
                                              StringResolverPtr toMaterialXResolver,
                                              StringResolverPtr fromMaterialXResolver);

    /// \brief Returns the type of element resolved by the pair of resolvers
    /// \return Type of element resolved by the pair of resolvers
    const RtNameResolverInfo::ElementType getType() const;

    /// \brief Returns the resolver used to convert string identifiers to MaterialX document format
    /// \return StringResolverPtr to the resolver used to convert string identifiers to MaterialX document format
    StringResolverPtr getToMaterialXResolver();

    /// \brief Returns the resolver used to convert string identifiers from MaterialX document format
    /// \return StringResolverPtr to the resolver used to convert string identifiers from MaterialX document format
    StringResolverPtr getFromMaterialXResolver();

  private:
    PvtStringResolverPair(const RtNameResolverInfo::ElementType elementType,
                          StringResolverPtr toMaterialXResolver,
                          StringResolverPtr fromMaterialXResolver);

    const RtNameResolverInfo::ElementType _elementType;
    StringResolverPtr _toMaterialXResolver;
    StringResolverPtr _fromMaterialXResolver;
};

/// @class PvtNameResolverRegistry
class PvtNameResolverRegistry
{
public:
    /// Constructor
    PvtNameResolverRegistry();

    /// \brief Registers a pair of string resolvers for resolving scene identifiers to/from MaterialX
    /// \param nameResolverContext Name resolution information
    void registerNameResolvers(RtNameResolverInfo& info);

    /// \brief Deregisters a named pair of string resolvers
    /// \param name The name given to the pair of identifiers to deregister
    void deregisterNameResolvers(const RtIdentifier& name);

    /// \brief Resolves the provided string. Any registered custom resolvers are applied to determine
    /// the final resolved value. The resolvers are applied in the order that they are registered.
    ///
    /// \param valueToResolve The value to resolve
    /// \param elementType The type of element to resolve
    /// \param toMaterialX Whether to convert to/from MaterialX
    RtIdentifier resolveIdentifier(const RtIdentifier& valueToResolve, const RtNameResolverInfo::ElementType elementType, bool toMaterialX = true) const;                                              

private:
    PvtStringResolverMap _resolvers;
};

}

#endif

