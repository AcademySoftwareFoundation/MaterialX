#include <MaterialXGenShader/Nodes/ConvertNode.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/TypeDesc.h>

namespace MaterialX
{

namespace
{
    using ConvertTable = std::unordered_map<const TypeDesc*, std::unordered_map<const TypeDesc*, std::string> >;

    static const ConvertTable CONVERT_TABLE({
        { 
            Type::COLOR2,
            {
                { Type::VECTOR2, std::string("ra") }
            }
        },
        {
            Type::COLOR3,
            {
                { Type::VECTOR3, std::string("rgb") },
                { Type::COLOR4, std::string("rgb1") }
            }
        },
        {
            Type::COLOR4,
            {
                { Type::VECTOR4, std::string("rgba") },
                { Type::COLOR3, std::string("rgb") }
            }
        },
        {
            Type::VECTOR2,
            {
                { Type::COLOR2, std::string("xy") }
            }
        },
        {
            Type::VECTOR3,
            {
                { Type::COLOR3, std::string("xyz") }
            }
        },
        {
            Type::VECTOR4,
            {
                { Type::COLOR4, std::string("xyzw") }
            }
        },
        {
            Type::FLOAT,
            {
                { Type::COLOR2, std::string("rr") },
                { Type::COLOR3, std::string("rrr") },
                { Type::COLOR4, std::string("rrrr") },
                { Type::VECTOR2, std::string("rr") },
                { Type::VECTOR3, std::string("rrr") },
                { Type::VECTOR4, std::string("rrrr") },
            }
        }
    });

    static const string IN_STRING("in");
}

ShaderNodeImplPtr ConvertNode::create()
{
    return std::make_shared<ConvertNode>();
}

void ConvertNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader)
{
    BEGIN_SHADER_STAGE(shader, Shader::PIXEL_STAGE)

    const ShaderInput* in = node.getInput(IN_STRING);
    const ShaderOutput* out = node.getOutput();
    if (!in || !out)
    {
        throw ExceptionShaderGenError("Node '" + node.getName() + "' is not a valid convert node");
    }
    if (!in->connection && !in->value)
    {
        throw ExceptionShaderGenError("No connection or value found to convert on node '" + node.getName() + "'");
    }

    string result;

    // Handle supported scalar type conversions.
    if ((in->type == Type::BOOLEAN || in->type == Type::INTEGER) && out->type == Type::FLOAT)
    {
        shadergen.getInput(context, in, result);
        result = shadergen.getSyntax()->getTypeName(out->type) + "(" + result + ")";
    }
    // Handle supported vector type conversions.
    else
    {
        // Search the conversion table for a swizzle pattern to use.
        const string* swizzle = nullptr;
        auto i = CONVERT_TABLE.find(in->type);
        if (i != CONVERT_TABLE.end())
        {
            auto j = i->second.find(out->type);
            if (j != i->second.end())
            {
                swizzle = &j->second;
            }
        }
        if (!swizzle || swizzle->empty())
        {
            throw ExceptionShaderGenError("Conversion from '" + in->type->getName() + "' to '" + out->type->getName() + "' is not supported by convert node");
        }

        string variableName = in->connection ? in->connection->variable : in->variable;

        // If the input is unconnected we must declare a local variable
        // for it first, in order to swizzle it below.
        if (!in->connection)
        {
            string variableValue = in->value ? shadergen.getSyntax()->getValue(in->type, *in->value) : shadergen.getSyntax()->getDefaultValue(in->type);
            shader.addLine(shadergen.getSyntax()->getTypeName(in->type) + " " + variableName + " = " + variableValue);
        }
        const TypeDesc* type = in->connection ? in->connection->type : in->type;
        result = shadergen.getSyntax()->getSwizzledVariable(variableName, type, *swizzle, node.getOutput()->type);
    }

    shader.beginLine();
    shadergen.emitOutput(context, node.getOutput(), true, false, shader);
    shader.addStr(" = " + result);
    shader.endLine();

    END_SHADER_STAGE(shader, Shader::PIXEL_STAGE)
}

} // namespace MaterialX
