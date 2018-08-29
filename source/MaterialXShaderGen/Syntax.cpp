#include <MaterialXShaderGen/Syntax.h>
#include <MaterialXShaderGen/Shader.h>
#include <MaterialXCore/Value.h>
#include <MaterialXCore/Node.h>

namespace MaterialX
{
    Syntax::Syntax()
    {
    }

    void Syntax::registerTypeSyntax(const string& type, TypeSyntaxPtr syntax)
    {
        auto it = _typeSyntaxByName.find(type);
        if (it != _typeSyntaxByName.end())
        {
            _typeSyntaxs[it->second] = syntax;
        }
        else
        {
            _typeSyntaxs.push_back(syntax);
            _typeSyntaxByName[type] = _typeSyntaxs.size() - 1;
        }

        // Make this type a restricted name
        registerRestrictedNames({ type });
    }

    void Syntax::registerRestrictedNames(const StringSet& names)
    {
        _restrictedNames.insert(names.begin(), names.end());
    }

    /// Returns the type syntax object for a named type.
    /// Throws an exception if a type syntax is not defined for the given type.
    const TypeSyntax& Syntax::getTypeSyntax(const string& type) const
    {
        auto it = _typeSyntaxByName.find(type);
        if (it == _typeSyntaxByName.end())
        {
            throw ExceptionShaderGenError("No syntax is defined for the given type '" + type + "'.");
        }
        return *_typeSyntaxs[it->second];
    }

    string Syntax::getValue(const string& type, const Value& value, bool uniform) const
    {
        const TypeSyntax& syntax = getTypeSyntax(type);
        return syntax.getValue(value, uniform);
    }

    const string& Syntax::getDefaultValue(const string& type, bool uniform) const
    {
        const TypeSyntax& syntax = getTypeSyntax(type);
        return syntax.getDefaultValue(uniform);
    }

    const string& Syntax::getTypeName(const string& type) const
    {
        const TypeSyntax& syntax = getTypeSyntax(type);
        return syntax.getName();
    }

    string Syntax::getOutputTypeName(const string& type) const
    {
        const TypeSyntax& syntax = getTypeSyntax(type);
        const string& outputModifier = getOutputQualifier();
        return outputModifier.size() ? outputModifier + " " + syntax.getName() : syntax.getName();
    }

    const string& Syntax::getTypeDefStatement(const string& type) const
    {
        const TypeSyntax& syntax = getTypeSyntax(type);
        return syntax.getTypeDefStatement();
    }

    string Syntax::getSwizzledVariable(const string& srcName, const string& srcType, const string& channels, const string& dstType) const
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
                if (it->second >= srcMembers.size())
                {
                    throw ExceptionShaderGenError("Given member in channels pattern is incorrect for type '" + srcType + "'.");
                }
                membersSwizzled.push_back(srcName + srcMembers[it->second]);
            }
        }

        return dstSyntax.getValue(membersSwizzled, false);
    }

    void Syntax::makeUnique(string& name, UniqueNameMap& uniqueNames) const
    {
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
        const string& typeDefStatement, const vector<string>& members)
        : _name(name)
        , _defaultValue(defaultValue)
        , _uniformDefaultValue(uniformDefaultValue)
        , _typeDefStatement(typeDefStatement)
        , _members(members)
    {
    }


    ScalarTypeSyntax::ScalarTypeSyntax(const string& name, const string& defaultValue, const string& uniformDefaultValue, const string& typeDefStatement)
        : TypeSyntax(name, defaultValue, uniformDefaultValue, typeDefStatement, EMPTY_MEMBERS)
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
        return values[0];
    }


    StringTypeSyntax::StringTypeSyntax(const string& name, const string& defaultValue, const string& uniformDefaultValue, const string& typeDefStatement)
        : ScalarTypeSyntax(name, defaultValue, uniformDefaultValue, typeDefStatement)
    {
    }

    string StringTypeSyntax::getValue(const Value& value, bool /*uniform*/) const
    {
        return "\"" + value.getValueString() + "\"";
    }


    AggregateTypeSyntax::AggregateTypeSyntax(const string& name, const string& defaultValue, const string& uniformDefaultValue, 
        const string& typeDefStatement, const vector<string>& members)
        : TypeSyntax(name, defaultValue, uniformDefaultValue, typeDefStatement, members)
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

        string result = getName() + "(" + values[0];
        for (size_t i=1; i<values.size(); ++i)
        {
            result += ", " + values[i];
        }
        result += ")";

        return result;
    }


    const string DataType::BOOLEAN = "boolean";
    const string DataType::INTEGER = "integer";
    const string DataType::FLOAT = "float";
    const string DataType::VECTOR2 = "vector2";
    const string DataType::VECTOR3 = "vector3";
    const string DataType::VECTOR4 = "vector4";
    const string DataType::COLOR2 = "color2";
    const string DataType::COLOR3 = "color3";
    const string DataType::COLOR4 = "color4";
    const string DataType::MATRIX3 = "matrix33";
    const string DataType::MATRIX4 = "matrix44";
    const string DataType::STRING = "string";
    const string DataType::FILENAME = "filename";
    const string DataType::BSDF = "BSDF";
    const string DataType::EDF = "EDF";
    const string DataType::VDF = "VDF";
    const string DataType::SURFACE = "surfaceshader";
    const string DataType::VOLUME = "volumeshader";
    const string DataType::DISPLACEMENT = "displacementshader";
    const string DataType::LIGHT = "lightshader";

    const std::set<string> DataType::SCALARS = { DataType::BOOLEAN, DataType::FLOAT, DataType::INTEGER };
    const std::set<string> DataType::TUPLES = { DataType::VECTOR2, DataType::COLOR2 };
    const std::set<string> DataType::TRIPLES = { DataType::VECTOR3, DataType::COLOR3 };
    const std::set<string> DataType::QUADRUPLES = { DataType::VECTOR4, DataType::COLOR4 };
}
