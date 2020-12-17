//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenOgsFx/OgsFxSyntax.h>

#include <MaterialXGenShader/Shader.h>

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
            const string& typeAlias = EMPTY_STRING, const string& typeDefinition = EMPTY_STRING,
            const StringVec& members = EMPTY_MEMBERS)
            : AggregateTypeSyntax(name, defaultValue, uniformDefaultValue, typeAlias, typeDefinition, members)
        {}

        string getValue(const Value& value, bool uniform) const override
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

        string getValue(const StringVec& values, bool uniform) const override
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
        Type::COLOR3,
        std::make_shared<OgsFxAggregateTypeSyntax>(
            "vec3", 
            "vec3(0.0)", 
            "{0.0, 0.0, 0.0}", 
            EMPTY_STRING,
            EMPTY_STRING,
            VEC3_MEMBERS)
    );

    registerTypeSyntax
    (
        Type::COLOR4,
        std::make_shared<OgsFxAggregateTypeSyntax>(
            "vec4", 
            "vec4(0.0)", 
            "{0.0, 0.0, 0.0, 0.0}", 
            EMPTY_STRING,
            EMPTY_STRING,
            VEC4_MEMBERS)
    );

    registerTypeSyntax
    (
        Type::VECTOR2,
        std::make_shared<OgsFxAggregateTypeSyntax>(
            "vec2", 
            "vec2(0.0)", 
            "{0.0, 0.0}", 
            EMPTY_STRING,
            EMPTY_STRING,
            VEC2_MEMBERS)
    );

    registerTypeSyntax
    (
        Type::VECTOR3,
        std::make_shared<OgsFxAggregateTypeSyntax>(
            "vec3", 
            "vec3(0.0)", 
            "{0.0, 0.0, 0.0}", 
            EMPTY_STRING,
            EMPTY_STRING,
            VEC3_MEMBERS)
    );

    registerTypeSyntax
    (
        Type::VECTOR4,
        std::make_shared<OgsFxAggregateTypeSyntax>(
            "vec4", 
            "vec4(0.0)", 
            "{0.0, 0.0, 0.0, 0.0}", 
            EMPTY_STRING,
            EMPTY_STRING,
            VEC4_MEMBERS)
    );

    registerTypeSyntax
    (
        Type::MATRIX33,
        std::make_shared<OgsFxAggregateTypeSyntax>(
            "mat3", 
            "mat3(1.0)", 
            "{1, 0, 0, 0, 1, 0, 0, 0, 1}")
    );

    registerTypeSyntax
    (
        Type::MATRIX44,
        std::make_shared<OgsFxAggregateTypeSyntax>(
            "mat4", 
            "mat4(1.0)", 
            "{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}")
    );
}

}
