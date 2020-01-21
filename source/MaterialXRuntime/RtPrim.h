//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTPRIM_H
#define MATERIALX_RTPRIM_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtPathItem.h>
#include <MaterialXRuntime/RtTraversal.h>

namespace MaterialX
{

/// @class RtPrim
/// API for accessing a prim object. This API can be
/// attached to objects of all prim types.
class RtPrim : public RtPathItem
{
public:
    /// Constructor attaching an object to the API.
    RtPrim(const RtObject& obj);

    /// Return the type name for nodes.
    static const RtToken& typeName();

    /// Return the type for this API.
    RtApiType getApiType() const override;

    /// Return the prim type name for this prim.
    const RtToken& getPrimTypeName() const;

    /// Add a relationship to the prim.
    RtObject createRelationship(const RtToken& name);

    /// Remove a relationship from the prim.
    void removeRelationship(const RtToken& name);

    /// Return a relationship by name, or a null object
    /// if no such relationship exists.
    RtObject getRelationship(const RtToken& name) const;

    /// Add an attribute to the nodegraph.
    RtObject createAttribute(const RtToken& name, const RtToken& type, uint32_t flags = 0);

    /// Remove an attribute from the nodegraph.
    void removeAttribute(const RtToken& name);

    /// Return an attribute by name, or a null object
    /// if no such attribute exists.
    RtObject getAttribute(const RtToken& name) const;

    /// Return an iterator traversing all attributes
    /// of this prim.
    RtAttrIterator getAttributes(RtObjectPredicate predicate = nullptr) const;

    /// Return a child prim by name, or a null object
    /// if no such child prim exists.
    RtObject getChild(const RtToken& name) const;

    /// Return an iterator traversing all child prims (siblings).
    /// Using a predicate this method can be used to find all child prims
    /// of a specific object type, or all child prims supporting a
    /// specific API, etc.
    RtPrimIterator getChildren(RtObjectPredicate predicate = nullptr) const;
};

}

#endif
