//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/Syntax.h>
#include <MaterialXGenShader/TypeDesc.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>

#include <MaterialXCore/Value.h>

MATERIALX_NAMESPACE_BEGIN

const string Syntax::NEWLINE = "\n";
const string Syntax::SEMICOLON = ";";
const string Syntax::COMMA = ",";
const string Syntax::INDENTATION = "    ";
const string Syntax::STRING_QUOTE = "\"";
const string Syntax::INCLUDE_STATEMENT = "#include";
const string Syntax::SINGLE_LINE_COMMENT = "// ";
const string Syntax::BEGIN_MULTI_LINE_COMMENT = "/* ";
const string Syntax::END_MULTI_LINE_COMMENT = " */";

const std::unordered_map<char, size_t> Syntax::CHANNELS_MAPPING =
{
    { 'r', 0 }, { 'x', 0 },
    { 'g', 1 }, { 'y', 1 },
    { 'b', 2 }, { 'z', 2 },
    { 'a', 3 }, { 'w', 3 }
};

//
// Syntax methods
//

Syntax::Syntax(TypeSystemPtr typeSystem) :
    _typeSystem(typeSystem)
{
}

void Syntax::registerTypeSyntax(TypeDesc type, TypeSyntaxPtr syntax)
{
    auto it = _typeSyntaxIndexByType.find(type);
    if (it != _typeSyntaxIndexByType.end())
    {
        _typeSyntaxes[it->second] = syntax;
    }
    else
    {
        _typeSyntaxes.push_back(syntax);
        _typeSyntaxIndexByType[type] = _typeSyntaxes.size() - 1;
    }

    // Make this type a restricted name
    registerReservedWords({ syntax->getName() });
}

void Syntax::registerReservedWords(const StringSet& names)
{
    _reservedWords.insert(names.begin(), names.end());
}

void Syntax::registerInvalidTokens(const StringMap& tokens)
{
    _invalidTokens.insert(tokens.begin(), tokens.end());
}

/// Returns the type syntax object for a named type.
/// Throws an exception if a type syntax is not defined for the given type.
const TypeSyntax& Syntax::getTypeSyntax(TypeDesc type) const
{
    auto it = _typeSyntaxIndexByType.find(type);
    if (it == _typeSyntaxIndexByType.end())
    {
        throw ExceptionShaderGenError("No syntax is defined for the given type '" + type.getName() + "'.");
    }
    return *_typeSyntaxes[it->second];
}

string Syntax::getValue(const ShaderPort* port, bool uniform) const
{
    const TypeSyntax& syntax = getTypeSyntax(port->getType());
    return syntax.getValue(port, uniform);
}

string Syntax::getValue(TypeDesc type, const Value& value, bool uniform) const
{
    const TypeSyntax& syntax = getTypeSyntax(type);
    return syntax.getValue(value, uniform);
}

const string& Syntax::getDefaultValue(TypeDesc type, bool uniform) const
{
    const TypeSyntax& syntax = getTypeSyntax(type);
    return syntax.getDefaultValue(uniform);
}

const string& Syntax::getTypeName(TypeDesc type) const
{
    const TypeSyntax& syntax = getTypeSyntax(type);
    return syntax.getName();
}

string Syntax::getOutputTypeName(TypeDesc type) const
{
    const TypeSyntax& syntax = getTypeSyntax(type);
    const string& outputModifier = getOutputQualifier();
    return outputModifier.size() ? outputModifier + " " + syntax.getName() : syntax.getName();
}

const string& Syntax::getTypeAlias(TypeDesc type) const
{
    const TypeSyntax& syntax = getTypeSyntax(type);
    return syntax.getTypeAlias();
}

const string& Syntax::getTypeDefinition(TypeDesc type) const
{
    const TypeSyntax& syntax = getTypeSyntax(type);
    return syntax.getTypeDefinition();
}

bool Syntax::typeSupported(const TypeDesc*) const
{
    return true;
}

string Syntax::getArrayVariableSuffix(TypeDesc type, const Value& value) const
{
    if (type.isArray())
    {
        if (value.isA<vector<float>>())
        {
            const size_t size = value.asA<vector<float>>().size();
            return "[" + std::to_string(size) + "]";
        }
        else if (value.isA<vector<int>>())
        {
            const size_t size = value.asA<vector<int>>().size();
            return "[" + std::to_string(size) + "]";
        }
    }
    return string();
}

static bool isInvalidChar(char c)
{
    return !isalnum(c) && c != '_';
}

void Syntax::makeValidName(string& name) const
{
    std::replace_if(name.begin(), name.end(), isInvalidChar, '_');
    name = replaceSubstrings(name, _invalidTokens);
}

