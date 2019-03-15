//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/Syntax.h>
#include <MaterialXGenShader/TypeDesc.h>
#include <MaterialXGenShader/ShaderGenerator.h>

#include <MaterialXCore/Value.h>

namespace MaterialX
{

const string Syntax::NEWLINE = "\n";
const string Syntax::INDENTATION = "    ";
const string Syntax::STRING_QUOTE = "\"";
const string Syntax::INCLUDE_STATEMENT = "#include";
const string Syntax::SINGLE_LINE_COMMENT = "// ";
const string Syntax::BEGIN_MULTI_LINE_COMMENT = "/* ";
const string Syntax::END_MULTI_LINE_COMMENT = " */";

//
// Syntax methods
//

Syntax::Syntax()
{
}

void Syntax::registerTypeSyntax(const TypeDesc* type, TypeSyntaxPtr syntax)
{
    auto it = _typeSyntaxByType.find(type);
    if (it != _typeSyntaxByType.end())
    {
        _typeSyntaxes[it->second] = syntax;
    }
    else
    {
        _typeSyntaxes.push_back(syntax);
        _typeSyntaxByType[type] = _typeSyntaxes.size() - 1;
    }

    // Make this type a restricted name
    registerRestrictedNames({ syntax->getName() });
}

void Syntax::registerRestrictedNames(const StringSet& names)
{
    _restrictedNames.insert(names.begin(), names.end());
}

void Syntax::registerInvalidTokens(const StringMap& tokens)
{
    _invalidTokens.insert(tokens.begin(), tokens.end());
}

/// Returns the type syntax object for a named type.
/// Throws an exception if a type syntax is not defined for the given type.
const TypeSyntax& Syntax::getTypeSyntax(const TypeDesc* type) const
{
    auto it = _typeSyntaxByType.find(type);
    if (it == _typeSyntaxByType.end())
    {
        throw ExceptionShaderGenError("No syntax is defined for the given type '" + type->getName() + "'.");
    }
    return *_typeSyntaxes[it->second];
}

string Syntax::getValue(const TypeDesc* type, const Value& value, bool uniform) const
{
    const TypeSyntax& syntax = getTypeSyntax(type);
    return syntax.getValue(value, uniform);
}

const string& Syntax::getDefaultValue(const TypeDesc* type, bool uniform) const
{
    const TypeSyntax& syntax = getTypeSyntax(type);
    return syntax.getDefaultValue(uniform);
}

const string& Syntax::getTypeName(const TypeDesc* type) const
{
    const TypeSyntax& syntax = getTypeSyntax(type);
    return syntax.getName();
}

string Syntax::getOutputTypeName(const TypeDesc* type) const
{
    const TypeSyntax& syntax = getTypeSyntax(type);
    const string& outputModifier = getOutputQualifier();
    return outputModifier.size() ? outputModifier + " " + syntax.getName() : syntax.getName();
}

const string& Syntax::getTypeAlias(const TypeDesc* type) const
{
    const TypeSyntax& syntax = getTypeSyntax(type);
    return syntax.getTypeAlias();
}

const string& Syntax::getTypeDefinition(const TypeDesc* type) const
{
    const TypeSyntax& syntax = getTypeSyntax(type);
    return syntax.getTypeDefinition();
}

string Syntax::getSwizzledVariable(const string& srcName, const TypeDesc* srcType, const string& channels, const TypeDesc* dstType) const
{
    static const std::unordered_map<char, size_t> s_channelsMapping =
    {
        { 'r', 0 },{ 'x', 0 },
        { 'g', 1 },{ 'y', 1 },
        { 'b', 2 },{ 'z', 2 },
        { 'a', 3 },{ 'w', 3 }
    };

    const TypeSyntax& srcSyntax = getTypeSyntax(srcType);
    const TypeSyntax& dstSyntax = getTypeSyntax(dstType);

    const StringVec& srcMembers = srcSyntax.getMembers();

    StringVec membersSwizzled;

    for (size_t i = 0; i < channels.size(); ++i)
    {
        const char ch = channels[i];
        if (ch == '0' || ch == '1')
        {
            membersSwizzled.push_back(string(1,ch));
            continue;
        }

        auto it = s_channelsMapping.find(ch);
        if (it == s_channelsMapping.end())
        {
            throw ExceptionShaderGenError("Invalid channel pattern '" + channels + "'.");
        }

        if (srcMembers.empty())
        {
            membersSwizzled.push_back(srcName);
        }
        else
        {
            int channelIndex = srcType->getChannelIndex(ch);
            if (channelIndex < 0 || channelIndex >= static_cast<int>(srcMembers.size()))
            {
                throw ExceptionShaderGenError("Given channel index: '" + string(1,ch) + "' in channels pattern is incorrect for type '" + srcType->getName() + "'.");
            }
            membersSwizzled.push_back(srcName + srcMembers[channelIndex]);
        }
    }

    return dstSyntax.getValue(membersSwizzled, false);
}

void Syntax::makeUnique(string& name, UniqueNameMap& uniqueNames) const
{
    makeValidName(name);

    UniqueNameMap::iterator it = uniqueNames.find(name);
    if (it != uniqueNames.end())
    {
        name += std::to_string(++(it->second));
    }
    else
    {
        if (_restrictedNames.count(name))
        {
            uniqueNames[name] = 1;
            name += "1";
        }
        else
        {
            uniqueNames[name] = 0;
        }
    }
}

bool Syntax::typeSupported(const TypeDesc*) const
{
    return true;
}

string Syntax::getArraySuffix(const TypeDesc* type, const Value& value) const
{
    if (type->isArray())
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
    if (_invalidTokens.size())
    {
        name = replaceSubstrings(name, _invalidTokens);
    }
}


const StringVec TypeSyntax::EMPTY_MEMBERS;

TypeSyntax::TypeSyntax(const string& name, const string& defaultValue, const string& uniformDefaultValue,
                       const string& typeAlias, const string& typeDefinition, const StringVec& members) :
    _name(name),
    _defaultValue(defaultValue),
    _uniformDefaultValue(uniformDefaultValue),
    _typeAlias(typeAlias),
    _typeDefinition(typeDefinition),
    _members(members)
{
}


ScalarTypeSyntax::ScalarTypeSyntax(const string& name, const string& defaultValue, const string& uniformDefaultValue,
                                   const string& typeAlias, const string& typeDefinition) :
    TypeSyntax(name, defaultValue, uniformDefaultValue, typeAlias, typeDefinition, EMPTY_MEMBERS)
{
}

string ScalarTypeSyntax::getValue(const Value& value, bool /*uniform*/) const
{
    return value.getValueString();
}

string ScalarTypeSyntax::getValue(const StringVec& values, bool /*uniform*/) const
{
    if (values.empty())
    {
        throw ExceptionShaderGenError("No values given to construct a value");
    }
    // Write the value using a stream to maintain any float formatting set
    // using Value::setFloatFormat() and Value::setFloatPrecision()
    std::stringstream ss;
    ss << values[0];
    return ss.str();
}


StringTypeSyntax::StringTypeSyntax(const string& name, const string& defaultValue, const string& uniformDefaultValue,
                                   const string& typeAlias, const string& typeDefinition) :
    ScalarTypeSyntax(name, defaultValue, uniformDefaultValue, typeAlias, typeDefinition)
{
}

string StringTypeSyntax::getValue(const Value& value, bool /*uniform*/) const
{
    return "\"" + value.getValueString() + "\"";
}


AggregateTypeSyntax::AggregateTypeSyntax(const string& name, const string& defaultValue, const string& uniformDefaultValue,
                                         const string& typeAlias, const string& typeDefinition, const StringVec& members) :
    TypeSyntax(name, defaultValue, uniformDefaultValue, typeAlias, typeDefinition, members)
{
}

string AggregateTypeSyntax::getValue(const Value& value, bool /*uniform*/) const
{
    return getName() + "(" + value.getValueString() + ")";
}

string AggregateTypeSyntax::getValue(const StringVec& values, bool /*uniform*/) const
{
    if (values.empty())
    {
        throw ExceptionShaderGenError("No values given to construct a value");
    }

    // Write the value using a stream to maintain any float formatting set
    // using Value::setFloatFormat() and Value::setFloatPrecision()
    std::stringstream ss;
    ss << getName() << "(" << values[0];
    for (size_t i=1; i<values.size(); ++i)
    {
        ss << ", " << values[i];
    }
    ss << ")";

    return ss.str();
}

}
