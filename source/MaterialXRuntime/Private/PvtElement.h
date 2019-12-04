//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTELEMENT_H
#define MATERIALX_PVTELEMENT_H

#include <MaterialXRuntime/Private/PvtObject.h>

#include <MaterialXRuntime/RtElement.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

using PvtDataHandleVec = vector<PvtDataHandle>;
using PvtDataHandleSet = std::set<PvtDataHandle>;
using PvtDataHandleMap = RtTokenMap<PvtDataHandle>;

using PvtAttributePtr = std::shared_ptr<RtAttribute>;
using PvtAttributeVec = vector<PvtAttributePtr>;
using PvtAttributeMap = RtTokenMap<PvtAttributePtr>;


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


class PvtElement : public PvtObject
{
public:
    virtual ~PvtElement() {}

    const RtToken& getName() const
    {
        return _name;
    }

    void setName(const RtToken& name);

    PvtElement* getParent() const
    {
        return _parent;
    }

    // Note: This should only be called from inside addChild or addPort.
    // Arbitrary reparenting of elements is not supported.
    void setParent(PvtElement* parent)
    {
        _parent = parent;
    }

    PvtElement* getRoot() const;

    void addChild(const PvtDataHandle& elem);

    void removeChild(const RtToken& name);

    size_t numChildren() const
    {
        return _children.size();
    }

    const PvtDataHandle& getChild(size_t index) const
    {
        return index < _children.size() ? _children[index] : PvtObject::NULL_DATA_HANDLE;
    }

    const PvtDataHandleVec& getChildren() const
    {
        return _children;
    }

    virtual PvtDataHandle findChildByName(const RtToken& name) const;

    virtual PvtDataHandle findChildByPath(const string& path) const;

    RtAttribute* addAttribute(const RtToken& name, const RtToken& type, uint32_t flags = 0);

    void removeAttribute(const RtToken& name);

    const RtAttribute* getAttribute(const RtToken& name) const
    {
        auto it = _attributesByName.find(name);
        return it != _attributesByName.end() ? it->second.get() : nullptr;
    }

    RtAttribute* getAttribute(const RtToken& name)
    {
        auto it = _attributesByName.find(name);
        return it != _attributesByName.end() ? it->second.get() : nullptr;
    }

    const RtAttribute* getAttribute(size_t index) const
    {
        return index < _attributes.size() ? _attributes[index].get() : nullptr;
    }

    RtAttribute* getAttribute(size_t index)
    {
        return index < _attributes.size() ? _attributes[index].get() : nullptr;
    }

    size_t numAttributes() const
    {
        return _attributes.size();
    }

    virtual PvtAllocator& getAllocator();

protected:
    PvtElement(RtObjType objType, const RtToken& name);

    // Make a unique name among the element's children.
    RtToken makeUniqueChildName(const RtToken& name) const;

    PvtAttributePtr createAttribute(const RtToken& name, const RtToken& type, uint32_t flags);

    RtToken _name;
    PvtElement* _parent;
    PvtDataHandleVec _children;
    PvtDataHandleMap _childrenByName;
    PvtAttributeVec _attributes;
    PvtAttributeMap _attributesByName;

    friend class PvtStage;
};


class PvtAllocatingElement : public PvtElement
{
public:
    PvtAllocator& getAllocator() override
    {
        return _allocator;
    }

protected:
    PvtAllocatingElement(RtObjType objType, const RtToken& name):
        PvtElement(objType, name)
    {}

    PvtAllocator _allocator;
};


class PvtUnknownElement : public PvtElement
{
public:
    static PvtDataHandle createNew(PvtElement* parent, const RtToken& name, const RtToken& category);

    const RtToken& getCategory() const
    {
        return _category;
    }

protected:
    PvtUnknownElement(const RtToken& name, const RtToken& category);

    const RtToken _category;
};

}

#endif
