//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTPRIM_H
#define MATERIALX_PVTPRIM_H

#include <MaterialXRuntime/Private/PvtAttribute.h>
#include <MaterialXRuntime/Private/PvtRelationship.h>

#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtPath.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

// Allocator class handling allocation of data for prims.
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

class RtAttrIterator;
class RtPrimIterator;

class PvtPrim : public PvtObject
{
    RT_DECLARE_RUNTIME_OBJECT(PvtPrim)

public:
    template<class T = PvtPrim>
    static PvtDataHandle createNew(const RtTypeInfo* type, const RtToken& name, PvtPrim* parent)
    {
        // Make the name unique.
        const RtToken primName = parent->makeUniqueChildName(name);
        return PvtDataHandle(new T(type, primName, parent));
    }

    RtPrim prim() const
    {
        return RtPrim(hnd());
    }

    void dispose(bool state);

    void destroy();

    const RtTypeInfo* getTypeInfo() const
    {
        return _typeInfo;
    }

    template<class T>
    bool hasApi() const
    {
        static_assert(std::is_base_of<RtSchemaBase, T>::value,
            "Templated type must be a concrete subclass of RtSchemaBase");
        return _typeInfo->isCompatible(T::typeName());
    }

    PvtRelationship* createRelationship(const RtToken& name);

    void removeRelationship(const RtToken& name);

    void renameRelationship(const RtToken& name, const RtToken& newName);

    PvtRelationship* getRelationship(const RtToken& name)
    {
        auto it = _relMap.find(name);
        return it != _relMap.end() ? it->second->asA<PvtRelationship>() : nullptr;
    }

    const PvtDataHandleVec& getAllRelationships() const
    {
        return _relOrder;
    }

    PvtAttribute* createAttribute(const RtToken& name, const RtToken& type, uint32_t flags = 0);

    PvtInput* createInput(const RtToken& name, const RtToken& type, uint32_t flags = 0);

    PvtOutput* createOutput(const RtToken& name, const RtToken& type, uint32_t flags = 0);

    void removeAttribute(const RtToken& name);

    void setAttributeName(const RtToken& name, const RtToken& newName);
    RtToken renameAttribute(const RtToken& name, const RtToken& newName);

    PvtAttribute* getAttribute(const RtToken& name) const
    {
        auto it = _attrMap.find(name);
        return it != _attrMap.end() ? it->second->asA<PvtAttribute>() : nullptr;
    }

    PvtInput* getInput(const RtToken& name) const
    {
        // TODO: Improve type check and type conversion for RtObject subclasses.
        auto it = _attrMap.find(name);
        return it != _attrMap.end() && it->second->isA<PvtInput>() ?
            it->second->asA<PvtInput>() : nullptr;
    }

    PvtOutput* getOutput(const RtToken& name) const
    {
        // TODO: Improve type check and type conversion for RtObject subclasses.
        auto it = _attrMap.find(name);
        return it != _attrMap.end() && it->second->isA<PvtOutput>() ?
            it->second->asA<PvtOutput>() : nullptr;
    }

    PvtOutput* getOutput() const
    {
        // Return first output found.
        // Iterate backwards since outputs are often created after inputs.
        for (auto it = _attrOrder.rbegin(); it != _attrOrder.rend(); ++it)
        {
            const PvtDataHandle& hnd = *it;
            if (hnd->isA<PvtOutput>())
            {
                return hnd->asA<PvtOutput>();
            }
        }
        return nullptr;
    }

    size_t numInputs() const
    {
        size_t count = 0;
        for (const PvtDataHandle& hnd : _attrOrder)
        {
            count += size_t(hnd->isA<PvtInput>());
        }
        return count;
    }

    size_t numOutputs() const
    {
        size_t count = 0;
        for (const PvtDataHandle& hnd : _attrOrder)
        {
            count += size_t(hnd->isA<PvtOutput>());
        }
        return count;
    }

    RtAttrIterator getAttributes(RtObjectPredicate predicate = nullptr) const;

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

    RtPrimIterator getChildren(RtObjectPredicate predicate = nullptr) const;

    const PvtDataHandleVec& getAllChildren() const
    {
        return _primOrder;
    }

    PvtAllocator& getAllocator()
    {
        return _allocator;
    }

    RtToken makeUniqueChildName(const RtToken& name) const;

    // Validate that typenames match when creating a new prim.
    static void validateCreation(const RtTypeInfo& typeInfo, const RtToken& typeName, const RtToken& name)
    {
        if (typeName != typeInfo.getShortTypeName())
        {
            throw ExceptionRuntimeError("Type names mismatch when creating prim '" + name.str() + "'");
        }
    }

    // Validate that typenames match and that parent path is at the root.
    static void validateCreation(const RtTypeInfo& typeInfo, const RtToken& typeName, const RtToken& name, const RtPath& parentPath)
    {
        validateCreation(typeInfo, typeName, name);

        if (!parentPath.isRoot())
        {
            throw ExceptionRuntimeError("A '" + typeName.str() + "' prim can only be created at the top / root level");
        }
    }

protected:
    PvtPrim(const RtTypeInfo* typeInfo, const RtToken& name, PvtPrim* parent);

    void addChildPrim(const PvtPrim* prim);
    void removeChildPrim(const PvtPrim* prim);

    const RtTypeInfo* _typeInfo;

    // Relationships
    PvtDataHandleMap _relMap;
    PvtDataHandleVec _relOrder;

    // Attributes
    PvtDataHandleMap _attrMap;
    PvtDataHandleVec _attrOrder;

    // Child prims
    PvtDataHandleMap _primMap;
    PvtDataHandleVec _primOrder;

    PvtAllocator _allocator;

    friend class PvtApi;
    friend class PvtStage;
    friend class RtNodeGraph;
    friend class RtRelationshipIterator;
};

}

#endif
