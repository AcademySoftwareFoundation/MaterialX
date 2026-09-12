//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenOsl/OslNetworkSyntax.h>

#include <MaterialXGenShader/ShaderGenerator.h>

#include <sstream>

MATERIALX_NAMESPACE_BEGIN

namespace
{

class OslNetworkTypeSyntax : public TypeSyntax
{
public:
    OslNetworkTypeSyntax(const Syntax* parent, const string& name) :
        TypeSyntax(parent, name, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING, EMPTY_MEMBERS)
    {
    }
    virtual ~OslNetworkTypeSyntax() = default;

    string getValue(const Value& value, bool /*uniform*/) const override
    {
        return value.getValueString();
    }
};

class OslNetworkConnectionOnlyTypeSyntax : public OslNetworkTypeSyntax, public OslNetworkSyntaxEmit
{
  public:
    OslNetworkConnectionOnlyTypeSyntax(const Syntax* parent, const string& name) :
        OslNetworkTypeSyntax(parent, name)
    {
    }
    virtual ~OslNetworkConnectionOnlyTypeSyntax() = default;

    EmitParamPartVec getEmitParamParts(const string& /*name*/, TypeDesc /*typeDesc*/, const Value& /*value*/) const override
    {
        throw ExceptionShaderGenError("Param set for connection only param");
    }
};

class OslNetworkScalarTypeSyntax : public OslNetworkTypeSyntax, public OslNetworkSyntaxEmit
{
  public:
    OslNetworkScalarTypeSyntax(const Syntax* parent, const string& name) :
        OslNetworkTypeSyntax(parent, name)
    {
    }
    virtual ~OslNetworkScalarTypeSyntax() = default;

    EmitParamPartVec getEmitParamParts(const string& name, TypeDesc /*typeDesc*/, const Value& value) const override
    {
        return { { _name, name, getValue(value, true) } };
    }
};

class OslNetworkBooleanTypeSyntax : public OslNetworkScalarTypeSyntax
{
  public:
    OslNetworkBooleanTypeSyntax(const Syntax* parent) :
        OslNetworkScalarTypeSyntax(parent, "int")
    {
    }

    string getValue(const Value& value, bool /*uniform*/) const override
    {
        return value.asA<bool>() ? "1" : "0";
    }
};

class OslNetworkMatrix3TypeSyntax : public OslNetworkScalarTypeSyntax
{
  public:
    OslNetworkMatrix3TypeSyntax(const Syntax* parent, const string& name) :
        OslNetworkScalarTypeSyntax(parent, name)
    {
    }

    string getValue(const Value& value, bool /*uniform*/) const override
    {
        StringVec values = splitString(value.getValueString(), ",");
        if (values.empty())
        {
            throw ExceptionShaderGenError("No values given to construct a value");
        }

        StringStream ss;
        for (size_t i = 0; i < values.size(); i++)
        {
            ss << values[i] << " ";
            if ((i + 1) % 3 == 0)
            {
                ss << "0.000"
                    << " ";
            }
        }
        static string ROW_4("0.000  0.000  0.000  1.000");
        ss << ROW_4;

        return ss.str();
    }
};

template <class T>
class OslNetworkVectorStructTypeSyntax : public OslNetworkTypeSyntax, public OslNetworkSyntaxEmit
{
  public:
    OslNetworkVectorStructTypeSyntax(const Syntax* parent, const string& name) :
        OslNetworkTypeSyntax(parent, name)
    {
    }

    EmitParamPartVec getEmitParamParts(const string& name, TypeDesc typeDesc, const Value& value) const override;
};

template <>
OslNetworkSyntaxEmit::EmitParamPartVec OslNetworkVectorStructTypeSyntax<Vector2>::getEmitParamParts(const string& name, TypeDesc /*typeDesc*/, const Value& value) const
{
    const TypedValue<Vector2>& vec2Value = static_cast<const TypedValue<Vector2>&>(value);
    const Vector2& vec2 = vec2Value.getData();
    return {
        { "float", name + ".x", std::to_string(vec2[0]) },
        { "float", name + ".y", std::to_string(vec2[1]) },
    };
}

template <>
OslNetworkSyntaxEmit::EmitParamPartVec OslNetworkVectorStructTypeSyntax<Vector4>::getEmitParamParts(const string& name, TypeDesc /*typeDesc*/, const Value& value) const
{
    const TypedValue<Vector4>& vec4Value = static_cast<const TypedValue<Vector4>&>(value);
    const Vector4& vec4 = vec4Value.getData();
    return {
        { "float", name + ".x", std::to_string(vec4[0]) },
        { "float", name + ".y", std::to_string(vec4[1]) },
        { "float", name + ".z", std::to_string(vec4[2]) },
        { "float", name + ".w", std::to_string(vec4[3]) },
    };
}

template <>
OslNetworkSyntaxEmit::EmitParamPartVec OslNetworkVectorStructTypeSyntax<Color4>::getEmitParamParts(const string& name, TypeDesc /*typeDesc*/, const Value& value) const
{
    const TypedValue<Color4>& col4Value = static_cast<const TypedValue<Color4>&>(value);
    const Color4& col4 = col4Value.getData();
    return {
        { "color", name + ".rgb", std::to_string(col4[0]) + " " + std::to_string(col4[1]) + " " + std::to_string(col4[2]) },
        { "float", name + ".a", std::to_string(col4[3]) },
    };
}

class OslNetworkFloatTupleTypeSyntax : public OslNetworkScalarTypeSyntax
{
public:
    OslNetworkFloatTupleTypeSyntax(const Syntax* parent, const string& name) :
        OslNetworkScalarTypeSyntax(parent, name)
    {
    }

