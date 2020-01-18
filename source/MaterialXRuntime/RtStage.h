//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTSTAGE_H
#define MATERIALX_RTSTAGE_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtTraversal.h>
#include <MaterialXRuntime/RtPath.h>

namespace MaterialX
{

/// @class RtStage
/// API for accessing a stage. This API can only be
/// attached to objects of type STAGE.
class RtStage : public RtApiBase
{
public:
    /// Constructor attaching a stage object to the API.
    RtStage(const RtObject& obj);

    /// Return the type for this API.
    RtApiType getApiType() const override;

    /// Return the name of the stage.
    const RtToken& getName() const;

    /// Create a new empty stage.
    static RtObject createNew(const RtToken& name);

    /// Create a new prim at the root of the stage.
    RtObject createPrim(const RtToken& typeName, const RtObject def = RtObject());

    /// Create a new prim at the given path.
    RtObject createPrim(const RtPath& path, const RtToken& typeName, const RtObject def = RtObject());

    /// Create a new prim inside the parent given by path.
    /// If an empty name is given a name will be generated.
    RtObject createPrim(const RtPath& parentPath, const RtToken& name,
                        const RtToken& typeName, const RtObject def = RtObject());

    /// Remove a prim from the stage.
    void removePrim(const RtPath& path);

    /// Rename a prim in the stage.
    RtToken renamePrim(const RtPath& path, const RtToken& newName);

    /// Move a prim to a new parent.
    RtToken reparentPrim(const RtPath& path, const RtPath& newParentPath);

    // Find the prim at the given path, Returns a null object
    // if no such prim is found.
    RtObject getPrimAtPath(const RtPath& path);

    // Return the prim representing the root of the stage's prim hierarchy.
    RtObject getRootPrim();

    /// Return an iterator traversing all child prims (siblings) in the stage,
    /// including children from any referenced stages.
    /// Using a predicate this method can be used to find all child prims
    /// of a specific object type, or all child prims supporting a
    /// specific API, etc.
    RtStageIterator traverse(RtObjectPredicate predicate);

    /// Add a reference to another stage.
    void addReference(const RtObject& stage);

    /// Remove a reference to another stage.
    void removeReference(const RtToken& name);

    /// Remove all references to other stages
    void removeReferences();

    /// Return the number of references
    size_t numReferences() const;

    /// Get a reference by index.
    RtObject getReference(size_t index) const;

    /// Find a reference by name
    RtObject findReference(const RtToken& name) const;

  protected:
    friend class RtFileIo;
};

}

#endif
