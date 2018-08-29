#include <MaterialXShaderGen/ShaderGenerators/Glsl/OgsFx/OgsFxSyntax.h>
#include <MaterialXShaderGen/Shader.h>

#include <memory>

namespace MaterialX
{

namespace
{
    // OgsFx has different syntax for values at uniform initialization and at 
    // normal usage - {0,0,0} vs vec3(0,0,0)
    // So override the aggregate type syntax to support this
    class OgsFxAggregateTypeSyntax : public AggregateTypeSyntax
    {
    public:
        OgsFxAggregateTypeSyntax(const string& name, const string& defaultValue, const string& uniformDefaultValue,
            const string& typeDefStatement = EMPTY_STRING, const vector<string>& members = EMPTY_MEMBERS)
            : AggregateTypeSyntax(name, defaultValue, uniformDefaultValue, typeDefStatement, members)
        {}

        string OgsFxAggregateTypeSyntax::getValue(const Value& value, bool uniform) const override
        {
            if (uniform)
            {
                return "{" + value.getValueString() + "}";
            }
            else
            {
                return getName() + "(" + value.getValueString() + ")";
            }
        }

        string OgsFxAggregateTypeSyntax::getValue(const vector<string>& values, bool uniform) const override
        {
            if (values.empty())
            {
                throw ExceptionShaderGenError("No values given to construct a value");
            }

            string result = uniform ? "{" : getName() + "(" + values[0];
            for (size_t i = 1; i<values.size(); ++i)
            {
                result += ", " + values[i];
            }
            result += uniform ? "}" : ")";

            return result;
        }
    };
}

OgsFxSyntax::OgsFxSyntax()
{
    //
    // Override the GLSL syntax for aggregate types where the syntax differ
    //

    registerTypeSyntax
    (
        DataType::COLOR2,
        std::make_shared<OgsFxAggregateTypeSyntax>(
            "vec2", 
            "vec2(0.0)", 
            "{0.0, 0.0}", 
            EMPTY_STRING,
            VEC2_MEMBERS)
    );

    registerTypeSyntax
    (
        DataType::COLOR3,
        std::make_shared<OgsFxAggregateTypeSyntax>(
            "vec3", 
            "vec3(0.0)", 
            "{0.0, 0.0, 0.0}", 
            EMPTY_STRING,
            VEC3_MEMBERS)
    );

    registerTypeSyntax
    (
        DataType::COLOR4,
        std::make_shared<OgsFxAggregateTypeSyntax>(
            "vec4", 
            "vec4(0.0)", 
            "{0.0, 0.0, 0.0, 0.0}", 
            EMPTY_STRING,
            VEC4_MEMBERS)
    );

    registerTypeSyntax
    (
        DataType::VECTOR2,
        std::make_shared<OgsFxAggregateTypeSyntax>(
            "vec2", 
            "vec2(0.0)", 
            "{0.0, 0.0}", 
            EMPTY_STRING,
            VEC2_MEMBERS)
    );

    registerTypeSyntax
    (
        DataType::VECTOR3,
        std::make_shared<OgsFxAggregateTypeSyntax>(
            "vec3", 
            "vec3(0.0)", 
            "{0.0, 0.0, 0.0}", 
            EMPTY_STRING,
            VEC3_MEMBERS)
    );

    registerTypeSyntax
    (
        DataType::VECTOR4,
        std::make_shared<OgsFxAggregateTypeSyntax>(
            "vec4", 
            "vec4(0.0)", 
            "{0.0, 0.0, 0.0, 0.0}", 
            EMPTY_STRING,
            VEC4_MEMBERS)
    );

    registerTypeSyntax
    (
        DataType::MATRIX3,
        std::make_shared<OgsFxAggregateTypeSyntax>(
            "mat3", 
            "mat3(1.0)", 
            "{1, 0, 0, 0, 1, 0, 0, 0, 1}")
    );

    registerTypeSyntax
    (
        DataType::MATRIX4,
        std::make_shared<OgsFxAggregateTypeSyntax>(
            "mat4", 
            "mat4(1.0)", 
            "{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}")
    );
}

}
