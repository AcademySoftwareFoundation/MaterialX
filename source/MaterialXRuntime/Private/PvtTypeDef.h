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
    PvtTypeDef(const RtString& name, const RtString& basetype, const RtValueFuncs& funcs, 
               const RtString& semantic, size_t size);

    const RtString& getName() const
    {
        return _name;
    }

    const RtString& getBaseType() const
    {
        return _basetype;
    }

    const RtString& getSemantic() const
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

    void setComponent(size_t index, const RtString& name, const RtString& basetype)
    {
        if (index >= _size)
        {
            throw ExceptionRuntimeError("PvtTypeDef::setComponent: index is out of range for type '" + _name.str() + "'");
        }
        _components[index].name = name;
        _components[index].basetype = basetype;
        _componentIndices[name] = index;
    }

    size_t getComponentIndex(const RtString& name) const
    {
        auto it = _componentIndices.find(name);
        if (it == _componentIndices.end())
        {
            throw ExceptionRuntimeError("PvtTypeDef::getComponentIndex: no component named " + name.str() + " exists for type '" + _name.str() + "'");
        }
        return it->second;
    }

    const RtString& getComponentName(size_t index) const
    {
        if (index >= _size)
        {
            throw ExceptionRuntimeError("PvtTypeDef::getComponentName: index is out of range for type '" + _name.str() + "'");
        }
        return _components[index].name;
    }

    const RtString& getComponentBaseType(size_t index) const
    {
        if (index >= _size)
        {
            throw ExceptionRuntimeError("PvtTypeDef::getComponentBaseType: index is out of range for type '" + _name.str() + "'");
        }
        return _components[index].basetype;
    }

    const RtStringSet& getValidConnectionTypes() const
    {
        return _connectionTypes;
    }

private:
    struct AggregateComponent
    {
        RtString name;
        RtString basetype;
    };

    const RtString _name;
    const RtString _basetype;
    const RtValueFuncs _funcs;
    const RtString _semantic;
    const size_t _size;
    vector<AggregateComponent> _components;
    RtStringMap<size_t> _componentIndices;
    RtStringSet _connectionTypes;
};

class PvtTypeDefRegistry
{
public:
    PvtTypeDefRegistry();

    RtTypeDef* newType(const RtString& name, const RtString& basetype, const RtValueFuncs& funcs,
        const RtString& sematic = RtTypeDef::SEMANTIC_NONE, size_t size = 1);

    size_t numTypes()
    {
        return _types.size();
    }

    const RtTypeDef* getType(size_t index)
    {
        return index < _types.size() ? _types[index].get() : nullptr;
    }

    const RtTypeDef* findType(const RtString& name)
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
    RtStringMap<RtTypeDef*> _typesByName;
};

} // namespace MaterialX

#endif
