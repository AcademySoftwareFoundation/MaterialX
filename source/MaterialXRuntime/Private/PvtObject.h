//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTOBJECT_H
#define MATERIALX_PVTOBJECT_H

#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtToken.h>
#include <MaterialXRuntime/RtValue.h>

#include <MaterialXRuntime/Private/PvtPath.h>

#include <unordered_map>
#include <set>
#include <atomic>

/// @file
/// TODO: Docs

namespace MaterialX
{

// Class representing an object in the scene hierarchy.
// This is the base class for prims, attributes and relationships.
class PvtObject : public RtRefCounted<PvtObject>
{
    RT_DECLARE_RUNTIME_OBJECT(PvtObject)

public:
    using TypeBits = uint8_t;

public:
    virtual ~PvtObject();

    bool isDisposed() const
    {
        return (_typeBits & TypeBits(RtObjType::DISPOSED)) != 0;
    }

    void setDisposed(bool state)
    {
        if (state)
        {
            _typeBits |= TypeBits(RtObjType::DISPOSED);
        }
        else
        {
            _typeBits &= ~TypeBits(RtObjType::DISPOSED);
        }
    }

    bool isCompatible(RtObjType objType) const
    {
        return ((_typeBits & TypeBits(objType)) &
            ~TypeBits(RtObjType::DISPOSED)) != 0;
    }

    /// Return true if this object is of the templated type.
    template<class T>
    bool isA() const
    {
        static_assert(std::is_base_of<PvtObject, T>::value,
            "Templated type must be an PvtObject or a subclass of PvtObject");
        return isCompatible(T::classType());
    }

    // Casting the object to a given type.
    // NOTE: In release builds no type check is performed so the templated type 
    // must be of a type compatible with this object.
    template<class T> T* asA()
    {
        static_assert(std::is_base_of<PvtObject, T>::value,
            "Templated type must be an PvtObject or a subclass of PvtObject");
// TODO: We enable these runtime checks for all build configurations for now,
//       but disabled this later to avoid the extra cost in release builds.
// #ifndef NDEBUG
        // In debug mode we do safety checks on object validity
        // and type cast compatibility.
        if (isDisposed())
        {
            throw ExceptionRuntimeError("Trying to access a disposed object '" + getName().str() + "'");
        }
        if (!isCompatible(T::classType()))
        {
            throw ExceptionRuntimeError("Types are incompatible for type cast, '" + getName().str() + "' is not a '" + T::className().str() + "'");
        }
// #endif
        return static_cast<T*>(this);
    }

    // Casting the object to a given type.
    // NOTE: In release builds no type check is performed so the templated type 
    // must be of a type compatible with this object.
    template<class T> const T* asA() const
    {
        return const_cast<PvtObject*>(this)->asA<T>();
    }

    // Return a handle for the object.
    PvtObjHandle hnd() const
    {
        return PvtObjHandle(const_cast<PvtObject*>(this));
    }

    // Return a handle for the given object.
    static PvtObjHandle hnd(const RtObject& obj)
    {
        return obj.hnd();
    }

    // Return an RtObject for this object.
    RtObject obj() const
    {
        return RtObject(hnd());
    }

    // Retreive a raw pointer to the private data of an RtObject.
    // NOTE: No type check is performed so the templated type 
    // must be a type supported by the object.
    template<class T = PvtObject>
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

    RtTypedValue* createAttribute(const RtToken& name, const RtToken& type);

    void removeAttribute(const RtToken& name);

    // Get an attribute without a type check.
    RtTypedValue* getAttribute(const RtToken& name)
    {
        auto it = _attr.find(name);
        return it != _attr.end() ? it->second : nullptr;
    }

    // Get an attribute without a type check.
    const RtTypedValue* getAttribute(const RtToken& name) const
    {
        return const_cast<PvtObject*>(this)->getAttribute(name);
    }

    // Get an attribute with type check.
    RtTypedValue* getAttribute(const RtToken& name, const RtToken& type);

    // Get an attribute with type check.
    const RtTypedValue* getAttribute(const RtToken& name, const RtToken& type) const
    {
        return const_cast<PvtObject*>(this)->getAttribute(name, type);
    }

    // Get the map of all attributes.
    const RtTokenMap<RtTypedValue*>& getAttributes() const
    {
        return _attr;
    }

    // Get the vector of all attributes.
    const RtTokenVec& getAttributeNames() const
    {
        return _attrNames;
    }

protected:
    PvtObject(const RtToken& name, PvtPrim* parent);

    template<typename T>
    void setTypeBit()
    {
        _typeBits |= TypeBits(T::classType());
    }

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

    TypeBits _typeBits;
    RtToken _name; // TODO: Store a path instead of name token
    PvtPrim* _parent;
    RtTokenMap<RtTypedValue*> _attr;
    RtTokenVec _attrNames;

    friend class PvtPrim;
    friend class PvtPort;
    friend class PvtInput;
    friend class PvtOutput;
    friend class PvtNodeGraphPrim;
    friend class PvtObjectList; 
    friend class RtAttributeIterator;
    RT_FRIEND_REF_PTR_FUNCTIONS(PvtObject)
};


using PvtObjectVec = vector<PvtObject*>;

// An object container with support for random access, 
// ordered access and access by name search.
class PvtObjectList
{
public:
    size_t size() const
    {
        return _vec.size();
    }

    bool empty() const
    {
        return _vec.empty();
    }

    size_t count(const RtToken& name) const
    {
        return _map.count(name);
    }

    PvtObject* find(const RtToken& name) const
    {
        auto it = _map.find(name);
        return it != _map.end() ? it->second.get() : nullptr;
    }

    PvtObject* operator[](size_t i) const
    {
        return i < _vec.size() ? _vec[i] : nullptr;
    }

    void add(PvtObject* obj)
    {
        _map[obj->getName()] = obj->hnd();
        _vec.push_back(obj);
    }

    PvtObjHandle remove(const RtToken& name);

    RtToken rename(const RtToken& name, const RtToken& newName, const PvtPrim* parent);

    void clear()
    {
        _map.clear();
        _vec.clear();
    }

    const PvtObjectVec& vec() const
    {
        return _vec;
    }

private:
    RtTokenMap<PvtObjHandle> _map;
    PvtObjectVec _vec;

    friend class RtPrimIterator;
    friend class RtInputIterator;
    friend class RtOutputIterator;
    friend class RtRelationshipIterator;
};

}

#endif
