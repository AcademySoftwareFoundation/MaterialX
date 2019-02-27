#include <MaterialXGenShader/Nodes/CompareNode.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/ShaderNode.h>

namespace MaterialX
{

const vector<string> CompareNode::INPUT_NAMES = { "intest", "cutoff", "in1", "in2" };

ShaderNodeImplPtr CompareNode::create()
{
    return std::make_shared<CompareNode>();
}

void CompareNode::emitFunctionCall(ShaderStage& stage, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const
{
BEGIN_SHADER_STAGE(stage, MAIN_STAGE)

    const ShaderGraph& graph = *node.getParent();

    // Declare the output variable
    shadergen.emitLineBegin(stage);
    shadergen.emitOutput(stage, context, node.getOutput(), true, true);
    shadergen.emitLineEnd(stage);

    const ShaderInput* intest = node.getInput(INPUT_NAMES[0]);
    const ShaderInput* cutoff = node.getInput(INPUT_NAMES[1]);

    // Process the if and else branches of the conditional
    for (int branch = 2; branch <= 3; ++branch)
    {
        const ShaderInput* input = node.getInput(INPUT_NAMES[branch]);

        if (branch > 2)
        {
            shadergen.emitLine(stage, "else", false);
        }
        else
        {
            shadergen.emitLineBegin(stage);
            shadergen.emitString(stage, "if (");
            shadergen.emitInput(stage, context, intest);
            shadergen.emitString(stage, " <= ");
            shadergen.emitInput(stage, context, cutoff);
            shadergen.emitString(stage, ")");
            shadergen.emitLineEnd(stage, false);
        }

        shadergen.emitScopeBegin(stage);

        // Emit function calls for nodes that are ONLY needed in this scope
        for (const ShaderNode* otherNode : graph.getNodes())
        {
            const ShaderNode::ScopeInfo& scope = otherNode->getScopeInfo();
            if (scope.conditionalNode == &node && scope.usedByBranch(branch))
            {
                // Force ignore scope otherwise the function call will be omitted.
                shadergen.emitFunctionCall(stage, context, *otherNode, false);
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
