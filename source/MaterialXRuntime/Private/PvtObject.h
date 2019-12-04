//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTOBJECT_H
#define MATERIALX_PVTOBJECT_H

#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtToken.h>

#include <unordered_map>
#include <memory>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PvtObject : public std::enable_shared_from_this<PvtObject>
{
public:
    PvtObject(RtObjType type);

    virtual ~PvtObject();

    /// Return the type for this object.
    RtObjType getObjType() const
    {
        return _objType;
    }

    /// Query if the given API type is supported by this object.
    bool hasApi(RtApiType type) const;

    /// Casting the object to a given type.
    /// NOTE: No type check if performed so the templated type 
    /// must be a type supported by the object.
    template<class T> T* asA()
    {
        return static_cast<T*>(this);
    }

    /// Casting the object to a given type.
    /// NOTE: No type check if performed so the templated type 
    /// must be a type supported by the object.
    template<class T> const T* asA() const
    {
        return static_cast<const T*>(this);
    }

    /// Return an RtObject for this object.
    RtObject getObject()
    {
        return RtObject(shared_from_this());
    }

    /// Construct an RtObject from a data handle.
    static RtObject object(const PvtDataHandle& data)
    {
        return RtObject(data);
    }

    /// Retrieve the data handle from an RtObject.
    static const PvtDataHandle& data(const RtObject& obj)
    {
        return obj.data();
    }

    /// Retreive a raw pointer to the private data of an RtObject.
    /// NOTE: No type check if performed so the templated type 
    /// must be a type supported by the object.
    template<class T>
    static T* ptr(const RtObject& obj)
    {
        return static_cast<T*>(obj.data().get());
    }

    /// A nullptr data handle.
    static const PvtDataHandle NULL_DATA_HANDLE;

protected:
    RtObjType _objType;
};

}

#endif
