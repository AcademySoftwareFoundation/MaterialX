//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PRVELEMENT_H
#define MATERIALX_PRVELEMENT_H

#include <MaterialXRuntime/Private/PrvObject.h>

#include <MaterialXRuntime/RtElement.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

using PrvObjectHandleVec = vector<PrvObjectHandle>;
using PrvObjectHandleSet = std::set<PrvObjectHandle>;

// Allocator class handling allocation of data for elements.
// The data allocated is kept by the allocator and freed
// upon allocator destruction or by calling free() explicitly.
// NOTE: Data is stored as raw byte pointers and destructors
// for allocated objects will not be called when freeing data.
class PrvAllocator
{
public:
    ~PrvAllocator()
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


class PrvElement : public PrvObject
{
public:
    virtual ~PrvElement() {}

    const RtToken& getName() const
    {
        return _name;
    }

    void setName(const RtToken& name);

    PrvElement* getParent() const
    {
        return _parent;
    }

    // Note: This should only be called from inside addChild or addPort.
    // Arbitrary reparenting of elements is not supported.
    void setParent(PrvElement* parent)
    {
        _parent = parent;
    }

    PrvElement* getRoot() const;

    void addChild(PrvObjectHandle elem);

    void removeChild(const RtToken& name);

    size_t numChildren() const
    {
        return _children.size();
    }

    PrvObjectHandle getChild(size_t index) const
    {
        return index < _children.size() ? _children[index] : nullptr;
    }

    const PrvObjectHandleVec& getChildren() const
    {
        return _children;
    }

    virtual PrvObjectHandle findChildByName(const RtToken& name) const;

    virtual PrvObjectHandle findChildByPath(const string& path) const;

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

    virtual PrvAllocator& getAllocator();

    static const string PATH_SEPARATOR;

protected:
    PrvElement(RtObjType objType, const RtToken& name);

    // Make a unique name among the element's children.
    RtToken makeUniqueChildName(const RtToken& name) const;

    RtToken _name;
    PrvElement* _parent;
    PrvObjectHandleVec _children;
    RtTokenMap<PrvObjectHandle> _childrenByName;

    using AttrPtr = std::shared_ptr<RtAttribute>;
    vector<AttrPtr> _attributes;
    RtTokenMap<AttrPtr> _attributesByName;

    friend class PrvStage;
};


class PrvAllocatingElement : public PrvElement
{
public:
    PrvAllocator& getAllocator() override
    {
        return _allocator;
    }

protected:
    PrvAllocatingElement(RtObjType objType, const RtToken& name):
        PrvElement(objType, name)
    {}

    PrvAllocator _allocator;
};


class PrvUnknownElement : public PrvElement
{
public:
    static PrvObjectHandle createNew(PrvElement* parent, const RtToken& name, const RtToken& category);

    const RtToken& getCategory() const
    {
        return _category;
    }

protected:
    PrvUnknownElement(const RtToken& name, const RtToken& category);

    const RtToken _category;
};

}

#endif
