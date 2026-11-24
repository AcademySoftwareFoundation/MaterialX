//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenWgsl/Nodes/WgslMaterialNode.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr WgslMaterialNode::create()
{
    return std::make_shared<WgslMaterialNode>();
}

void WgslMaterialNode::emitFunctionCall(const ShaderNode& _node, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        ShaderNode& node = const_cast<ShaderNode&>(_node);
        ShaderInput* surfaceshaderInput = node.getInput(ShaderNode::SURFACESHADER);

        if (!surfaceshaderInput->getConnection())
        {
            // Just declare the output variable with a default value.
            emitOutputVariables(node, context, stage);
            return;
        }

        const ShaderGenerator& shadergen = context.getShaderGenerator();
        const Syntax& syntax = shadergen.getSyntax();

        // Emit the function call for upstream surface shader.
        const ShaderNode* surfaceshaderNode = surfaceshaderInput->getConnection()->getNode();
        if (surfaceshaderNode->isAGraph())
        {
            emitOutputVariables(node, context, stage);
            return;
        }
        shadergen.emitFunctionCall(*surfaceshaderNode, context, stage);

        // Assign this result to the material output variable (WGSL declaration order).
        const ShaderOutput* output = node.getOutput();
        shadergen.emitLine("var " + output->getVariable() + ": " + syntax.getTypeName(output->getType()) +
                               " = " + surfaceshaderInput->getConnection()->getVariable(),
                           stage);
    }
}

MATERIALX_NAMESPACE_END
