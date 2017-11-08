#include <MaterialXShaderGen/Implementations/Swizzle.h>
#include <MaterialXShaderGen/Shader.h>
#include <MaterialXShaderGen/ShaderGenerator.h>

namespace MaterialX
{

SgImplementationPtr Swizzle::creator()
{
    return std::make_shared<Swizzle>();
}

void Swizzle::emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader, int, ...)
{
    const SgInput* in = node.getInput("in");
    const SgInput* channels = node.getInput("channels");
    if (!in || !channels)
    {
        throw ExceptionShaderGenError("Node '" + node.getName() +"' is not a valid swizzle node");
    }

    const SyntaxPtr& syntax = shadergen.getSyntax();
    const string& swizzle = channels->value->getValueString();

    string variableName;

    if (in->connection)
    {
        variableName = syntax->getVariableName(in->connection);
        if (!swizzle.empty())
        {
            variableName = syntax->getSwizzledVariable(variableName, node.getOutput()->type, in->connection->type, swizzle);
        }
    }
    else
    {
        if (!in->value)
        {
            throw ExceptionShaderGenError("No connection or value found to swizzle on node '" + node.getName() + "'");
        }

        variableName = syntax->getVariableName(in);

        shader.beginLine();
        shader.addStr(syntax->getTypeName(in->type) + " " + variableName);
        shader.addStr(" = " + syntax->getValue(*in->value));
        shader.endLine();

        if (!swizzle.empty())
        {
            variableName = syntax->getSwizzledVariable(variableName, node.getOutput()->type, in->type, swizzle);
        }
    }

    shader.beginLine();
    shadergen.emitOutput(node.getOutput(), true, shader);
    shader.addStr(" = " + variableName);
    shader.endLine();
}

} // namespace MaterialX
