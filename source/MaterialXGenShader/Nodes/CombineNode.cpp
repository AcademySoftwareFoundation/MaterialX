#include <MaterialXGenShader/Nodes/CombineNode.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/TypeDesc.h>

namespace MaterialX
{

ShaderNodeImplPtr CombineNode::create()
{
    return std::make_shared<CombineNode>();
}

void CombineNode::emitFunctionCall(ShaderStage& stage, const ShaderNode& node, const ShaderGenerator& shadergen, GenContext& context) const
{
BEGIN_SHADER_STAGE(stage, MAIN_STAGE)

    const ShaderInput* in1 = node.getInput(0);
    const ShaderOutput* out = node.getOutput();
    if (!in1 || !out)
    {
        throw ExceptionShaderGenError("Node '" + node.getName() + "' is not a valid convert node");
    }

    // Check the node signature to determine which
    // type conversion to perform, and get the value
    // components to use for constructing the new value.
    //
    vector<string> valueComponents;
    if (in1->type == Type::FLOAT)
    {
        // Get the components of the input values.
        const size_t numInputs = node.numInputs();
        valueComponents.resize(numInputs);
        for (size_t i = 0; i < numInputs; ++i)
        {
            const ShaderInput* input = node.getInput(i);
            shadergen.getInput(context, input, valueComponents[i]);
        }

    }
    else if (in1->type == Type::COLOR3 || in1->type == Type::VECTOR3)
    {
        const ShaderInput* in2 = node.getInput(1);
        if (!in2 || in2->type != Type::FLOAT)
        {
            throw ExceptionShaderGenError("Node '" + node.getName() + "' is not a valid convert node");
        }

        // If in1 is unconnected we must declare a local variable
        // for it first, in order to access components from it below.
        string in1Variable = in1->connection ? in1->connection->variable : in1->variable;
        if (!in1->connection)
        {
            string variableValue = in1->value ? shadergen.getSyntax()->getValue(in1->type, *in1->value) : shadergen.getSyntax()->getDefaultValue(in1->type);
            shadergen.emitLine(stage, shadergen.getSyntax()->getTypeName(in1->type) + " " + in1Variable + " = " + variableValue);
        }

        // Get the components of the input values.
        valueComponents.resize(4);

        // Get components from in1
        const vector<string>& in1Members = shadergen.getSyntax()->getTypeSyntax(in1->type).getMembers();
        valueComponents[0] = in1Variable + in1Members[0];
        valueComponents[1] = in1Variable + in1Members[1];
        valueComponents[2] = in1Variable + in1Members[2];

        // Get component from in2
        shadergen.getInput(context, in2, valueComponents[3]);
    }
    else if (in1->type == Type::COLOR2 || in1->type == Type::VECTOR2)
    {
        const ShaderInput* in2 = node.getInput(1);
        if (!in2 || (in2->type != Type::COLOR2 && in2->type != Type::VECTOR2))
        {
            throw ExceptionShaderGenError("Node '" + node.getName() + "' is not a valid convert node");
        }

        // If in1 is unconnected we must declare a local variable
        // for it first, in order to access components from it below.
        string in1Variable = in1->connection ? in1->connection->variable : in1->variable;
        if (!in1->connection)
        {
            string variableValue = in1->value ? shadergen.getSyntax()->getValue(in1->type, *in1->value) : shadergen.getSyntax()->getDefaultValue(in1->type);
            shadergen.emitLine(stage, shadergen.getSyntax()->getTypeName(in1->type) + " " + in1Variable + " = " + variableValue);
        }

        // If in2 is unconnected we must declare a local variable
        // for it first, in order to access components from it below.
        string in2Variable = in2->connection ? in2->connection->variable : in2->variable;
        if (!in2->connection)
        {
            string variableValue = in2->value ? shadergen.getSyntax()->getValue(in2->type, *in2->value) : shadergen.getSyntax()->getDefaultValue(in2->type);
            shadergen.emitLine(stage, shadergen.getSyntax()->getTypeName(in2->type) + " " + in2Variable + " = " + variableValue);
        }

        // Get the components of the input values.
        valueComponents.resize(4);

        // Get components from in1.
        const vector<string>& in1Members = shadergen.getSyntax()->getTypeSyntax(in1->type).getMembers();
        valueComponents[0] = in1Variable + in1Members[0];
        valueComponents[1] = in1Variable + in1Members[1];

        // Get components from in2.
        const vector<string>& in2Members = shadergen.getSyntax()->getTypeSyntax(in2->type).getMembers();
        valueComponents[2] = in2Variable + in2Members[0];
        valueComponents[3] = in2Variable + in2Members[1];
    }

    if (valueComponents.empty())
    {
        throw ExceptionShaderGenError("Node '" + node.getName() + "' is not a valid convert node");
    }

    // Let the TypeSyntax construct the value from the components.
    const TypeSyntax& outTypeSyntax = shadergen.getSyntax()->getTypeSyntax(out->type);
    const string result = outTypeSyntax.getValue(valueComponents, false);

    shadergen.emitLineBegin(stage);
    shadergen.emitOutput(stage, context, node.getOutput(), true, false);
    shadergen.emitString(stage, " = " + result);
    shadergen.emitLineEnd(stage);

END_SHADER_STAGE(stage, MAIN_STAGE)
}

} // namespace MaterialX
