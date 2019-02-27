#include <MaterialXGenShader/Nodes/SwizzleNode.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

static const string IN_STRING("in");
static const string CHANNELS_STRING("channels");

ShaderNodeImplPtr SwizzleNode::create()
{
    return std::make_shared<SwizzleNode>();
}

void SwizzleNode::emitFunctionCall(ShaderStage& stage, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const
{
BEGIN_SHADER_STAGE(stage, MAIN_STAGE)

    const ShaderInput* in = node.getInput(IN_STRING);
    const ShaderInput* channels = node.getInput(CHANNELS_STRING);
    if (!in || !channels)
    {
        throw ExceptionShaderGenError("Node '" + node.getName() +"' is not a valid swizzle node");
    }
    if (!in->getConnection() && !in->getValue())
    {
        throw ExceptionShaderGenError("No connection or value found to swizzle on node '" + node.getName() + "'");
    }

    const string& swizzle = channels->getValue() ? channels->getValue()->getValueString() : EMPTY_STRING;
    string variableName = in->getConnection() ? in->getConnection()->getVariable() : in->getVariable();

    // If the input is unconnected we must declare a variable
    // for it first, in order to swizzle it below.
    if (!in->getConnection())
    {
        string variableValue = in->getValue() ? shadergen.getSyntax()->getValue(in->getType(), *in->getValue()) : shadergen.getSyntax()->getDefaultValue(in->getType());
        shadergen.emitLine(stage, shadergen.getSyntax()->getTypeName(in->getType()) + " " + variableName + " = " + variableValue);
    }

    if (!swizzle.empty())
    {
        const TypeDesc* type = in->getConnection() ? in->getConnection()->getType() : in->getType();
        variableName = shadergen.getSyntax()->getSwizzledVariable(variableName, type, swizzle, node.getOutput()->getType());
    }

    shadergen.emitLineBegin(stage);
    shadergen.emitOutput(stage, context, node.getOutput(), true, false);
    shadergen.emitString(stage, " = " + variableName);
    shadergen.emitLineEnd(stage);

END_SHADER_STAGE(stage, MAIN_STAGE)
}

bool SwizzleNode::isEditable(const ShaderInput& input) const
{
    return (input.getName() != CHANNELS_STRING);
}

} // namespace MaterialX
