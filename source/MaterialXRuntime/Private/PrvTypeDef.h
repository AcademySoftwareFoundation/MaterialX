//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PRVTYPEDEF_H
#define MATERIALX_PRVTYPEDEF_H

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtTypeDef.h>

namespace MaterialX
{

class PrvTypeDef
{
public:
    using ChannelMap = std::unordered_map<char, int>;

public:
    PrvTypeDef(const RtToken& name, const RtToken& basetype, const RtValueFuncs& funcs, const RtToken& semantic,
        size_t size, const ChannelMap& channelMap = ChannelMap());

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

    ChannelMap& getChannelMap()
    {
        return _channelMap;
    }

    const RtTokenSet& getValidConnectionTypes() const
    {
        return _connectionTypes;
    }

private:
    const RtToken _name;
    const RtToken _basetype;
    const RtValueFuncs _funcs;
    const RtToken _semantic;
    const size_t _size;
    ChannelMap _channelMap;
    RtTokenSet _connectionTypes;
};

class PrvTypeDefRegistry
{
public:
    PrvTypeDefRegistry();

    RtTypeDef* newType(const RtToken& name, const RtToken& basetype, const RtValueFuncs& funcs,
        const RtToken& sematic = RtTypeDef::SEMANTIC_NONE, size_t size = 1,
        const PrvTypeDef::ChannelMap& channelMap = PrvTypeDef::ChannelMap());

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

    static PrvTypeDefRegistry& get()
    {
        static PrvTypeDefRegistry _registry;
        return _registry;
    }

private:
    using RtTypeDefPtr = std::unique_ptr<RtTypeDef>;
    vector<RtTypeDefPtr> _types;
    RtTokenMap<RtTypeDef*> _typesByName;
};

} // namespace MaterialX

#endif
