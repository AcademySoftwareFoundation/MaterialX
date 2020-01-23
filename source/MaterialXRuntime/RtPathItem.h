//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTPATHITEM_H
#define MATERIALX_RTPATHITEM_H

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtPath.h>
#include <MaterialXRuntime/RtValue.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

/// @class RtPathItem
/// Base class for all objects in the scene hierarchy
/// that can be identified by a path.
class RtPathItem : public RtApiBase
{
public:
    /// Return the name of the item.
    const RtToken& getName() const;

    /// Return the full path to this item.
    RtPath getPath() const;

    /// Return the parent to this path item.
    RtObject getParent() const;

    /// Return the root for this path item.
    RtObject getRoot() const;

    /// Return the stage that owns this path item.
    RtStageWeakPtr getStage() const;

    /// Add new metadata to this item.
    RtTypedValue* addMetadata(const RtToken& name, const RtToken& type);

    /// Remove metadata from this item.
    void removeMetadata(const RtToken& name);

    /// Return metadata from this item.
    RtTypedValue* getMetadata(const RtToken& name);

protected:
    /// Constructor attaching an object to the API.
    RtPathItem(const RtObject& obj);
};

}

#endif
