//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTOBJECT_H
#define MATERIALX_RTOBJECT_H

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtToken.h>

#include <memory>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PrvObject;

// A handle to private data
// TODO: implement a custom refcounted handle class
using PrvObjectHandle = std::shared_ptr<PrvObject>;

/// Type identifiers for concrete runtime objects.
enum class RtObjType
{
    INVALID,
    PORTDEF,
    NODEDEF,
    NODE,
    NODEGRAPH,
    STAGE,
    UNKNOWN,
    NUM_TYPES
};

/// Type identifiers for API's attachable to runtime objects.
enum class RtApiType
{
    ELEMENT,
    PORTDEF,
    NODEDEF,
    NODE,
    NODEGRAPH,
    STAGE,
    CORE_IO,
    STAGE_ITERATOR,
    TREE_ITERATOR,
    GRAPH_ITERATOR,
    NUM_TYPES
};

/// @class RtObject
/// Base class for all MaterialX runtime objects.
class RtObject
{
public:
    /// Default constructor.
    RtObject();

    /// Construct from a data handle.
    RtObject(PrvObjectHandle data);

    /// Copy constructor.
    RtObject(const RtObject& other);

    /// Destructor.
    ~RtObject();

    /// Return the type for this object.
    RtObjType getObjType() const;

    /// Query if the given API type is supported by this object.
    bool hasApi(RtApiType type) const;

    /// Return true if the object is valid.
    bool isValid() const;

    /// Returns true if the object is invalid.
    bool operator!() const
    {
        return !isValid();
    }

    /// Explicit bool conversion operator.
    /// Return true if the object is valid.
    explicit operator bool() const
    {
        return isValid();
    }

    /// Return true if the objects are equal.
    bool operator==(const RtObject& other) const
    {
        return _data == other._data;
    }

    /// Return the data handle.
    PrvObjectHandle data() const
    {
        return _data;
    }

private:
    PrvObjectHandle _data;
};

/// @class RtObject
/// Base class for all API supported on objects.
class RtApiBase
{
public:
    /// Destructor.
    virtual ~RtApiBase() {};

    /// Return the type for this API.
    virtual RtApiType getApiType() const = 0;

    /// Query if the given object type is supported by this API.
    /// Derived classes should override this.
    bool isSupported(RtObjType type) const;

    /// Query if the given object is supported by this API.
    bool isSupported(const RtObject& obj) const
    {
        return isSupported(obj.getObjType());
    }

    /// Attach an object to this API.
    void setObject(const RtObject& obj);

    /// Return the object attached to this API.
    RtObject getObject();

    /// Return true if the API object is valid.
    bool isValid() const;

    /// Return true if the API object is invalid.
    bool operator!() const
    {
        return !isValid();
    }

    /// Explicit bool conversion operator.
    /// Return true if the API object is valid.
    explicit operator bool() const
    {
        return isValid();
    }

    /// Return true if the attached objects are equal.
    bool operator==(const RtApiBase& other) const
    {
        return _data == other._data;
    }

protected:
    /// Default constructor.
    RtApiBase();

    /// Construct from a data handle.
    RtApiBase(PrvObjectHandle data);

    /// Construct from an object.
    RtApiBase(const RtObject& obj);

    /// Copy constructor.
    RtApiBase(const RtApiBase& other);

    /// Set data for this API.
    void setData(PrvObjectHandle data);

    /// Return data set for this API.
    PrvObjectHandle& data() { return _data; }

    /// Return data set for this API.
    const PrvObjectHandle& data() const { return _data; }

private:
    /// Internal data attached to the API.
    PrvObjectHandle _data;
};

}

#endif
