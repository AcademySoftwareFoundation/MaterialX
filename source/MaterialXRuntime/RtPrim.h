//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTPRIM_H
#define MATERIALX_RTPRIM_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtTypeInfo.h>
#include <MaterialXRuntime/RtAttribute.h>
#include <MaterialXRuntime/RtRelationship.h>

namespace MaterialX
{

class RtAttrIterator;
class RtRelationshipIterator;
class RtPrimIterator;
class RtSchemaBase;

/// @class RtPrim
class RtPrim : public RtObject
{
    RT_DECLARE_RUNTIME_OBJECT(RtPrim)

public:
    /// Empty constructor.
    /// Creating an invalid object.
    RtPrim() {}

    /// Construct from a data handle.
    RtPrim(PvtDataHandle hnd);

    /// Construct from an object.
    RtPrim(RtObject obj);

    /// Return the typeinfo for this prim.
    const RtTypeInfo* getTypeInfo() const;

    /// Return true if this prim supports the templated schema class.
    template<class T>
    bool hasApi() const
    {
        static_assert(std::is_base_of<RtSchemaBase, T>::value,
            "Templated type must be a concrete subclass of RtSchemaBase");
        return getTypeInfo()->isCompatible(T::typeName());
    }

    /// Return the typename for this prim.
    const RtToken& getTypeName() const
    {
        return getTypeInfo()->getShortTypeName();
    }

    /// Add a relationship to the prim.
    RtRelationship createRelationship(const RtToken& name);

    /// Remove a relationship from the prim.
    void removeRelationship(const RtToken& name);

    /// Return a relationship by name, or a null object
    /// if no such relationship exists.
    RtRelationship getRelationship(const RtToken& name) const;

    /// Return an iterator over all relationships of this prim.
    RtRelationshipIterator getRelationships() const;

    /// Add an attribute to the prim.
    RtAttribute createAttribute(const RtToken& name, const RtToken& type, uint32_t flags = 0);

    /// Add an input attribute to the prim.
    RtInput createInput(const RtToken& name, const RtToken& type, uint32_t flags = 0);

    /// Remove an attribute from the prim.
    void removeAttribute(const RtToken& name);

    /// Add an output attribute to the prim.
    RtOutput createOutput(const RtToken& name, const RtToken& type, uint32_t flags = 0);

    /// Return an attribute by name, or a null object
    /// if no such attribute exists.
    RtAttribute getAttribute(const RtToken& name) const;

    /// Return the number of inputs on the prim.
    size_t numInputs() const;

    /// Return an input attribute by name, or a null object
    /// if no such input attribute exists.
    RtInput getInput(const RtToken& name) const;

    /// Return the number of outputs on the prim.
    size_t numOutputs() const;

    /// Return an input attribute by name, or a null object
    /// if no such input attribute exists. If an empty name
    /// is provided, then the first output is returned
    RtOutput getOutput(const RtToken& name) const;

    /// Return the single output for single output prims.
    /// Or if multiple outputs are available return the
    /// last created output.
    RtOutput getOutput() const;

    /// Return an iterator traversing all attributes
    /// of this prim.
    RtAttrIterator getAttributes(RtObjectPredicate predicate = nullptr) const;

    /// Return an iterator traversing all input attributes
    /// on this prim.
    RtAttrIterator getInputs() const;

    /// Return an iterator traversing all output attributes
    /// on this prim.
    RtAttrIterator getOutputs() const;

    /// Return the number of children in the prim.
    size_t numChildren() const;

    /// Return a child prim by name, or a null object
    /// if no such child prim exists.
    RtPrim getChild(const RtToken& name) const;

    /// Return an iterator traversing all child prims (siblings).
    /// Using a predicate this method can be used to find all child prims
    /// of a specific object type, or all child prims supporting a
    /// specific API, etc.
    RtPrimIterator getChildren(RtObjectPredicate predicate = nullptr) const;
};

/// Function type for creating prims for a typed schema.
using RtPrimCreateFunc = std::function<RtPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)>;

}

#endif
