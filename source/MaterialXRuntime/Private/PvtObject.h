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
    /// NOTE: no type check if performed so the templated type 
    /// must be a type supported by the object.
    template<class T> T* asA()
    {
        return static_cast<T*>(this);
    }

    /// Casting the object to a given type.
    /// NOTE: no type check if performed so the templated type 
    /// must be a type supported by the object.
    template<class T> const T* asA() const
    {
        return static_cast<const T*>(this);
    }

    RtObject getObject()
    {
        return RtObject(shared_from_this());
    }

protected:
    RtObjType _objType;
};

}

#endif
