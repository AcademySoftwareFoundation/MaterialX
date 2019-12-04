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
    RtPath(const RtObject& obj);

    /// Copy constructor.
    RtPath(const RtPath& obj);

    /// Assignment operator.
    RtPath& operator=(const RtPath& other);

    /// Destructor.
    ~RtPath();

    /// Return true if the path points to a valid object.
    bool isValid() const;

    /// Return true if the object at the end of the path
    /// is a root object. An object with no parent.
    bool isRoot() const;

    /// Return the type for the object at the end of the path.
    RtObjType getObjType() const;

    /// Query if the given API type is supported by the object
    /// at the end of this path.
    bool hasApi(RtApiType type) const;

    /// Return the object at the end of the path.
    /// A null object is returned if the path is no
    /// longer valid.
    RtObject getObject() const;

    /// Reset the path to point to a new object.
    void setObject(const RtObject& obj);

    /// Return a string representation of this path.
    string getPathString() const;

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

    /// Equality operator.
    bool operator==(const RtPath& other) const;

    /// Inequality operator.
    bool operator!=(const RtPath& other) const
    {
        return !(*this == other);
    }

private:
    void* _ptr;
};

}

#endif
