#include <MaterialXGenShader/Nodes/Swizzle.h>
#include <MaterialXGenShader/HwShader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

SgImplementationPtr Swizzle::create()
{
    return std::make_shared<Swizzle>();
}

void Swizzle::emitFunctionCall(const SgNode& node, const SgNodeContext& /*context*/, ShaderGenerator& shadergen, Shader& shader)
{
    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

    const SgInput* in = node.getInput("in");
    const SgInput* channels = node.getInput("channels");
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
    shadergen.emitOutput(node.getOutput(), true, false, shader);
    shader.addStr(" = " + variableName);
    shader.endLine();

    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
