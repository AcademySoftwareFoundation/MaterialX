//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/Nodes/ClosureLayerNodeGlsl.h>
#include <MaterialXGenGlsl/GlslShaderGenerator.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/TypeDesc.h>

namespace MaterialX
{

const string ClosureLayerNodeGlsl::TOP = "top";
const string ClosureLayerNodeGlsl::BASE = "base";
const string ClosureLayerNodeGlsl::THICKNESS = "thickness";
const string ClosureLayerNodeGlsl::IOR = "ior";

ShaderNodeImplPtr ClosureLayerNodeGlsl::create()
{
    return std::make_shared<ClosureLayerNodeGlsl>();
}

void ClosureLayerNodeGlsl::emitFunctionCall(const ShaderNode& _node, GenContext& context, ShaderStage& stage) const
{
BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
    const ShaderGenerator& shadergen = context.getShaderGenerator();

    ShaderNode& node = const_cast<ShaderNode&>(_node);

    ShaderInput* topInput = node.getInput(TOP);
    ShaderInput* baseInput = node.getInput(BASE);
    ShaderOutput* output = node.getOutput();

    // Make sure the layer is fully connected.
    if (!(topInput->getConnection() && baseInput->getConnection()))
    {
        // Just declare the output variable with default value.
        emitOutputVariables(node, context, stage);
        return;
    }

    ClosureContext* cct = context.getClosureContext();
    if (!cct)
    {
        throw ExceptionShaderGenError("No closure context set to evaluate node '" + node.getName() + "'");
    }

    ShaderNode* top = topInput->getConnection()->getNode();
    ShaderNode* base = baseInput->getConnection()->getNode();

    if (top->hasClassification(ShaderNode::Classification::THINFILM))
    {
        // This is a layer node with thin-film as top layer.
        // Call only the base BSDF but with thin-film parameters set.

        // Make sure the connection to base is a sibling node and not the graph interface.
        if (base->getParent() != node.getParent())
        {
            throw ExceptionShaderGenError("Thin-film can only be applied to a sibling node, not through a graph interface");
        }

        // Store the base result in the layer result variable.
        const string oldVariable = base->getOutput()->getVariable();
        base->getOutput()->setVariable(output->getVariable());

        // Set the extra parameters for thin-film.
        ClosureContext::ClosureParams params;
        params[THICKNESS] = top->getInput(THICKNESS);
        params[IOR] = top->getInput(IOR);

        // Emit the function call.
        ScopedAssignClosureParams assign(&params, base, cct);
        shadergen.emitFunctionCall(*base, context, stage);

        // Restore state.
        base->getOutput()->setVariable(oldVariable);
    }
    else
    {
        // Evaluate top and base nodes and combine their result
        // according to throughput.
        //
        // TODO: In the BSDF over BSDF case should we emit code
        //       to check the top throughput amount before calling
        //       the base BSDF?

        // Make sure the connections are sibling nodes and not the graph interface.
        if (top->getParent() == node.getParent())
        {
            // If this layer node has closure parameters set,
            // we pass this on to the top component only.
            ScopedAssignClosureParams assign(&node, top, cct);
            shadergen.emitFunctionCall(*top, context, stage);
        }
        if (base->getParent() == node.getParent())
        {
            shadergen.emitFunctionCall(*base, context, stage);
        }

        // Get the result variables.
        const string& topResult = topInput->getConnection()->getVariable();
        const string& baseResult = baseInput->getConnection()->getVariable();

        // Calculate the layering result.
        emitOutputVariables(node, context, stage);
        if (base->getOutput()->getType() == Type::VDF)
        {
            shadergen.emitLine(output->getVariable() + ".result = " + topResult + ".result * " + baseResult + ".throughput", stage);
            shadergen.emitLine(output->getVariable() + ".throughput = " + topResult + ".throughput * " + baseResult + ".throughput", stage);
        }
        else
        {
            shadergen.emitLine(output->getVariable() + ".result = " + topResult + ".result + " + baseResult + ".result * " + topResult + ".throughput", stage);
            shadergen.emitLine(output->getVariable() + ".throughput = " + topResult + ".throughput * " + baseResult + ".throughput", stage);
        }
    }
END_SHADER_STAGE(stage, Stage::PIXEL)
}

} // namespace MaterialX
