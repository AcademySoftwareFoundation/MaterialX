#include <MaterialXGenShader/Nodes/SwizzleNode.h>
#include <MaterialXGenShader/HwShader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

static const string IN_STRING("in");
static const string CHANNELS_STRING("channels");

ShaderNodeImplPtr SwizzleNode::create()
{
    return std::make_shared<SwizzleNode>();
}

void SwizzleNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader)
{
    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

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

    if (!swizzle.empty())
    {
		// If the input is unconnected we must declare a variable
		// for it first, in order to swizzle it below.
		if (!in->connection)
		{
			string variableValue = in->value ? shadergen.getSyntax()->getValue(in->type, *in->value) : shadergen.getSyntax()->getDefaultValue(in->type);
			shader.addLine(shadergen.getSyntax()->getTypeName(in->type) + " " + variableName + " = " + variableValue);
		}
		const TypeDesc* type = in->connection ? in->connection->type : in->type;
		variableName = shadergen.getSyntax()->getSwizzledVariable(variableName, type, swizzle, node.getOutput()->type);
    }

    shader.beginLine();
    shadergen.emitOutput(context, node.getOutput(), true, false, shader);
    shader.addStr(" = " + variableName);
    shader.endLine();

    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

bool SwizzleNode::isEditable(const ShaderInput& input) const
{
    return (input._name != CHANNELS_STRING);
}

} // namespace MaterialX
