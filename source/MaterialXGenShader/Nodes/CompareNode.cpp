//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/Nodes/CompareNode.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

const StringVec CompareNode::INPUT_NAMES = { "intest", "cutoff", "in1", "in2" };

ShaderNodeImplPtr CompareNode::create()
{
    return std::make_shared<CompareNode>();
}

void CompareNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        const ShaderGenerator& shadergen = context.getShaderGenerator();

        const ShaderGraph& graph = *node.getParent();

        // Declare the output variable
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, true, context, stage);
        shadergen.emitLineEnd(stage);

        const ShaderInput* intest = node.getInput(INPUT_NAMES[0]);
        const ShaderInput* cutoff = node.getInput(INPUT_NAMES[1]);

        // Process the if and else branches of the conditional
        for (int branch = 2; branch <= 3; ++branch)
        {
            const ShaderInput* input = node.getInput(INPUT_NAMES[branch]);

            if (branch > 2)
            {
                shadergen.emitLine("else", stage, false);
            }
            else
            {
                shadergen.emitLineBegin(stage);
                shadergen.emitString("if (", stage);
                shadergen.emitInput(intest, context, stage);
                shadergen.emitString(" <= ", stage);
                shadergen.emitInput(cutoff, context, stage);
                shadergen.emitString(")", stage);
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
                    shadergen.emitFunctionCall(*otherNode, context, stage, false);
                }
            }

            shadergen.emitLineBegin(stage);
            shadergen.emitOutput(node.getOutput(), false, false, context, stage);
            shadergen.emitString(" = ", stage);
            shadergen.emitInput(input, context, stage);
            shadergen.emitLineEnd(stage);

            shadergen.emitScopeEnd(stage);
        }
    END_SHADER_STAGE(stage, Stage::PIXEL)
}

} // namespace MaterialX
