#include <MaterialXGenShader/Nodes/Switch.h>
#include <MaterialXGenShader/HwShader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

const vector<string> Switch::INPUT_NAMES = { "in1", "in2", "in3", "in4", "in5", "which" };

ShaderImplementationPtr Switch::create()
{
    return std::make_shared<Switch>();
}

void Switch::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader)
{
    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

    // Declare the output variable
    shader.beginLine();
    shadergen.emitOutput(context, node.getOutput(), true, true, shader);
    shader.endLine();

    const ShaderInput* which = node.getInput(INPUT_NAMES[5]);

    // Process the branches of the switch node
    for (int branch = 0; branch < 5; ++branch)
    {
        const ShaderInput* input = node.getInput(INPUT_NAMES[branch]);
        if (!input)
        {
            // The boolean version only has two inputs
            // so break if the input doesn't exist
            break;
        }

        shader.beginLine();
        if (branch > 0)
        {
            shader.addStr("else ");
        }
        if (branch < 5)
        {
            // 'which' can be float, integer or boolean, 
            // so always convert to float to make sure the comparison is valid
            shader.addStr("if (float("); shadergen.emitInput(context, which, shader); shader.addStr(") < "); shader.addValue(float(branch + 1));  shader.addStr(")");
        }
        shader.endLine(false);

        shader.beginScope();

        // Emit nodes that are ONLY needed in this scope
        for (ShaderNode* otherNode : shader.getGraph()->getNodes())
        {
            const ShaderNode::ScopeInfo& scope = otherNode->getScopeInfo();
            if (scope.conditionalNode == &node && scope.usedByBranch(branch))
            {
                shader.addFunctionCall(otherNode, context, shadergen);
            }
        }

        shader.beginLine();
        shadergen.emitOutput(context, node.getOutput(), false, false, shader);
        shader.addStr(" = ");
        shadergen.emitInput(context, input, shader);
        shader.endLine();

        shader.endScope();
    }

    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
