//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_DISPLACEMENTNODEGLSL_H
#define MATERIALX_DISPLACEMENTNODEGLSL_H

#include <MaterialXGenGlsl/Export.h>

#include <MaterialXGenShader/ShaderNodeImpl.h>

MATERIALX_NAMESPACE_BEGIN

/// Displacement node implementation for GLSL.
/// Evaluates displacement in the vertex stage and passes it to the pixel stage.
class MX_GENGLSL_API DisplacementNodeGlsl : public ShaderNodeImpl
{
  public:
    static ShaderNodeImplPtr create();

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;
    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
