#include <MaterialXShaderGen/NodeImplementations/Compare.h>
#include <MaterialXShaderGen/Shader.h>
#include <MaterialXShaderGen/ShaderGenerator.h>

namespace MaterialX
{

DEFINE_NODE_IMPLEMENTATION(Compare, "compare", "", "")

void Compare::emitCode(const SgNode& sgnode, ShaderGenerator& shadergen, Shader& shader)
{
    const Node& node = sgnode.getNode();

    // Declare the output variable
    shader.beginLine();
    shadergen.emitOutput(node, true, shader);
    shader.endLine();

    static const vector<string> inputNames = { "intest", "in1", "in2" };
    const InputPtr intest = node.getInput("intest");
    const ParameterPtr cutoff = node.getParameter("cutoff");

    // Check if we have a constant conditional expression
    NodePtr intestNode = intest->getConnectedNode();
    if (!intestNode || intestNode->getCategory() == "constant")
    {
        // Constant conditional so just emit the one and only branch taken

        const float intestValue = intestNode ?
            intestNode->getParameter("value")->getValue()->asA<float>() :
            intest->getValue()->asA<float>();

        const int branch = (intestValue <= cutoff->getValue()->asA<float>() ? 1 : 2);
        const InputPtr input = node.getInput(inputNames[branch]);

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
    }
    else
    {
        // Non-constant conditional so we must emit an if-else statement

        // Process the if and else branches of the conditional
        for (int branch = 1; branch <= 2; ++branch)
        {
            const InputPtr input = node.getInput(inputNames[branch]);

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
}

} // namespace MaterialX