    string getValue(const Value& value, bool /*uniform*/) const override
    {
        string valStr = value.getValueString();
        return replaceSubstrings(valStr, { { ",", " " } });
    }
};

template <class T>
class OslNetworkArrayTypeSyntax : public OslNetworkScalarTypeSyntax
{
  public:
    OslNetworkArrayTypeSyntax(const Syntax* parent, const string& name) :
        OslNetworkScalarTypeSyntax(parent, name)
    {
    }

    EmitParamPartVec getEmitParamParts(const string& /*name*/, TypeDesc /*typeDesc*/, const Value& /*value*/) const override
    {
        throw ExceptionShaderGenError("OSL Network array output unimplemented - no nodes use array inputs currently");
        return {};
    }

    string getValue(const Value& value, bool uniform) const override
    {
        if (!isEmpty(value))
        {
            return "{" + value.getValueString() + "}";
        }
        // OSL disallows arrays without initialization when specified as input uniform
        else if (uniform)
        {
            throw ExceptionShaderGenError("Uniform array cannot initialize to a empty value.");
        }
        return EMPTY_STRING;
    }

  protected:
    bool isEmpty(const Value& value) const
    {
        vector<T> valueArray = value.asA<vector<T>>();
        return valueArray.empty();
    }
};

} // anonymous namespace

//
// OslNetworkSyntax methods
//

OslNetworkSyntax::OslNetworkSyntax(TypeSystemPtr typeSystem) :
    Syntax(typeSystem)
{
    // We don't add any reserved words to the OSL Network syntax
    // even the three "words" used in the construction of the OSL
    // shader group string, "shader", "param" and "connect, are not
    // reserved. The structure of the command string ensures
    // no ambiguity if they are used as names.

    //
    // Register type syntax handlers for each data type.
    //

    registerTypeSyntax(
        Type::FLOAT,
        std::make_shared<OslNetworkScalarTypeSyntax>(
            this,
            "float"));

    registerTypeSyntax(
        Type::FLOATARRAY,
        std::make_shared<OslNetworkArrayTypeSyntax<float>>(
            this,
            "float"));

    registerTypeSyntax(
        Type::INTEGER,
        std::make_shared<OslNetworkScalarTypeSyntax>(
            this,
            "int"));

    registerTypeSyntax(
        Type::INTEGERARRAY,
        std::make_shared<OslNetworkArrayTypeSyntax<int>>(
            this,
            "int"));

    registerTypeSyntax(
        Type::BOOLEAN,
        std::make_shared<OslNetworkBooleanTypeSyntax>(this));

    registerTypeSyntax(
        Type::COLOR3,
        std::make_shared<OslNetworkFloatTupleTypeSyntax>(
            this,
            "color"));

    registerTypeSyntax(
        Type::COLOR4,
        std::make_shared<OslNetworkVectorStructTypeSyntax<Color4>>(
            this,
            "color4"));

    registerTypeSyntax(
        Type::VECTOR2,
        std::make_shared<OslNetworkVectorStructTypeSyntax<Vector2>>(
            this,
            "vector2"));

    registerTypeSyntax(
        Type::VECTOR3,
        std::make_shared<OslNetworkFloatTupleTypeSyntax>(
            this,
            "vector"));

    registerTypeSyntax(
        Type::VECTOR4,
        std::make_shared<OslNetworkVectorStructTypeSyntax<Vector4>>(
            this,
            "vector4"));

    registerTypeSyntax(
        Type::MATRIX33,
        std::make_shared<OslNetworkMatrix3TypeSyntax>(
            this,
            "matrix"));

    registerTypeSyntax(
        Type::MATRIX44,
        std::make_shared<OslNetworkFloatTupleTypeSyntax>(
            this,
            "matrix"));

    registerTypeSyntax(
        Type::STRING,
        std::make_shared<OslNetworkScalarTypeSyntax>(
            this,
            "string"));

    registerTypeSyntax(
        Type::FILENAME,
        std::make_shared<OslNetworkScalarTypeSyntax>(
            this,
            "string"));

    registerTypeSyntax(
        Type::BSDF,
        std::make_shared<OslNetworkConnectionOnlyTypeSyntax>(
            this,
            "BSDF"));

    registerTypeSyntax(
        Type::EDF,
        std::make_shared<OslNetworkConnectionOnlyTypeSyntax>(
            this,
            "EDF"));

    registerTypeSyntax(
        Type::VDF,
        std::make_shared<OslNetworkConnectionOnlyTypeSyntax>(
            this,
            "VDF"));

    registerTypeSyntax(
        Type::SURFACESHADER,
        std::make_shared<OslNetworkConnectionOnlyTypeSyntax>(
            this,
            "surfaceshader"));

    registerTypeSyntax(
        Type::VOLUMESHADER,
        std::make_shared<OslNetworkConnectionOnlyTypeSyntax>(
            this,
            "volumeshader"));

    registerTypeSyntax(
        Type::DISPLACEMENTSHADER,
        std::make_shared<OslNetworkConnectionOnlyTypeSyntax>(
            this,
            "displacementshader"));

    registerTypeSyntax(
        Type::LIGHTSHADER,
        std::make_shared<OslNetworkConnectionOnlyTypeSyntax>(
            this,
            "lightshader"));

    registerTypeSyntax(
        Type::MATERIAL,
        std::make_shared<OslNetworkConnectionOnlyTypeSyntax>(
            this,
            "MATERIAL"));
}

