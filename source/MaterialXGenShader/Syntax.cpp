#include <MaterialXGenShader/Syntax.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXCore/Value.h>

namespace MaterialX
{

Syntax::Syntax()
{
}

void Syntax::registerTypeSyntax(const TypeDesc* type, TypeSyntaxPtr syntax)
{
    auto it = _typeSyntaxByType.find(type);
    if (it != _typeSyntaxByType.end())
    {
        _typeSyntaxs[it->second] = syntax;
    }
    else
    {
        _typeSyntaxs.push_back(syntax);
        _typeSyntaxByType[type] = _typeSyntaxs.size() - 1;
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
    return *_typeSyntaxs[it->second];
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

    const vector<string>& srcMembers = srcSyntax.getMembers();

    vector<string> membersSwizzled;

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
    if (_invalidTokens.size())
    {
        name = replaceSubstrings(name, _invalidTokens);
    }
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


const vector<string> TypeSyntax::EMPTY_MEMBERS;

TypeSyntax::TypeSyntax(const string& name, const string& defaultValue, const string& uniformDefaultValue,
    const string& typeAlias, const string& typeDefinition, const vector<string>& members)
    : _name(name)
    , _defaultValue(defaultValue)
    , _uniformDefaultValue(uniformDefaultValue)
    , _typeAlias(typeAlias)
    , _typeDefinition(typeDefinition)
    , _members(members)
{
}


ScalarTypeSyntax::ScalarTypeSyntax(const string& name, const string& defaultValue, const string& uniformDefaultValue, const string& typeAlias, const string& typeDefinition)
    : TypeSyntax(name, defaultValue, uniformDefaultValue, typeAlias, typeDefinition, EMPTY_MEMBERS)
{
}

string ScalarTypeSyntax::getValue(const Value& value, bool /*uniform*/) const
{
    return value.getValueString();
}

string ScalarTypeSyntax::getValue(const vector<string>& values, bool /*uniform*/) const
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


StringTypeSyntax::StringTypeSyntax(const string& name, const string& defaultValue, const string& uniformDefaultValue, const string& typeAlias, const string& typeDefinition)
    : ScalarTypeSyntax(name, defaultValue, uniformDefaultValue, typeAlias, typeDefinition)
{
}

string StringTypeSyntax::getValue(const Value& value, bool /*uniform*/) const
{
    return "\"" + value.getValueString() + "\"";
}


AggregateTypeSyntax::AggregateTypeSyntax(const string& name, const string& defaultValue, const string& uniformDefaultValue,
    const string& typeAlias, const string& typeDefinition, const vector<string>& members)
    : TypeSyntax(name, defaultValue, uniformDefaultValue, typeAlias, typeDefinition, members)
{
}

string AggregateTypeSyntax::getValue(const Value& value, bool /*uniform*/) const
{
    return getName() + "(" + value.getValueString() + ")";
}

string AggregateTypeSyntax::getValue(const vector<string>& values, bool /*uniform*/) const
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
