#include <MaterialXShaderGen/Nodes/Compare.h>
#include <MaterialXShaderGen/HwShader.h>
#include <MaterialXShaderGen/ShaderGenerator.h>
#include <MaterialXShaderGen/SgNode.h>

namespace MaterialX
{

const vector<string> Compare::INPUT_NAMES = { "intest", "cutoff", "in1", "in2" };

SgImplementationPtr Compare::create()
{
    return std::make_shared<Compare>();
}

void Compare::emitFunctionCall(const SgNode& node, const SgNodeContext& context, ShaderGenerator& shadergen, Shader& shader)
{
    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

    // Declare the output variable
    shader.beginLine();
    shadergen.emitOutput(node.getOutput(), true, shader);
    shader.endLine();

    const SgInput* intest = node.getInput(INPUT_NAMES[0]);
    const SgInput* cutoff = node.getInput(INPUT_NAMES[1]);

    // Process the if and else branches of the conditional
    for (int branch = 2; branch <= 3; ++branch)
    {
        const SgInput* input = node.getInput(INPUT_NAMES[branch]);

        if (branch > 2)
        {
            shader.addLine("else", false);
        }
        else
        {
            shader.beginLine();
            shader.addStr("if (");
            shadergen.emitInput(intest, shader);
            shader.addStr(" <= ");
            shadergen.emitInput(cutoff, shader);
            shader.addStr(")");
            shader.endLine(false);
        }

        shader.beginScope();

        // Emit nodes that are ONLY needed in this scope
        for (SgNode* otherNode : shader.getNodeGraph()->getNodes())
        {
            const SgNode::ScopeInfo& scope = otherNode->getScopeInfo();
            if (scope.conditionalNode == &node && scope.usedByBranch(branch))
            {
                shader.addFunctionCall(otherNode, context, shadergen);
            }
        }

        shader.beginLine();
        shadergen.emitOutput(node.getOutput(), false, shader);
        shader.addStr(" = ");
        shadergen.emitInput(input, shader);
        shader.endLine();

        shader.endScope();
    }

    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
