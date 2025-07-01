//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_LIGHTCOMPOUNDNODESLANG_H
#define MATERIALX_LIGHTCOMPOUNDNODESLANG_H

#include <MaterialXGenSlang/Export.h>

#include <MaterialXGenShader/Nodes/CompoundNode.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/GenContext.h>

MATERIALX_NAMESPACE_BEGIN

class SlangShaderGenerator;

/// LightCompound node implementation for Slang
/// Differs from LightCompoundNodeGlsl by:
/// - downcasting to SlangShaderGenerator instead of the GlslShaderGenerator.
class MX_GENSLANG_API LightCompoundNodeSlang : public CompoundNode
{
  public:
    LightCompoundNodeSlang();

    static ShaderNodeImplPtr create();

    void initialize(const InterfaceElement& element, GenContext& context) override;

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

  protected:
    VariableBlock _lightUniforms;
};

MATERIALX_NAMESPACE_END

#endif
