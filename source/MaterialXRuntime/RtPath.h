//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTPATH_H
#define MATERIALX_RTPATH_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtObject.h>

namespace MaterialX
{

/// @class RtPath
/// Class representing a path which points to, and uniquely identifies,
/// an object in a stage.
/// Can be used to store references to objects in a stage hierarchy.
/// Note that an RtPath may become invalid if the object pointed to or
/// any of its parents are renamed, reparented or removed from the stage.
/// RtPath::isValid() can be used to query if a path is still valid.
class RtPath
{
public:
    /// Construct an empty path.
    RtPath();

    /// Construct a path from an object.
    explicit RtPath(const RtObject& obj);

    /// Construct a path from a string.
    RtPath(const string& path);

    /// Construct a path from a raw string.
    RtPath(const char* path);

    /// Copy constructor.
    RtPath(const RtPath& path);

    /// Assignment operator.
    RtPath& operator=(const RtPath& other);

    /// Destructor.
    ~RtPath();

    /// Return the name of the object at the end of this path.
    const RtToken& getName() const;

    /// Return a string representation of this path.
    string asString() const;

    /// Push a child element on the path and make it the
    /// top element pointed to by the path.
    /// The given child must be an existing child object on the
    /// top most object pointed to by the path. If not the path
    /// becomes invalid after the push operation.
    void push(const RtToken& childName);

    /// Pop the top most object from the path.
    /// After this operation the path will point to
    /// the parent of the currently top most object.
    void pop();

    /// Set a new object for this path.
    void setObject(const RtObject& obj);

    /// Equality operator.
    bool operator==(const RtPath& other) const;

    /// Inequality operator.
    bool operator!=(const RtPath& other) const
    {
        return !(*this == other);
    }

    /// Return true of this path points to the root.
    bool isRoot() const;

private:
    void* _ptr;
    friend class RtStage;
    friend class PvtPath;
};

}

#endif
