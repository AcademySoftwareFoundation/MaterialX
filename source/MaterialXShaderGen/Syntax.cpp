#include <MaterialXShaderGen/Syntax.h>
#include <MaterialXShaderGen/Shader.h>
#include <MaterialXCore/Value.h>
#include <MaterialXCore/Node.h>

namespace MaterialX
{
    Syntax::Syntax()
    {
    }

    void Syntax::addValueConstructSyntax(const string& type, const ValueConstructSyntax& syntax)
    {
        _valueConstructSyntax.push_back(syntax);
        _valueConstructSyntaxByName[type] = _valueConstructSyntax.size() - 1;
    }

    void Syntax::addTypeSyntax(const string& type, const TypeSyntax& syntax)
    {
        _typeSyntax.push_back(syntax);
        _typeSyntaxByName[type] = _typeSyntax.size() - 1;
    }

    string Syntax::getValue(const Value& value, bool paramInit) const
    {
        auto it = _valueConstructSyntaxByName.find(value.getTypeString());
        if (it != _valueConstructSyntaxByName.end())
        {
            const ValueConstructSyntax& def = _valueConstructSyntax[it->second];
            return paramInit ?
                def.paramValueConstructor.first + value.getValueString() + def.paramValueConstructor.second :
                def.valueConstructor.first + value.getValueString() + def.valueConstructor.second;
        }
        return value.getValueString();
    }

    const string& Syntax::getTypeName(const string& type) const
    {
        auto it = _typeSyntaxByName.find(type);
        if (it != _typeSyntaxByName.end())
        {
            return _typeSyntax[it->second].name;
        }
        return type;
    }

    const string& Syntax::getTypeDef(const string& type) const
    {
        auto it = _typeSyntaxByName.find(type);
        if (it != _typeSyntaxByName.end())
        {
            return _typeSyntax[it->second].typeDef;
        }
        return EMPTY_STRING;
    }

    const string& Syntax::getTypeDefault(const string& type, bool paramInit) const
    {
        auto it = _typeSyntaxByName.find(type);
        if (it != _typeSyntaxByName.end())
        {
            return paramInit ? _typeSyntax[it->second].paramDefaultValue : _typeSyntax[it->second].defaultValue;
        }
//        const Value* dv = type.getDefaultValue();
//        return (dv ? getValue(*dv, paramInit) : "");
        // TODO: Do we need default values for our TypeDef elements?
        return EMPTY_STRING;
    }

    const string& Syntax::getOutputTypeName(const string& type) const
    {
        auto it = _typeSyntaxByName.find(type);
        if (it != _typeSyntaxByName.end())
        {
            return _typeSyntax[it->second].outputName;
        }
        return EMPTY_STRING;
    }

    string Syntax::getVariableName(const SgInput* input) const
    {
        return input->node->getName() + "_" + input->name;
    }

    string Syntax::getVariableName(const SgOutput* output) const
    {
        return output->node->getName() + "_" + output->name;
    }

    string Syntax::getSwizzledVariable(const string& name, const string& type, const string& fromType, const string& channels) const
    {
        // Get vector component syntax for the from type, if this is a vector type
        const vector<string>* vectorComponents = nullptr;
        auto it = _valueConstructSyntaxByName.find(fromType);
        if (it != _valueConstructSyntaxByName.end())
        {
            vectorComponents = &(_valueConstructSyntax[it->second].vectorComponents);
        }

        string result;

        const std::pair<string, string>* constructorSyntax = nullptr;
        it = _valueConstructSyntaxByName.find(type);
        if (it != _valueConstructSyntaxByName.end())
        {
            constructorSyntax = &(_valueConstructSyntax[it->second].valueConstructor);
            result = constructorSyntax->first;
        }

        static const std::unordered_map<char, size_t> s_channelsMapping =
        {
            { 'r', 0 },{ 'x', 0 },
            { 'g', 1 },{ 'y', 1 },
            { 'b', 2 },{ 'z', 2 },
            { 'a', 3 },{ 'w', 3 }
        };

        string delim = "";
        for (size_t i = 0; i < channels.size(); ++i)
        {
            const char ch = channels[i];
            auto it2 = s_channelsMapping.find(ch);
            if (it2 == s_channelsMapping.end())
            {
                // Not a vector component so just return the character.
                // '0' and '1' is allowed in the channels syntax for example.
                result += delim + ch;
            }
            else if (vectorComponents)
            {
                size_t index = it2->second;
                if (index >= vectorComponents->size())
                {
                    throw ExceptionShaderGenError("Given vector component in channels pattern is incorrect for type '" + fromType + "'.");
                }
                result += delim + name + (*vectorComponents)[index];
            }
            else
            {
                result += delim + name;
            }
            delim = ", ";
        }

        if (constructorSyntax)
        {
            result += constructorSyntax->second;
        }

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
    const string DataType::STRING = "string";
    const string DataType::FILENAME = "filename";
    const string DataType::BSDF = "BSDF";
    const string DataType::EDF = "EDF";
    const string DataType::VDF = "VDF";
    const string DataType::SURFACE = "surfaceshader";
    const string DataType::VOLUME = "volumeshader";
    const string DataType::DISPLACEMENT = "displacementshader";

    const std::set<string> DataType::SCALARS = { DataType::BOOLEAN, DataType::FLOAT, DataType::INTEGER };
    const std::set<string> DataType::TUPLES = { DataType::VECTOR2, DataType::COLOR2 };
    const std::set<string> DataType::TRIPLES = { DataType::VECTOR3, DataType::COLOR3 };
    const std::set<string> DataType::QUADRUPLES = { DataType::VECTOR4, DataType::COLOR4 };
}
