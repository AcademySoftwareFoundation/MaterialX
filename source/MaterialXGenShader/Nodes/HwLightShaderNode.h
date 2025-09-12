//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HWLIGHTSHADERNODE_H
#define MATERIALX_HWLIGHTSHADERNODE_H

#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/Nodes/SourceCodeNode.h>

MATERIALX_NAMESPACE_BEGIN

/// LightShader node implementation for hardware languages.
/// Used for all light shaders implemented in source code.
class MX_GENSHADER_API HwLightShaderNode : public SourceCodeNode
{
  public:
    HwLightShaderNode();

    static ShaderNodeImplPtr create();

    void initialize(const InterfaceElement& element, GenContext& context) override;

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

  protected:
    VariableBlock _lightUniforms;
};

MATERIALX_NAMESPACE_END

#endif
