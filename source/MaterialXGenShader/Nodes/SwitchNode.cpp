#include <MaterialXGenShader/Nodes/SwitchNode.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

const vector<string> SwitchNode::INPUT_NAMES = { "in1", "in2", "in3", "in4", "in5", "which" };

ShaderNodeImplPtr SwitchNode::create()
{
    return std::make_shared<SwitchNode>();
}

void SwitchNode::emitFunctionCall(ShaderStage& stage, const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen)
{
BEGIN_SHADER_STAGE(stage, MAIN_STAGE)

    // Declare the output variable
    shadergen.emitLineBegin(stage);
    shadergen.emitOutput(stage, context, node.getOutput(), true, true);
    shadergen.emitLineEnd(stage);

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

        shadergen.emitLineBegin(stage);
        if (branch > 0)
        {
            shadergen.emitString(stage, "else ");
        }
        if (branch < 5)
        {
            // 'which' can be float, integer or boolean, 
            // so always convert to float to make sure the comparison is valid
            shadergen.emitString(stage, "if (float("); 
            shadergen.emitInput(stage, context, which);
            shadergen.emitString(stage, ") < ");
            shadergen.emitValue(stage, float(branch + 1));
            shadergen.emitString(stage, ")");
        }
        shadergen.emitLineEnd(stage, false);

        shadergen.emitScopeBegin(stage);

        // Emit nodes that are ONLY needed in this scope
        for (ShaderNode* otherNode : stage.getGraph()->getNodes())
        {
            const ShaderNode::ScopeInfo& scope = otherNode->getScopeInfo();
            if (scope.conditionalNode == &node && scope.usedByBranch(branch))
            {
                shadergen.emitFunctionCall(stage, otherNode, context, false);
            }
        }

        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(stage, context, node.getOutput(), false, false);
        shadergen.emitString(stage, " = ");
        shadergen.emitInput(stage, context, input);
        shadergen.emitLineEnd(stage);

        shadergen.emitScopeEnd(stage);
    }

    END_SHADER_STAGE(stage, HW::PIXEL_STAGE)
}

} // namespace MaterialX
