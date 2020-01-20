//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTPRIM_H
#define MATERIALX_PVTPRIM_H

#include <MaterialXRuntime/Private/PvtObject.h>
#include <MaterialXRuntime/Private/PvtAttribute.h>

#include <MaterialXRuntime/RtTraversal.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

// Allocator class handling allocation of data for elements.
// The data allocated is kept by the allocator and freed
// upon allocator destruction or by calling free() explicitly.
// NOTE: Data is stored as raw byte pointers and destructors
// for allocated objects will not be called when freeing data.
class PvtAllocator
{
public:
    ~PvtAllocator()
    {
        free();
    }

    // Allocate and return a block of data.
    uint8_t* alloc(size_t size)
    {
        uint8_t* ptr = new uint8_t[size];
        _storage.push_back(ptr);
        return ptr;
    }

    // Allocate and return a single object of templated type.
    // The object constructor is called to initialize it.
    template<class T>
    T* allocType()
    {
        uint8_t* buffer = alloc(sizeof(T));
        return new (buffer) T();
    }

    // Free all allocated data.
    void free()
    {
        for (uint8_t* ptr : _storage)
        {
            delete[] ptr;
        }
        _storage.clear();
    }

private:
    vector<uint8_t*> _storage;
};

class PvtPrim : public PvtPathItem
{
public:
    static RtObjType typeId() { return _typeId; }

    static const RtToken& typeName() { return _typeName; }

    static PvtDataHandle createNew(const RtToken& name, PvtPrim* parent);

    RtObjType getObjType() const override
    {
        return _typeId;
    }

    const RtToken& getObjTypeName() const override
    {
        return _typeName;
    }

    const RtToken& getPrimTypeName() const;

    void setPrimTypeName(const RtToken& primTypeName);

    virtual PvtAttribute* createAttribute(const RtToken& name, const RtToken& type, uint32_t flags = 0);

    virtual void removeAttribute(const RtToken& name);

    PvtAttribute* getAttribute(const RtToken& name)
    {
        auto it = _attrMap.find(name);
        return it != _attrMap.end() ? it->second->asA<PvtAttribute>() : nullptr;
    }

    const PvtAttribute* getAttribute(const RtToken& name) const
    {
        return const_cast<PvtPrim*>(this)->getAttribute(name);
    }

    RtAttrIterator getAttributes(RtObjectPredicate predicate = nullptr) const
    {
        return RtAttrIterator(this->obj(), predicate);
    }

    const PvtDataHandleVec& getAllAttributes() const
    {
        return _attrOrder;
    }

    PvtPrim* getChild(const RtToken& name)
    {
        auto it = _primMap.find(name);
        return it != _primMap.end() ? it->second->asA<PvtPrim>() : nullptr;
    }

    const PvtPrim* getChild(const RtToken& name) const
    {
        return const_cast<PvtPrim*>(this)->getChild(name);
    }

    RtPrimIterator getChildren(RtObjectPredicate predicate = nullptr) const
    {
        return RtPrimIterator(this->obj(), predicate);
    }

    const PvtDataHandleVec& getAllChildren() const
    {
        return _primOrder;
    }

    PvtAllocator& getAllocator()
    {
        return _allocator;
    }

    RtToken makeUniqueName(const RtToken& name) const;

private:
    static const RtObjType _typeId;
    static const RtToken _typeName;

protected:
    PvtPrim(const RtToken& name, PvtPrim* parent);

    void addChildPrim(const PvtPrim* prim);
    void removeChildPrim(const PvtPrim* prim);

    PvtDataHandleMap _attrMap;
    PvtDataHandleVec _attrOrder;

    PvtDataHandleMap _primMap;
    PvtDataHandleVec _primOrder;

    PvtAllocator _allocator;

    friend class PvtStage;
    friend class RtGraphIterator;
};

}

#endif
