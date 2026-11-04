//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenWgsl/Nodes/WgslGeomPropValueNode.h>

#include <MaterialXGenHw/HwConstants.h>
#include <MaterialXGenHw/HwShaderGenerator.h>

#include <MaterialXGenShader/Exception.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr WgslGeomPropValueNodeBoolean::create()
{
    return std::make_shared<WgslGeomPropValueNodeBoolean>();
}

void WgslGeomPropValueNodeBoolean::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        const ShaderInput* geomPropInput = node.getInput(GEOMPROP);
        if (!geomPropInput || !geomPropInput->getValue())
        {
            throw ExceptionShaderGenError("No 'geomprop' parameter found on geompropvalue node '" + node.getName() + "'. Don't know what property to bind");
        }
        const string attrName = geomPropInput->getValue()->getValueString();
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        shadergen.emitString(" = bool(" + HW::T_GEOMPROP + "_" + attrName + ")", stage);
        shadergen.emitLineEnd(stage);
    }
}

MATERIALX_NAMESPACE_END
