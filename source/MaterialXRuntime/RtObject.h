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

class PvtObject;

// A handle to private data
// TODO: implement a custom refcounted handle class
using PvtDataHandle = std::shared_ptr<PvtObject>;

/// Type identifiers for concrete runtime objects.
enum class RtObjType
{
    NONE,
    PRIM,
    RELATIONSHIP,
    ATTRIBUTE,
    NODEDEF,
    NODE,
    NODEGRAPH,
    STAGE,
    NUM_TYPES
};

/// Type identifiers for API's attachable to runtime objects.
enum class RtApiType
{
    PATH_ITEM,
    PRIM,
    RELATIONSHIP,
    ATTRIBUTE,
    INPUT,
    OUTPUT,
    NODEDEF,
    NODE,
    NODEGRAPH,
    STAGE,
    FILE_IO,
    NUM_TYPES
};

/// @class RtObject
/// Base class for all MaterialX runtime objects.
class RtObject
{
public:
    /// Default constructor.
    RtObject();

    /// Copy constructor.
    RtObject(const RtObject& other);

    /// Destructor.
    ~RtObject();

    /// Return the type id for this object.
    RtObjType getObjType() const;

    /// Return the type name for this object.
    const RtToken& getObjTypeName() const;

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
        return _hnd == other._hnd;
    }

private:
    /// Construct from a data handle.
    RtObject(PvtDataHandle data);

    /// Return the data handle.
    const PvtDataHandle& hnd() const
    {
        return _hnd;
    }

    /// Internal data handle.
    PvtDataHandle _hnd;

    friend class PvtObject;
    friend class RtApiBase;
};

/// @class RtApiBase
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
    RtObject getObject() const;

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
        return _hnd == other._hnd;
    }

protected:
    /// Construct from a data handle.
    RtApiBase(PvtDataHandle data);

    /// Construct from an object.
    RtApiBase(const RtObject& obj);

    /// Copy constructor.
    RtApiBase(const RtApiBase& other);

    /// Set data handle for this API.
    void setHnd(PvtDataHandle hnd);

    /// Return data set for this API.
    PvtDataHandle& hnd() { return _hnd; }

    /// Return data set for this API.
    const PvtDataHandle& hnd() const { return _hnd; }

private:
    /// Handle for object attached to the API.
    PvtDataHandle _hnd;
};

}

#endif
