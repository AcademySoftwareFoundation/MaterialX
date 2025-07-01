//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_LIGHTSHADERNODESLANG_H
#define MATERIALX_LIGHTSHADERNODESLANG_H

#include <MaterialXGenSlang/SlangShaderGenerator.h>
#include <MaterialXGenShader/Nodes/SourceCodeNode.h>

MATERIALX_NAMESPACE_BEGIN

/// LightShader node implementation for Slang
/// Used for all light shaders implemented in source code.
/// Does not differ from LightShaderNodeGlsl, but we do not include cross generators.
class MX_GENSLANG_API LightShaderNodeSlang : public SourceCodeNode
{
  public:
    LightShaderNodeSlang();

    static ShaderNodeImplPtr create();

    void initialize(const InterfaceElement& element, GenContext& context) override;

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

  protected:
    VariableBlock _lightUniforms;
};

MATERIALX_NAMESPACE_END

#endif
