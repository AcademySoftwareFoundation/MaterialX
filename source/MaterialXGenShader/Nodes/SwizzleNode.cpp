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

void SwizzleNode::emitFunctionCall(ShaderStage& stage, const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen) const
{
BEGIN_SHADER_STAGE(stage, MAIN_STAGE)

    const ShaderInput* in = node.getInput(IN_STRING);
    const ShaderInput* channels = node.getInput(CHANNELS_STRING);
    if (!in || !channels)
    {
        throw ExceptionShaderGenError("Node '" + node.getName() +"' is not a valid swizzle node");
    }
    if (!in->connection && !in->value)
    {
        throw ExceptionShaderGenError("No connection or value found to swizzle on node '" + node.getName() + "'");
    }

    const string& swizzle = channels->value ? channels->value->getValueString() : EMPTY_STRING;
    string variableName = in->connection ? in->connection->variable : in->variable;

    // If the input is unconnected we must declare a variable
    // for it first, in order to swizzle it below.
    if (!in->connection)
    {
        string variableValue = in->value ? shadergen.getSyntax()->getValue(in->type, *in->value) : shadergen.getSyntax()->getDefaultValue(in->type);
        shadergen.emitLine(stage, shadergen.getSyntax()->getTypeName(in->type) + " " + variableName + " = " + variableValue);
    }

    if (!swizzle.empty())
    {
        const TypeDesc* type = in->connection ? in->connection->type : in->type;
        variableName = shadergen.getSyntax()->getSwizzledVariable(variableName, type, swizzle, node.getOutput()->type);
    }

    shadergen.emitLineBegin(stage);
    shadergen.emitOutput(stage, context, node.getOutput(), true, false);
    shadergen.emitString(stage, " = " + variableName);
    shadergen.emitLineEnd(stage);

END_SHADER_STAGE(stage, MAIN_STAGE)
}

bool SwizzleNode::isEditable(const ShaderInput& input) const
{
    return (input.name != CHANNELS_STRING);
}

} // namespace MaterialX
