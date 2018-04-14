#include <MaterialXShaderGen/ShaderGenerators/Common/Swizzle.h>
#include <MaterialXShaderGen/HwShader.h>
#include <MaterialXShaderGen/ShaderGenerator.h>

namespace MaterialX
{

SgImplementationPtr Swizzle::creator()
{
    return std::make_shared<Swizzle>();
}

void Swizzle::emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader)
{
    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

    const SgInput* in = node.getInput("in");
    const SgInput* channels = node.getInput("channels");
    if (!in || !channels)
    {
        throw ExceptionShaderGenError("Node '" + node.getName() +"' is not a valid swizzle node");
    }

    const SyntaxPtr& syntax = shadergen.getSyntax();
    const string& swizzle = channels->value ? channels->value->getValueString() : EMPTY_STRING;

    string variableName;

    if (in->connection)
    {
        variableName = shadergen.getVariableName(in->connection);
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

        variableName = shadergen.getVariableName(in);

        shader.beginLine();
        shader.addStr(syntax->getTypeName(in->type) + " " + variableName);
        shader.addStr(" = " + syntax->getValue(*in->value, in->type));
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

    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
