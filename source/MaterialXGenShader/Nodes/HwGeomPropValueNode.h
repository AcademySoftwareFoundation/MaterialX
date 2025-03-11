//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HWGEOMPROPVALUENODE_H
#define MATERIALX_HWGEOMPROPVALUENODE_H

#include <MaterialXGenShader/HwShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

/// GeomPropValue node implementation for Hw shader languages
class MX_GENSHADER_API HwGeomPropValueNode : public HwImplementation
{
  public:
    static ShaderNodeImplPtr create();

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    bool isEditable(const ShaderInput& /*input*/) const override { return false; }
};

/// GeomPropValue node non-implementation for Hw shader languages
class MX_GENSHADER_API HwGeomPropValueNodeAsUniform : public HwImplementation
{
  public:
    static ShaderNodeImplPtr create();

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
