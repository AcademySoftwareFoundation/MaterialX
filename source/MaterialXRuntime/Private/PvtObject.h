//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTOBJECT_H
#define MATERIALX_PVTOBJECT_H

#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtToken.h>
#include <MaterialXRuntime/RtValue.h>

#include <unordered_map>
#include <set>
#include <memory>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PvtPrim;
class PvtStage;

using PvtDataHandleVec = vector<PvtDataHandle>;
using PvtDataHandleMap = RtTokenMap<PvtDataHandle>;
using PvtDataHandleSet = std::set<PvtDataHandle>;

/// Class representing an object in the scene hierarchy.
/// This is the base class for prims, attributes and relationships.
class PvtObject : public RtRefBase<PvtObject>
{
public:
    virtual ~PvtObject() {}

    /// Return the type id for this object.
    virtual RtObjType getObjType() const = 0;

    /// Return the type name for this object.
    virtual const RtToken& getObjTypeName() const = 0;

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

    /// Return a handle for the object.
    PvtDataHandle hnd() const
    {
        return const_cast<PvtObject*>(this)->shared_from_this();
    }

    /// Return an RtObject for this object.
    RtObject obj() const
    {
        return RtObject(hnd());
    }

    /// Construct an RtObject from a data handle.
    static RtObject obj(const PvtDataHandle& hnd)
    {
        return RtObject(hnd);
    }

    /// Retrieve the data handle from an RtObject.
    static const PvtDataHandle& hnd(const RtObject& obj)
    {
        return obj.hnd();
    }

    /// Retreive a raw pointer to the private data of an RtObject.
    /// NOTE: No type check if performed so the templated type 
    /// must be a type supported by the object.
    template<class T>
    static T* ptr(const RtObject& obj)
    {
        return static_cast<T*>(obj.hnd().get());
    }
};

}

#endif
