//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/TypeDesc.h>

#include <MaterialXGenShader/ShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{

using DataBlockPtr = std::shared_ptr<TypeDesc::DataBlock>;
using TypeDescMap = std::unordered_map<string, TypeDesc>;

class TypeDescRegistryImpl
{
public:
    void registerType(TypeDesc type)
    {
        _types.push_back(type);
        _typesByName[type.getName()] = type;
    }

    void registerType(const string& name, uint8_t basetype, uint8_t semantic, uint16_t size, StructMemberDescVecPtr members)
    {
        // Allocate a data block and use to initialize a new type description.
        DataBlockPtr data = std::make_shared<TypeDesc::DataBlock>(name, members);
        const TypeDesc type(name, basetype, semantic, size, data.get());

        _dataBlocks.push_back(data);
        _types.push_back(type);
        _typesByName[name] = type;
    }

    void clear()
    {
        _types.clear();
        _typesByName.clear();
        _dataBlocks.clear();
    }

    TypeDesc get(const string& name)
    {
        auto it = _typesByName.find(name);
        return (it != _typesByName.end() ? it->second : Type::NONE);
    }

    TypeDescVec _types;
    TypeDescMap _typesByName;
    vector<DataBlockPtr> _dataBlocks;
};

static TypeDescRegistryImpl s_registryImpl;

} // anonymous namespace

const string TypeDesc::NONE_TYPE_NAME = "none";

const string& TypeDesc::getName() const
{
    return _data ? _data->getName() : NONE_TYPE_NAME;
}

const StructMemberDescVecPtr TypeDesc::getStructMembers() const
{
    return _data ? _data->getStructMembers() : StructMemberDescVecPtr();
}

void TypeDesc::registerType(TypeDesc type)
{
    s_registryImpl.registerType(type);
}

void TypeDesc::registerType(const string& name, uint8_t basetype, uint8_t semantic, uint16_t size, StructMemberDescVecPtr members)
{
    s_registryImpl.registerType(name, basetype, semantic, size, members);
}

const TypeDescVec& TypeDesc::getTypes()
{
    return s_registryImpl._types;
}

TypeDesc TypeDesc::get(const string& name)
{
    return s_registryImpl.get(name);
}



ValuePtr TypeDesc::createValueFromStrings(const string& value) const
{
    if (!isStruct())
    {
        return Value::createValueFromStrings(value, getName());
    }

    // Value::createValueFromStrings() can only create a valid Value for a struct if it is passed
    // the optional TypeDef argument, otherwise it just returns a "string" typed Value.
    // So if this is a struct type we need to create a new AggregateValue.

    const StringVec subValues = parseStructValueString(value);
    auto structMembers = getStructMembers();
    if (!structMembers || (subValues.size() != structMembers->size()))
    {
        std::stringstream ss;
        ss << "Wrong number of initializers for struct type " << getName();
        throw ExceptionShaderGenError(ss.str());
    }

    AggregateValuePtr result = AggregateValue::createAggregateValue(getName());
    for (size_t i = 0; i < structMembers->size(); ++i)
    {
        result->appendValue(structMembers->at(i).getType().createValueFromStrings(subValues[i]));
    }

    return result;
}

MATERIALX_NAMESPACE_END
