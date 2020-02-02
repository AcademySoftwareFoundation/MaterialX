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
#include <atomic>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PvtPrim;
class PvtStage;
class PvtPath;

using PvtDataHandleVec = vector<PvtDataHandle>;
using PvtDataHandleMap = RtTokenMap<PvtDataHandle>;
using PvtDataHandleSet = std::set<PvtDataHandle>;

// Class representing an object in the scene hierarchy.
// This is the base class for prims, attributes and relationships.
class PvtObject
{
    DECLARE_REF_COUNTED_CLASS(PvtObject)

public:
    // Return the type id for this object.
    RtObjType getObjType() const
    {
        return _objType;
    }

    /// Return the type of this object class.
    static RtObjType objType()
    {
        return RtObjType::OBJECT;
    }

    /// Return true if this object is of the templated type.
    template<class T>
    bool isA() const
    {
        return _objType == T::objType();
    }

    // Casting the object to a given type.
    // NOTE: No type check if performed so the templated type 
    // must be a type supported by the object.
    template<class T> T* asA()
    {
        return static_cast<T*>(this);
    }

    // Casting the object to a given type.
    // NOTE: No type check if performed so the templated type 
    // must be a type supported by the object.
    template<class T> const T* asA() const
    {
        return static_cast<const T*>(this);
    }

    // Return a handle for the object.
    PvtDataHandle hnd() const
    {
        return PvtDataHandle(const_cast<PvtObject*>(this));
    }

    // Return a handle for the given object.
    static PvtDataHandle hnd(const RtObject& obj)
    {
        return obj.hnd();
    }

    // Return an RtObject for this object.
    RtObject obj() const
    {
        return RtObject(hnd());
    }

    // Retreive a raw pointer to the private data of an RtObject.
    // NOTE: No type check if performed so the templated type 
    // must be a type supported by the object.
    template<class T>
    static T* ptr(const RtObject& obj)
    {
        return static_cast<T*>(obj.hnd().get());
    }

    const RtToken& getName() const
    {
        return _name;
    }

    PvtPath getPath() const;

    PvtPrim* getParent() const
    {
        return _parent;
    }

    PvtPrim* getRoot() const;

    RtStageWeakPtr getStage() const;

    RtTypedValue* addMetadata(const RtToken& name, const RtToken& type);

    void removeMetadata(const RtToken& name);

    const RtTypedValue* getMetadata(const RtToken& name) const
    {
        auto it = _metadataMap.find(name);
        return it != _metadataMap.end() ? &it->second : nullptr;
    }

    RtTypedValue* getMetadata(const RtToken& name)
    {
        auto it = _metadataMap.find(name);
        return it != _metadataMap.end() ? &it->second : nullptr;
    }

    // For serialization to file we need the order.
    const vector<RtToken>& getMetadataOrder() const
    {
        return _metadataOrder;
    }

protected:
    PvtObject(RtObjType objType, const RtToken& name, PvtPrim* parent);

    // Protected as arbitrary renaming is not supported.
    // Must be done from the owning stage.
    void setName(const RtToken& name)
    {
        _name = name;
    }

    // Protected as arbitrary reparenting is not supported.
    // Must be done from the owning stage.
    void setParent(PvtPrim* parent)
    {
        _parent = parent;
    }

    const RtObjType _objType;
    RtToken _name; // TODO: Store a path instead of name token
    PvtPrim* _parent;
    RtTokenMap<RtTypedValue> _metadataMap;
    vector<RtToken> _metadataOrder;

    friend class PvtPrim;
    friend class PvtAttribute;
    friend class PvtInput;
    friend class PvtOutput;
};

}

#endif
