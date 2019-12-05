//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTSTAGE_H
#define MATERIALX_RTSTAGE_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtElement.h>
#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtTraversal.h>
#include <MaterialXRuntime/RtPath.h>

namespace MaterialX
{

/// @class RtStage
/// API for accessing a stage. This API can only be
/// attached to objects of type STAGE.
class RtStage : public RtElement
{
public:
    /// Constructor attaching a stage object to the API.
    RtStage(const RtObject& obj);

    /// Return the type for this API.
    RtApiType getApiType() const override;

    /// Create a new empty stage.
    static RtObject createNew(const RtToken& name);

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

    /// Add an element to the stage.
    void addElement(const RtObject& elem);

    /// Remove an element from the stage.
    void removeElement(const RtObject& elem);

    /// Remove the element at the end of the given path.
    /// Will throw exception if the given path is not
    /// rooted in this stage.
    void removeElementByPath(const RtPath& path);

    /// Return an element by name,
    /// or a null object if no such element exists.
    RtObject findElementByName(const RtToken& name) const;

    /// Find an element by string path.
    /// Paths are element names expressed hierarchicaly and seperated by '/'.
    ///
    /// Exanples:
    ///     "/nodegraph1/add1"
    ///     "/nodegraph2/multiply4/in1"
    ///     "/ND_foo/out"
    ///
    /// When finding ports on node instances an object for the portdef will
    /// be returned. In order to get to the port from the portdef use the
    /// port constructor method: RtPort(RtObject node, RtObject portdef)
    ///
    RtObject findElementByPath(const string& path) const;

    /// Return an iterator traversing the stage, including any referenced
    /// stages. If a filter is set it will be called to control which
    /// objects to return.
    RtStageIterator traverseStage(RtTraversalFilter filter = nullptr);

  protected:
    friend class RtFileIo;
};

}

#endif
