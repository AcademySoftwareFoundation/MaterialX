#include <MaterialXShaderGen/Implementations/Compare.h>
#include <MaterialXShaderGen/Shader.h>
#include <MaterialXShaderGen/ShaderGenerator.h>
#include <MaterialXShaderGen/SgNode.h>

namespace MaterialX
{

const vector<string> Compare::kInputNames = { "intest", "in1", "in2" };

SgImplementationPtr Compare::creator()
{
    return std::make_shared<Compare>();
}

void Compare::emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader, int, ...)
{
    // Declare the output variable
    shader.beginLine();
    shadergen.emitOutput(node.getOutput(), true, shader);
    shader.endLine();

    const SgInput* intest = node.getInput("intest");
    const SgInput* cutoff = node.getInput("cutoff");

    // Process the if and else branches of the conditional
    for (int branch = 1; branch <= 2; ++branch)
    {
        const SgInput* input = node.getInput(kInputNames[branch]);

        if (branch > 1)
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
        // TODO: Performance warning, iterating all nodes in the graph!
        for (const SgNode* sg : shader.getNodeGraph()->getNodes())
        {

            // TODO: FIX SCOPE
/*
            const SgNode::ScopeInfo& scope = sg->getScopeInfo();
            if (scope.conditionalNode == node.getNodePtr() && scope.usedByBranch(branch))
            {
                sg->getImplementation()->emitFunctionCall(*sg, shadergen, shader);
            }
            */
        }

        shader.beginLine();
        shadergen.emitOutput(node.getOutput(), false, shader);
        shader.addStr(" = ");
        shadergen.emitInput(input, shader);
        shader.endLine();

        shader.endScope();
    }
}

} // namespace MaterialX
