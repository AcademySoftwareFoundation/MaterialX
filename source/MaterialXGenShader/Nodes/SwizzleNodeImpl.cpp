#include <MaterialXGenShader/Nodes/SwizzleNodeImpl.h>
#include <MaterialXGenShader/HwShader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

ShaderNodeImplPtr SwizzleNodeImpl::create()
{
    return std::make_shared<SwizzleNodeImpl>();
}

void SwizzleNodeImpl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader)
{
    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

    const ShaderInput* in = node.getInput("in");
    const ShaderInput* channels = node.getInput("channels");
    if (!in || !channels)
    {
        throw ExceptionShaderGenError("Node '" + node.getName() +"' is not a valid swizzle node");
    }

    const Syntax* syntax = shadergen.getSyntax();
    const string& swizzle = channels->value ? channels->value->getValueString() : EMPTY_STRING;

    string variableName;

    if (in->connection)
    {
        variableName = in->connection->name;
        if (!swizzle.empty())
        {
            variableName = syntax->getSwizzledVariable(variableName, in->connection->type, swizzle, node.getOutput()->type);
        }
    }
    else
    {
        if (!in->value)
        {
            throw ExceptionShaderGenError("No connection or value found to swizzle on node '" + node.getName() + "'");
        }

        variableName = in->name;

        shader.beginLine();
        shader.addStr(syntax->getTypeName(in->type) + " " + variableName);
        shader.addStr(" = " + syntax->getValue(in->type, *in->value));
        shader.endLine();

        if (!swizzle.empty())
        {
            variableName = syntax->getSwizzledVariable(variableName, in->type, swizzle, node.getOutput()->type);
        }
    }

    shader.beginLine();
    shadergen.emitOutput(context, node.getOutput(), true, false, shader);
    shader.addStr(" = " + variableName);
    shader.endLine();

    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
