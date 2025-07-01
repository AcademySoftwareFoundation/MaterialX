//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_TRANSFORMNODESLANG_H
#define MATERIALX_TRANSFORMNODESLANG_H

#include <MaterialXGenShader/Nodes/HwTransformNode.h>

MATERIALX_NAMESPACE_BEGIN

/// Generic transformation node for hardware languages
/// Differs from HwTransformNode by:
/// - `floatX` instead of `vecX`
/// - `mul(v, m)` instead of `m * v`
/// - codegen logic slightly change to accomodate the prefix nature of mul, rather than the infix *
class MX_GENSHADER_API TransformNodeSlang : public HwTransformNode
{
  public:
    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

  protected:
    virtual const string& getModelToWorldMatrix() const = 0;
    virtual const string& getWorldToModelMatrix() const = 0;
    virtual string getHomogeneousCoordinate() const = 0;
    virtual bool shouldNormalize() const { return false; }
};

/// Vector transform implementation for hardware languages
class MX_GENSHADER_API TransformVectorNodeSlang : public TransformNodeSlang
{
  public:
    static ShaderNodeImplPtr create();

  protected:
    const string& getModelToWorldMatrix() const override { return HW::T_WORLD_MATRIX; }
    const string& getWorldToModelMatrix() const override { return HW::T_WORLD_INVERSE_MATRIX; }
    string getHomogeneousCoordinate() const override { return "0.0"; }
};

/// Point transform implementation for hardware languages
class MX_GENSHADER_API TransformPointNodeSlang : public TransformVectorNodeSlang
{
  public:
    static ShaderNodeImplPtr create();

  protected:
    string getHomogeneousCoordinate() const override { return "1.0"; }
};

/// Normal transform implementation for hardware languages
class MX_GENSHADER_API TransformNormalNodeSlang : public TransformNodeSlang
{
  public:
    static ShaderNodeImplPtr create();

  protected:
    const string& getModelToWorldMatrix() const override { return HW::T_WORLD_INVERSE_TRANSPOSE_MATRIX; }
    const string& getWorldToModelMatrix() const override { return HW::T_WORLD_TRANSPOSE_MATRIX; }
    string getHomogeneousCoordinate() const override { return "0.0"; }
    bool shouldNormalize() const override { return true; }
};

MATERIALX_NAMESPACE_END

#endif
