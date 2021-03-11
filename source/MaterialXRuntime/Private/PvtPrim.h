//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTPRIM_H
#define MATERIALX_PVTPRIM_H

#include <MaterialXRuntime/Private/PvtPort.h>
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

class RtPrimIterator;
class PvtNodeGraphPrim;

class PvtPrim : public PvtObject
{
    RT_DECLARE_RUNTIME_OBJECT(PvtPrim)

public:
    template<class T = PvtPrim>
    static PvtObjHandle createNew(const RtTypeInfo* type, const RtToken& name, PvtPrim* parent)
    {
        // Make the name unique.
        const RtToken primName = parent->makeUniqueChildName(name);
        return PvtObjHandle(new T(type, primName, parent));
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

    RtToken renameRelationship(const RtToken& name, const RtToken& newName)
    {
        return _rel.rename(name, newName, this);
    }

    PvtRelationship* getRelationship(const RtToken& name)
    {
        PvtObject* obj = _rel.find(name);
        return obj ? obj->asA<PvtRelationship>() : nullptr;
    }

    const PvtObjectVec& getAllRelationships() const
    {
        return _rel.vec();
    }

    PvtPort* getPort(const RtToken& name) const
    {
        PvtObject* obj = _inputs.find(name);
        if (!obj)
        {
            obj = _outputs.find(name);
        }
        return obj ? obj->asA<PvtPort>() : nullptr;
    }

    PvtInput* createInput(const RtToken& name, const RtToken& type, uint32_t flags = 0);

    void removeInput(const RtToken& name);

    RtToken renameInput(const RtToken& name, const RtToken& newName)
    {
        return _inputs.rename(name, newName, this);
    }

    size_t numInputs() const
    {
        return _inputs.size();
    }

    PvtInput* getInput(size_t index) const
    {
        return _inputs[index]->asA<PvtInput>();
    }

    PvtInput* getInput(const RtToken& name) const
    {
        PvtObject* obj = _inputs.find(name);
        return obj ? obj->asA<PvtInput>() : nullptr;
    }

    const PvtObjectVec& getInputs() const
    {
        return _inputs.vec();
    }

    PvtOutput* createOutput(const RtToken& name, const RtToken& type, uint32_t flags = 0);

    void removeOutput(const RtToken& name);

    RtToken renameOutput(const RtToken& name, const RtToken& newName)
    {
        return _outputs.rename(name, newName, this);
    }

    size_t numOutputs() const
    {
        return _outputs.size();
    }

    PvtOutput* getOutput(size_t index = 0) const
    {
        return _outputs[index]->asA<PvtOutput>();
    }

    PvtOutput* getOutput(const RtToken& name) const
    {
        PvtObject* obj = _outputs.find(name);
        return obj ? obj->asA<PvtOutput>() : nullptr;
    }

    const PvtObjectVec& getOutputs() const
    {
        return _outputs.vec();
    }

    size_t numChildren() const
    {
        return _prims.size();
    }

    PvtPrim* getChild(size_t index) const
    {
        return _prims[index]->asA<PvtPrim>();
    }

    PvtPrim* getChild(const RtToken& name) const
    {
        PvtObject* obj = _prims.find(name);
        return obj ? obj->asA<PvtPrim>() : nullptr;
    }

    RtToken renameChild(const RtToken& name, const RtToken& newName)
    {
        return _prims.rename(name, newName, this);
    }

    RtPrimIterator getChildren(RtObjectPredicate predicate = nullptr) const;

    const PvtObjectVec& getAllChildren() const
    {
        return _prims.vec();
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

    void addChildPrim(PvtPrim* prim);
    void removeChildPrim(PvtPrim* prim);

    const RtTypeInfo* _typeInfo;

    // Relationships
    PvtObjectList _rel;

    // Inputs
    PvtObjectList _inputs;

    // Outputs
    PvtObjectList _outputs;

    // Child prims
    PvtObjectList _prims;

    PvtAllocator _allocator;

    friend class PvtApi;
    friend class PvtStage;
    friend class RtNodeGraph;
    friend class RtPrimIterator;
    friend class RtInputIterator;
    friend class RtOutputIterator;
    friend class RtRelationshipIterator;
};


struct PvtAttributeSpec
{
    RtToken name;
    RtToken type;
    // TODO: Use an RtValue instead of string value.
    //       Need better handling of ownership of large values
    //       for this, which is part of another change list.
    // RtValue value;
    string value;
    bool custom;
    bool exportable;
};

class RtAttributeSpecList
{
public:
    ~RtAttributeSpecList()
    {
        for (RtAttributeSpec* spec : _vec)
        {
            delete spec;
        }
        _map.clear();
        _vec.clear();
    }

    void add(RtAttributeSpec* spec)
    {
        _map[spec->getName()] = spec;
        _vec.push_back(spec);
    }

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

    RtAttributeSpec* find(const RtToken& name) const
    {
        auto it = _map.find(name);
        return it != _map.end() ? it->second : nullptr;
    }

    RtAttributeSpec* operator[](size_t i) const
    {
        return i < _vec.size() ? _vec[i] : nullptr;
    }

private:
    RtTokenMap<RtAttributeSpec*> _map;
    RtAttributeSpecVec _vec;

    friend class PvtPrimSpec;
};


class PvtPrimSpec : public RtPrimSpec
{
public:
    PvtPrimSpec()
    {
    }

    const RtAttributeSpec* getAttribute(const RtToken& name) const override
    {
        return _primAttr.find(name);
    }

    const RtAttributeSpecVec& getAttributes() const override
    {
        return _primAttr._vec;
    }

    const RtAttributeSpec* getPortAttribute(const RtPort& port, const RtToken& name) const override;

    RtAttributeSpecVec getPortAttributes(const RtPort& port) const override;

    RtAttributeSpec* create(const RtToken& name, const RtToken& type, const string& value, bool exportable, bool custom);

    void addPrimAttribute(const RtToken& name, const RtToken& type, const string& value = EMPTY_STRING,
                      bool exportable = false, bool custom = false)
    {
        _primAttr.add(create(name, type, value, exportable, custom));
    }

    void addInputAttribute(const RtToken& name, const RtToken& type,
                           const string& value = EMPTY_STRING, bool exportable = false, bool custom = false)
    {
        _inputAttr.add(create(name, type, value, exportable, custom));
    }

    void addInputAttributeByName(const RtToken& portName, const RtToken& name, const RtToken& type,
                                const string& value = EMPTY_STRING, bool exportable = false, bool custom = false)
    {
        _inputAttrByName[portName].add(create(name, type, value, exportable, custom));
    }

    void addInputAttributeByType(const RtToken& portType, const RtToken& name, const RtToken& type,
                                 const string& value = EMPTY_STRING, bool exportable = false, bool custom = false)
    {
        _inputAttrByType[portType].add(create(name, type, value, exportable, custom));
    }

    void addOutputAttribute(const RtToken& name, const RtToken& type,
                            const string& value = EMPTY_STRING, bool exportable = false, bool custom = false)
    {
        _outputAttr.add(create(name, type, value, exportable, custom));
    }

    void addOutputAttributeByName(const RtToken& portName, const RtToken& name, const RtToken& type,
                                  const string& value = EMPTY_STRING, bool exportable = false, bool custom = false)
    {
        _outputAttrByName[portName].add(create(name, type, value, exportable, custom));
    }

    void addOutputAttributeByType(const RtToken& portType, const RtToken& name, const RtToken& type,
                                  const string& value = EMPTY_STRING, bool exportable = false, bool custom = false)
    {
        _outputAttrByType[portType].add(create(name, type, value, exportable, custom));
    }

    RtAttributeSpecList _primAttr;
    RtAttributeSpecList _inputAttr;
    RtAttributeSpecList _outputAttr;
    RtTokenMap<RtAttributeSpecList> _inputAttrByName;
    RtTokenMap<RtAttributeSpecList> _inputAttrByType;
    RtTokenMap<RtAttributeSpecList> _outputAttrByName;
    RtTokenMap<RtAttributeSpecList> _outputAttrByType;
    PvtAllocator _allocator; // TODO: Start using this allocator, change default value from strings to actual value type.
};

}

#endif
