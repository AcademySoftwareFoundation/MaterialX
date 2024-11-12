//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/TypeDesc.h>

#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>

MATERIALX_NAMESPACE_BEGIN

const string TypeDesc::NONE_TYPE_NAME = "none";

const string& TypeDesc::getName(const GenContext& context) const
{
    return context.getTypeDescName(*this);
}

ValuePtr TypeDesc::createValueFromStrings(const string& value, const GenContext& context) const
{
    const auto& typeName = getName(context);
    ValuePtr newValue = Value::createValueFromStrings(value, typeName);
    auto structMemberDescs = getStructMembers();
    if (!isStruct() || !structMemberDescs)
        return newValue;

    // Value::createValueFromStrings() can only create a valid Value for a struct if it is passed
    // the optional TypeDef argument, otherwise it just returns a "string" typed Value.
    // So if this is a struct type we need to create a new AggregateValue.

    StringVec subValues = parseStructValueString(value);
    AggregateValuePtr result = AggregateValue::createAggregateValue(typeName);

    if (subValues.size() != structMemberDescs->size())
    {
        std::stringstream ss;
        ss << "Wrong number of initializers - expect " << structMemberDescs->size();
        throw ExceptionShaderGenError(ss.str());
    }

    for (size_t i = 0; i < structMemberDescs->size(); ++i)
    {
        result->appendValue( structMemberDescs->at(i).getTypeDesc().createValueFromStrings(subValues[i], context));
    }

    return result;
}

MATERIALX_NAMESPACE_END
