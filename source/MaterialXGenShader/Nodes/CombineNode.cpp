//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/Nodes/CombineNode.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

ShaderNodeImplPtr CombineNode::create()
{
    return std::make_shared<CombineNode>();
}

void CombineNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        const ShaderGenerator& shadergen = context.getShaderGenerator();

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
        StringVec valueComponents;
        if (in1->getType() == Type::FLOAT)
        {
            // Get the components of the input values.
            const size_t numInputs = node.numInputs();
            valueComponents.resize(numInputs);
            for (size_t i = 0; i < numInputs; ++i)
            {
                const ShaderInput* input = node.getInput(i);
                valueComponents[i] = shadergen.getUpstreamResult(input, context);
            }
        }
        else if (in1->getType() == Type::COLOR3 || in1->getType() == Type::VECTOR3)
        {
            const ShaderInput* in2 = node.getInput(1);
            if (!in2 || in2->getType() != Type::FLOAT)
            {
                throw ExceptionShaderGenError("Node '" + node.getName() + "' is not a valid convert node");
            }

            // If in1 is unconnected we must declare a local variable
            // for it first, in order to access components from it below.
            string in1Variable = in1->getConnection() ? in1->getConnection()->getVariable() : in1->getVariable();
            if (!in1->getConnection())
            {
                string variableValue = in1->getValue() ? shadergen.getSyntax().getValue(in1->getType(), *in1->getValue()) : shadergen.getSyntax().getDefaultValue(in1->getType());
                shadergen.emitLine(shadergen.getSyntax().getTypeName(in1->getType()) + " " + in1Variable + " = " + variableValue, stage);
            }

            // Get the components of the input values.
            valueComponents.resize(4);

            // Get components from in1
            const StringVec& in1Members = shadergen.getSyntax().getTypeSyntax(in1->getType()).getMembers();
            valueComponents[0] = in1Variable + in1Members[0];
            valueComponents[1] = in1Variable + in1Members[1];
            valueComponents[2] = in1Variable + in1Members[2];

            // Get component from in2
            valueComponents[3] = shadergen.getUpstreamResult(in2, context);
        }
        else if (in1->getType() == Type::COLOR2 || in1->getType() == Type::VECTOR2)
        {
            const ShaderInput* in2 = node.getInput(1);
            if (!in2 || (in2->getType() != Type::COLOR2 && in2->getType() != Type::VECTOR2))
            {
                throw ExceptionShaderGenError("Node '" + node.getName() + "' is not a valid convert node");
            }

            // If in1 is unconnected we must declare a local variable
            // for it first, in order to access components from it below.
            string in1Variable = in1->getConnection() ? in1->getConnection()->getVariable() : in1->getVariable();
            if (!in1->getConnection())
            {
                string variableValue = in1->getValue() ? shadergen.getSyntax().getValue(in1->getType(), *in1->getValue()) : shadergen.getSyntax().getDefaultValue(in1->getType());
                shadergen.emitLine(shadergen.getSyntax().getTypeName(in1->getType()) + " " + in1Variable + " = " + variableValue, stage);
            }

            // If in2 is unconnected we must declare a local variable
            // for it first, in order to access components from it below.
            string in2Variable = in2->getConnection() ? in2->getConnection()->getVariable() : in2->getVariable();
            if (!in2->getConnection())
            {
                string variableValue = in2->getValue() ? shadergen.getSyntax().getValue(in2->getType(), *in2->getValue()) : shadergen.getSyntax().getDefaultValue(in2->getType());
                shadergen.emitLine(shadergen.getSyntax().getTypeName(in2->getType()) + " " + in2Variable + " = " + variableValue, stage);
            }

            // Get the components of the input values.
            valueComponents.resize(4);

            // Get components from in1.
            const StringVec& in1Members = shadergen.getSyntax().getTypeSyntax(in1->getType()).getMembers();
            valueComponents[0] = in1Variable + in1Members[0];
            valueComponents[1] = in1Variable + in1Members[1];

            // Get components from in2.
            const StringVec& in2Members = shadergen.getSyntax().getTypeSyntax(in2->getType()).getMembers();
            valueComponents[2] = in2Variable + in2Members[0];
            valueComponents[3] = in2Variable + in2Members[1];
        }

        if (valueComponents.empty())
        {
            throw ExceptionShaderGenError("Node '" + node.getName() + "' is not a valid convert node");
        }

        // Let the TypeSyntax construct the value from the components.
        const TypeSyntax& outTypeSyntax = shadergen.getSyntax().getTypeSyntax(out->getType());
        const string result = outTypeSyntax.getValue(valueComponents, false);

        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        shadergen.emitString(" = " + result, stage);
        shadergen.emitLineEnd(stage);
    END_SHADER_STAGE(stage, Stage::PIXEL)
}

} // namespace MaterialX
