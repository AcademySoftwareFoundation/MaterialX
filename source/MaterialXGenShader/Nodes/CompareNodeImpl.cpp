#include <MaterialXGenShader/Nodes/CompareNodeImpl.h>
#include <MaterialXGenShader/HwShader.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/ShaderNode.h>

namespace MaterialX
{

const vector<string> CompareNodeImpl::INPUT_NAMES = { "intest", "cutoff", "in1", "in2" };

ShaderNodeImplPtr CompareNodeImpl::create()
{
    return std::make_shared<CompareNodeImpl>();
}

void CompareNodeImpl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader)
{
    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

    // Declare the output variable
    shader.beginLine();
    shadergen.emitOutput(context, node.getOutput(), true, true, shader);
    shader.endLine();

    const ShaderInput* intest = node.getInput(INPUT_NAMES[0]);
    const ShaderInput* cutoff = node.getInput(INPUT_NAMES[1]);

    // Process the if and else branches of the conditional
    for (int branch = 2; branch <= 3; ++branch)
    {
        const ShaderInput* input = node.getInput(INPUT_NAMES[branch]);

        if (branch > 2)
        {
            shader.addLine("else", false);
        }
        else
        {
            shader.beginLine();
            shader.addStr("if (");
            shadergen.emitInput(context, intest, shader);
            shader.addStr(" <= ");
            shadergen.emitInput(context, cutoff, shader);
            shader.addStr(")");
            shader.endLine(false);
        }

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
