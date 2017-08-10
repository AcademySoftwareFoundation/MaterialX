#include <MaterialXShaderGen/NodeImplementations/Swizzle.h>
#include <MaterialXShaderGen/Shader.h>
#include <MaterialXShaderGen/ShaderGenerator.h>

namespace MaterialX
{

DEFINE_NODE_IMPLEMENTATION(Swizzle, "swizzle", "", "")

void Swizzle::emitCode(const SgNode& sgnode, ShaderGenerator& shadergen, Shader& shader)
{
    const Node& node = sgnode.getNode();

    const InputPtr in = node.getInput("in");
    const ParameterPtr& channels = node.getParameter("channels");
    if (!in || !channels)
    {
        throw ExceptionShaderGenError("Node '" + node.getName() +"' is not a valid swizzle node");
    }

    const SyntaxPtr& syntax = shadergen.getSyntax();
    const string& swizzle = channels->getValueString();

    string variableName;

    NodePtr connectedNode = in->getConnectedNode();
    if (connectedNode)
    {
        variableName = syntax->getVariableName(*connectedNode);
        if (!swizzle.empty())
        {
            variableName = syntax->getSwizzledVariable(variableName, node.getType(), connectedNode->getType(), swizzle);
        }
    }
    else
    {
        const ValuePtr value = in->getValue();
        if (!value)
        {
            throw ExceptionShaderGenError("No connection or value found to swizzle on node '" + node.getName() + "'");
        }

        variableName = syntax->getVariableName(*in);

        shader.beginLine();
        shader.addStr(syntax->getTypeName(in->getType()) + " " + variableName);
        shader.addStr(" = " + syntax->getValue(*value));
        shader.endLine();

        if (!swizzle.empty())
        {
            variableName = syntax->getSwizzledVariable(variableName, node.getType(), in->getType(), swizzle);
        }
    }

    shader.beginLine();
    shadergen.emitOutput(node, true, shader);
    shader.addStr(" = " + variableName);
    shader.endLine();
}

} // namespace MaterialX
