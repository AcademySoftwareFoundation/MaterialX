//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTTYPEDEF_H
#define MATERIALX_PVTTYPEDEF_H

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtTypeDef.h>

namespace MaterialX
{

class PvtTypeDef
{
public:
    PvtTypeDef(const RtToken& name, const RtToken& basetype, const RtValueFuncs& funcs, 
               const RtToken& semantic, size_t size);

    const RtToken& getName() const
    {
        return _name;
    }

    const RtToken& getBaseType() const
    {
        return _basetype;
    }

    const RtToken& getSemantic() const
    {
        return _semantic;
    }

    const size_t getSize() const
    {
        return _size;
    }

    const RtValueFuncs& getValueFuncs()
    {
        return _funcs;
    }

    void setComponent(size_t index, const RtToken& name, const RtToken& basetype)
    {
        if (index >= _size)
        {
            throw ExceptionRuntimeError("PvtTypeDef::setComponent: index is out of range for type '" + _name.str() + "'");
        }
        _components[index].name = name;
        _components[index].basetype = basetype;
        _componentIndices[name] = index;
    }

    size_t getComponentIndex(const RtToken& name) const
    {
        auto it = _componentIndices.find(name);
        if (it == _componentIndices.end())
        {
            throw ExceptionRuntimeError("PvtTypeDef::getComponentIndex: no component named " + name.str() + " exists for type '" + _name.str() + "'");
        }
        return it->second;
    }

    const RtToken& getComponentName(size_t index) const
    {
        if (index >= _size)
        {
            throw ExceptionRuntimeError("PvtTypeDef::getComponentName: index is out of range for type '" + _name.str() + "'");
        }
        return _components[index].name;
    }

    const RtToken& getComponentBaseType(size_t index) const
    {
        if (index >= _size)
        {
            throw ExceptionRuntimeError("PvtTypeDef::getComponentBaseType: index is out of range for type '" + _name.str() + "'");
        }
        return _components[index].basetype;
    }

    const RtTokenSet& getValidConnectionTypes() const
    {
        return _connectionTypes;
    }

private:
    struct AggregateComponent
    {
        AggregateComponent() : name(EMPTY_TOKEN), basetype(EMPTY_TOKEN) {}
        RtToken name;
        RtToken basetype;
    };

    const RtToken _name;
    const RtToken _basetype;
    const RtValueFuncs _funcs;
    const RtToken _semantic;
    const size_t _size;
    vector<AggregateComponent> _components;
    RtTokenMap<size_t> _componentIndices;
    RtTokenSet _connectionTypes;
};

class PvtTypeDefRegistry
{
public:
    PvtTypeDefRegistry();

    RtTypeDef* newType(const RtToken& name, const RtToken& basetype, const RtValueFuncs& funcs,
        const RtToken& sematic = RtTypeDef::SEMANTIC_NONE, size_t size = 1);

    size_t numTypes()
    {
        return _types.size();
    }

    const RtTypeDef* getType(size_t index)
    {
        return index < _types.size() ? _types[index].get() : nullptr;
    }

    const RtTypeDef* findType(const RtToken& name)
    {
        auto it = _typesByName.find(name);
        return it != _typesByName.end() ? it->second : nullptr;
    }

    static PvtTypeDefRegistry& get()
    {
        static PvtTypeDefRegistry _registry;
        return _registry;
    }

private:
    using RtTypeDefPtr = std::unique_ptr<RtTypeDef>;
    vector<RtTypeDefPtr> _types;
    RtTokenMap<RtTypeDef*> _typesByName;
};

} // namespace MaterialX

#endif
