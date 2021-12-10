//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenOsl/Nodes/SurfaceNodeOsl.h>
#include <MaterialXGenOsl/OslShaderGenerator.h>

#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/GenContext.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr SurfaceNodeOsl::create()
{
    return std::make_shared<SurfaceNodeOsl>();
}

void SurfaceNodeOsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    // Make sure a closure context is set for upstream closure evaluation,
    // and then just rely on the base class source code implementation.
    context.pushClosureContext(&_cct);
    ClosureSourceCodeNode::emitFunctionCall(node, context, stage);
    context.popClosureContext();
}

MATERIALX_NAMESPACE_END
