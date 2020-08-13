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

namespace MaterialX
{

class RtPath;
class RtNodeGraph;
class RtNodeDef;

/// @class RtStage
/// A stage is the root container of material description data.
/// Creates and owns the primitives that builds up the material
/// description graph hierarchy.
class RtStage : public RtSharedBase<RtStage>
{
public:
    /// Destructor
    ~RtStage();

    /// Return the name of the stage.
    const RtToken& getName() const;

    /// Add a source Uri for a stage
    void addSourceUri(const RtToken& uri);

    /// Return a list of source Uri loaded into a stage
    const RtTokenVec& getSourceUri() const;

    /// Create a new prim at the root of the stage.
    RtPrim createPrim(const RtToken& typeName);

    /// Create a new prim at the given path.
    RtPrim createPrim(const RtPath& path, const RtToken& typeName);

    /// Create a new prim inside the parent given by path.
    /// If an empty name is given a name will be generated.
    RtPrim createPrim(const RtPath& parentPath, const RtToken& name, const RtToken& typeName);

    /// Create a node definition based on a nodegraph
    RtPrim createNodeDef(RtNodeGraph& nodeGraph, 
                         const RtToken& nodeDefName, const RtToken& nodeName, const RtToken& version, bool isDefaultVersion, const RtToken& nodeGroup = EMPTY_TOKEN);

    /// Return the implementation for a given definition if it exists.
    /// Currently only nodegraph implemenations are considered. If the implementaiton is not a nodegraph
    /// an invalid RtPrim will be returned.
    RtPrim getImplementation(const RtNodeDef& definition) const;

    /// Remove a prim from the stage.
    void removePrim(const RtPath& path);

    /// Rename a prim in the stage.
    RtToken renamePrim(const RtPath& path, const RtToken& newName);

    /// Move a prim to a new parent.
    RtToken reparentPrim(const RtPath& path, const RtPath& newParentPath);

    /// Find the prim at the given path, Returns a null object
    /// if no such prim is found.
    RtPrim getPrimAtPath(const RtPath& path);

    /// Return the prim representing the root of the stage's prim hierarchy.
    RtPrim getRootPrim();

    /// Return an iterator traversing all child prims (siblings) in the stage,
    /// including children from any referenced stages.
    /// Using a predicate this method can be used to find all child prims
    /// of a specific object type, or all child prims supporting a
    /// specific API, etc.
    RtStageIterator traverse(RtObjectPredicate predicate = nullptr);

    /// Add a reference to another stage.
    void addReference(RtStagePtr stage);

    /// Return a referenced stage by name.
    RtStagePtr getReference(const RtToken& name) const;

    /// Remove a reference to another stage.
    void removeReference(const RtToken& name);

    /// Remove all references to other stages
    void removeReferences();

protected:
    RtStage();

    static RtStagePtr createNew(const RtToken& name);

    void setName(const RtToken& name);

    void disposePrim(const RtPath& path);
    void restorePrim(const RtPath& parentPath, const RtPrim& prim);

    void* _ptr;
    friend class PvtStage;
    friend class PvtApi;
    friend class PvtCreatePrimCmd;
    friend class PvtCopyPrimCmd;
    friend class PvtRemovePrimCmd;
};

}

#endif
