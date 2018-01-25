#include <MaterialXShaderGen/Implementations/Compare.h>
#include <MaterialXShaderGen/HwShader.h>
#include <MaterialXShaderGen/ShaderGenerator.h>
#include <MaterialXShaderGen/SgNode.h>

namespace MaterialX
{

const vector<string> Compare::kInputNames = { "intest", "cutoff", "in1", "in2" };

SgImplementationPtr Compare::creator()
{
    return std::make_shared<Compare>();
}

void Compare::emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader)
{
    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

    // Declare the output variable
    shader.beginLine();
    shadergen.emitOutput(node.getOutput(), true, shader);
    shader.endLine();

    const SgInput* intest = node.getInput("intest");
    const SgInput* cutoff = node.getInput("cutoff");

    // Process the if and else branches of the conditional
    for (int branch = 2; branch <= 3; ++branch)
    {
        const SgInput* input = node.getInput(kInputNames[branch]);

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
                shader.addFunctionCall(otherNode, shadergen);
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
