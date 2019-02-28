#include <MaterialXGenShader/Nodes/ConvertNode.h>
#include <MaterialXGenShader/GenContext.h>
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
                { Type::COLOR2, std::string("xy") },
                { Type::VECTOR3, std::string("xy0") }
            }
        },
        {
            Type::VECTOR3,
            {
                { Type::COLOR3, std::string("xyz") },
                { Type::VECTOR4, std::string("xyz1") },
                { Type::VECTOR2, std::string("xy") }
            }
        },
        {
            Type::VECTOR4,
            {
                { Type::COLOR4, std::string("xyzw") },
                { Type::VECTOR3, std::string("xyz") }
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
                { Type::VECTOR4, std::string("rrrr") }
            }
        }
    });

    static const string IN_STRING("in");
}

ShaderNodeImplPtr ConvertNode::create()
{
    return std::make_shared<ConvertNode>();
}

void ConvertNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, MAIN_STAGE)
        const ShaderGenerator& shadergen = context.getShaderGenerator();

        const ShaderInput* in = node.getInput(IN_STRING);
        const ShaderOutput* out = node.getOutput();
        if (!in || !out)
        {
            throw ExceptionShaderGenError("Node '" + node.getName() + "' is not a valid convert node");
        }
        if (!in->getConnection() && !in->getValue())
        {
            throw ExceptionShaderGenError("No connection or value found to convert on node '" + node.getName() + "'");
        }

        string result;

        // Handle supported scalar type conversions.
        if (in->getType()->isScalar() && out->getType()->isScalar())
        {
            result = shadergen.getUpstreamResult(in, context);
            result = shadergen.getSyntax().getTypeName(out->getType()) + "(" + result + ")";
        }
        // Handle supported vector type conversions.
        else
        {
            // Search the conversion table for a swizzle pattern to use.
            const string* swizzle = nullptr;
            auto i = CONVERT_TABLE.find(in->getType());
            if (i != CONVERT_TABLE.end())
            {
                auto j = i->second.find(out->getType());
                if (j != i->second.end())
                {
                    swizzle = &j->second;
                }
            }
            if (!swizzle || swizzle->empty())
            {
                throw ExceptionShaderGenError("Conversion from '" + in->getType()->getName() + "' to '" + out->getType()->getName() + "' is not supported by convert node");
            }

            string variableName = in->getConnection() ? in->getConnection()->getVariable() : in->getVariable();

            // If the input is unconnected we must declare a local variable
            // for it first, in order to swizzle it below.
            if (!in->getConnection())
            {
                string variableValue = in->getValue() ? shadergen.getSyntax().getValue(in->getType(), *in->getValue()) : shadergen.getSyntax().getDefaultValue(in->getType());
                shadergen.emitLine(shadergen.getSyntax().getTypeName(in->getType()) + " " + variableName + " = " + variableValue, stage);
            }
            const TypeDesc* type = in->getConnection() ? in->getConnection()->getType() : in->getType();
            result = shadergen.getSyntax().getSwizzledVariable(variableName, type, *swizzle, node.getOutput()->getType());
        }

        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        shadergen.emitString(" = " + result, stage);
        shadergen.emitLineEnd(stage);
    END_SHADER_STAGE(stage, MAIN_STAGE)
}

} // namespace MaterialX
