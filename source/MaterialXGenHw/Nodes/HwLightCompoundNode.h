//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HWLIGHTCOMPOUNDNODE_H
#define MATERIALX_HWLIGHTCOMPOUNDNODE_H

#include <MaterialXGenHw/Export.h>

#include <MaterialXGenShader/Nodes/CompoundNode.h>

MATERIALX_NAMESPACE_BEGIN

/// LightCompound node implementation for hardware languages.
class MX_GENHW_API HwLightCompoundNode : public CompoundNode
{
  public:
    HwLightCompoundNode();

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
