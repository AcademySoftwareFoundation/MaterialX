#include <MaterialXShaderGen/NodeImplementations/Compare.h>
#include <MaterialXShaderGen/Shader.h>
#include <MaterialXShaderGen/ShaderGenerator.h>

namespace MaterialX
{

DEFINE_NODE_IMPLEMENTATION(Compare, "compare", "", "")

const vector<string> Compare::kInputNames = { "intest", "in1", "in2" };

void Compare::emitFunctionCall(const SgNode& sgnode, ShaderGenerator& shadergen, Shader& shader)
{
    const Node& node = sgnode.getNode();

    // Declare the output variable
    shader.beginLine();
    shadergen.emitOutput(node, true, shader);
    shader.endLine();

    const InputPtr intest = node.getInput("intest");
    const ParameterPtr cutoff = node.getParameter("cutoff");

    // Process the if and else branches of the conditional
    for (int branch = 1; branch <= 2; ++branch)
    {
        const InputPtr input = node.getInput(kInputNames[branch]);

        if (branch > 1)
        {
            shader.addLine("else", false);
        }
        else
        {
            shader.beginLine();
            shader.addStr("if (");
            shadergen.emitInput(*intest, shader);
            shader.addStr(" <= ");
            shadergen.emitInput(*cutoff, shader);
            shader.addStr(")");
            shader.endLine(false);
        }

        shader.beginScope();

        // Emit nodes that are ONLY needed in this scope
        // TODO: Performance warning, iterating all nodes in the graph!
        for (const SgNode& sg : shader.getNodes())
        {
            const SgNode::ScopeInfo& scope = sg.getScopeInfo();
            if (scope.conditionalNode == sgnode.getNodePtr() && scope.usedByBranch(branch))
            {
                shadergen.emitFunctionCall(sg, shader);
            }
        }

        shader.beginLine();
        shadergen.emitOutput(node, false, shader);
        shader.addStr(" = ");
        shadergen.emitInput(*input, shader);
        shader.endLine();

        shader.endScope();
    }
}

} // namespace MaterialX