void Syntax::makeIdentifier(string& name, IdentifierMap& identifiers) const
{
    makeValidName(name);

    auto it = identifiers.find(name);
    if (it != identifiers.end())
    {
        // Name is not unique so append the counter and keep
        // incrementing until a unique name is found.
        string name2;
        do
        {
            name2 = name + std::to_string(it->second++);
        } while (identifiers.count(name2));

        name = name2;
    }

    // Save it among the known identifiers.
    identifiers[name] = 1;
}

string Syntax::getVariableName(const string& name, TypeDesc /*type*/, IdentifierMap& identifiers) const
{
    // Default implementation just makes an identifier, but derived
    // classes can override this for custom variable naming.
    string variable = name;
    makeIdentifier(variable, identifiers);
    return variable;
}

bool Syntax::remapEnumeration(const string&, TypeDesc, const string&, std::pair<TypeDesc, ValuePtr>&) const
{
    return false;
}

StructTypeSyntaxPtr Syntax::createStructSyntax(const string& structTypeName, const string& defaultValue,
                                               const string& uniformDefaultValue, const string& typeAlias,
                                               const string& typeDefinition) const
{
    return std::make_shared<StructTypeSyntax>(
        this,
        structTypeName,
        defaultValue,
        uniformDefaultValue,
        typeAlias,
        typeDefinition);
}

const StringVec TypeSyntax::EMPTY_MEMBERS;

TypeSyntax::TypeSyntax(const Syntax* parent, const string& name, const string& defaultValue, const string& uniformDefaultValue,
                       const string& typeAlias, const string& typeDefinition, const StringVec& members) :
    _parent(parent),
    _name(name),
    _defaultValue(defaultValue),
    _uniformDefaultValue(uniformDefaultValue),
    _typeAlias(typeAlias),
    _typeDefinition(typeDefinition),
    _members(members)
{
}

string TypeSyntax::getValue(const ShaderPort* port, bool uniform) const
{
    if (!port || !port->getValue())
    {
        return getDefaultValue(uniform);
    }
    return getValue(*port->getValue(), uniform);
}

ScalarTypeSyntax::ScalarTypeSyntax(const Syntax* parent, const string& name, const string& defaultValue, const string& uniformDefaultValue,
                                   const string& typeAlias, const string& typeDefinition) :
    TypeSyntax(parent, name, defaultValue, uniformDefaultValue, typeAlias, typeDefinition, EMPTY_MEMBERS)
{
}

string ScalarTypeSyntax::getValue(const Value& value, bool /*uniform*/) const
{
    return value.getValueString();
}

StringTypeSyntax::StringTypeSyntax(const Syntax* parent, const string& name, const string& defaultValue, const string& uniformDefaultValue,
                                   const string& typeAlias, const string& typeDefinition) :
    ScalarTypeSyntax(parent, name, defaultValue, uniformDefaultValue, typeAlias, typeDefinition)
{
}

string StringTypeSyntax::getValue(const Value& value, bool /*uniform*/) const
{
    return "\"" + value.getValueString() + "\"";
}

AggregateTypeSyntax::AggregateTypeSyntax(const Syntax* parent, const string& name, const string& defaultValue, const string& uniformDefaultValue,
                                         const string& typeAlias, const string& typeDefinition, const StringVec& members) :
    TypeSyntax(parent, name, defaultValue, uniformDefaultValue, typeAlias, typeDefinition, members)
{
}

string AggregateTypeSyntax::getValue(const Value& value, bool /*uniform*/) const
{
    const string valueString = value.getValueString();
    return valueString.empty() ? valueString : getName() + "(" + valueString + ")";
}

StructTypeSyntax::StructTypeSyntax(const Syntax* parent, const string& name, const string& defaultValue, const string& uniformDefaultValue,
                                   const string& typeAlias, const string& typeDefinition, const StringVec& members) :
    TypeSyntax(parent, name, defaultValue, uniformDefaultValue, typeAlias, typeDefinition, members)
{
}

string StructTypeSyntax::getValue(const Value& value, bool /*uniform*/) const
{
    const AggregateValue& aggValue = static_cast<const AggregateValue&>(value);

    string result = "{";

    string separator = "";
    for (const auto& memberValue : aggValue.getMembers())
    {
        result += separator;
        separator = ";";

        const string& memberTypeName = memberValue->getTypeString();
        TypeDesc memberTypeDesc = _parent->getType(memberTypeName);

        // Recursively use the syntax to generate the output, so we can support nested structs.
        const string valueStr = _parent->getValue(memberTypeDesc, *memberValue, true);

        result += valueStr;
    }

    result += "}";

    return result;
}

MATERIALX_NAMESPACE_END
