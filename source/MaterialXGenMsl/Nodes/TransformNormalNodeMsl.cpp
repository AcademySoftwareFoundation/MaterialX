//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenMsl/Nodes/TransformNormalNodeMsl.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr TransformNormalNodeMsl::create()
{
    return std::make_shared<TransformNormalNodeMsl>();
}

void TransformNormalNodeMsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    TransformVectorNodeMsl::emitFunctionCall(node, context, stage);

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        const ShaderOutput* output = node.getOutput();
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(output, false, false, context, stage);
        shadergen.emitString(" = normalize(" + output->getVariable() + ")", stage);
        shadergen.emitLineEnd(stage);
    }
}

const string& TransformNormalNodeMsl::getMatrix(const string& fromSpace, const string& toSpace) const
{
    if ((fromSpace == MODEL || fromSpace == OBJECT) && toSpace == WORLD)
    {
        return HW::T_WORLD_INVERSE_TRANSPOSE_MATRIX;
    }
    else if (fromSpace == WORLD && (toSpace == MODEL || toSpace == OBJECT))
    {
        return HW::T_WORLD_TRANSPOSE_MATRIX;
    }
    return EMPTY_STRING;
}

MATERIALX_NAMESPACE_END
