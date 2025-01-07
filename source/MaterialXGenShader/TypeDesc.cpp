//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/TypeDesc.h>

#include <MaterialXGenShader/ShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

const string TypeDesc::NONE_TYPE_NAME = "none";

const string& TypeDesc::getName() const
{
    return _data ? _data->getName() : NONE_TYPE_NAME;
}

const StructMemberDescVecPtr TypeDesc::getStructMembers() const
{
    return _data ? _data->getStructMembers() : StructMemberDescVecPtr();
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

TypeSystem::TypeSystem()
{
    // Register all the standard types.
    registerType(Type::BOOLEAN);
    registerType(Type::INTEGER);
    registerType(Type::INTEGERARRAY);
    registerType(Type::FLOAT);
    registerType(Type::FLOATARRAY);
    registerType(Type::VECTOR2);
    registerType(Type::VECTOR3);
    registerType(Type::VECTOR4);
    registerType(Type::COLOR3);
    registerType(Type::COLOR4);
    registerType(Type::MATRIX33);
    registerType(Type::MATRIX44);
    registerType(Type::STRING);
    registerType(Type::FILENAME);
    registerType(Type::BSDF);
    registerType(Type::EDF);
    registerType(Type::VDF);
    registerType(Type::SURFACESHADER);
    registerType(Type::VOLUMESHADER);
    registerType(Type::DISPLACEMENTSHADER);
    registerType(Type::LIGHTSHADER);
    registerType(Type::MATERIAL);
}

void TypeSystem::registerType(TypeDesc type)
{
    _types.push_back(type);
    _typesByName[type.getName()] = type;
}

void TypeSystem::registerType(const string& name, uint8_t basetype, uint8_t semantic, uint16_t size, StructMemberDescVecPtr members)
{
    // Allocate a data block and use to initialize a new type description.
    TypeDesc::DataBlockPtr data = std::make_shared<TypeDesc::DataBlock>(name, members);
    const TypeDesc type(name, basetype, semantic, size, data.get());

    _dataBlocks.push_back(data);
    _types.push_back(type);
    _typesByName[name] = type;
}

MATERIALX_NAMESPACE_END