StructTypeSyntaxPtr OslNetworkSyntax::createStructSyntax(const string& structTypeName, const string& defaultValue,
                                                         const string& uniformDefaultValue, const string& typeAlias,
                                                         const string& typeDefinition) const
{
    return std::make_shared<OslNetworkStructTypeSyntax>(
        this,
        structTypeName,
        defaultValue,
        uniformDefaultValue,
        typeAlias,
        typeDefinition);
}

OslNetworkStructTypeSyntax::OslNetworkStructTypeSyntax(const Syntax* parent, const string& name, const string& defaultValue, const string& uniformDefaultValue,
                                                       const string& typeAlias, const string& typeDefinition,
                                                       const StringVec& members) :
    StructTypeSyntax(parent, name, defaultValue, uniformDefaultValue, typeAlias, typeDefinition, members)
{
}

OslNetworkSyntaxEmit::EmitParamPartVec OslNetworkStructTypeSyntax::getEmitParamParts(const string& name, TypeDesc typeDesc, const Value& value) const
{
    EmitParamPartVec result;

    const AggregateValue& aggregateValue = dynamic_cast<const AggregateValue&>(value);

    StructMemberDescVecPtr structMembers = typeDesc.getStructMembers();
    if (!structMembers)
    {
        // we expect to get members in a struct - raise an error here?
        throw ExceptionShaderGenError("Expected to find struct members.");
    }

    unsigned int memberIndex = 0;
    for (const auto& member : *structMembers)
    {
        TypeDesc memberType = member.getType();
        ConstValuePtr memberValue = aggregateValue.getMemberValue(memberIndex);

        const TypeSyntax* memberTypeSyntax = &(_parent->getTypeSyntax(memberType));
        const OslNetworkSyntaxEmit* memberOslTypeSyntaxPtr = dynamic_cast<const OslNetworkSyntaxEmit*>(memberTypeSyntax);
        if (!memberOslTypeSyntaxPtr)
        {
            throw ExceptionShaderGenError("Could not cast to OslNetworkSyntaxEmit syntax type");
        }

        // Get the respective param parts for each member of the struct
        // and then add to the final result prefixing the name with the name of the struct currently being processed.
        EmitParamPartVec emitParamParts = memberOslTypeSyntaxPtr->getEmitParamParts(member.getName(), memberType, *memberValue);
        for (const EmitParamPart& paramPart : emitParamParts)
        {
            result.emplace_back(EmitParamPart(paramPart.typeName, name + "." + paramPart.paramName, paramPart.paramValue));
        }

        memberIndex++;
    }

    return result;
}

OslNetworkStructTypeSyntax::~OslNetworkStructTypeSyntax() { }

MATERIALX_NAMESPACE_END
