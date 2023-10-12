//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HWTRANSFORMNODE_H
#define MATERIALX_HWTRANSFORMNODE_H

#include <MaterialXGenShader/ShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

/// Generic transformation node for hardware languages.
class MX_GENSHADER_API HwTransformNode : public ShaderNodeImpl
{
  public:
    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;
    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

  protected:
    virtual const string& getMatrix(const string& fromSpace, const string& toSpace) const = 0;
    virtual string getHomogeneousCoordinate() const = 0;
    virtual bool shouldNormalize() const { return false; }

    virtual const ShaderInput* getFromSpaceInput(const ShaderNode&) const;
    virtual const ShaderInput* getToSpaceInput(const ShaderNode&) const;

    static string MODEL;
    static string OBJECT;
    static string WORLD;
};

class MX_GENSHADER_API HwTransformVectorNode : public HwTransformNode
{
  public:
    static ShaderNodeImplPtr create();

  protected:
    const string& getMatrix(const string& fromSpace, const string& toSpace) const override;
    string getHomogeneousCoordinate() const override;
};

class MX_GENSHADER_API HwTransformPointNode : public HwTransformVectorNode
{
  public:
    static ShaderNodeImplPtr create();

  protected:
    string getHomogeneousCoordinate() const override;
};

class MX_GENSHADER_API HwTransformNormalNode : public HwTransformNode
{
  public:
    static ShaderNodeImplPtr create();

  protected:
    const string& getMatrix(const string& fromSpace, const string& toSpace) const override;
    string getHomogeneousCoordinate() const override;
    bool shouldNormalize() const override { return true; }
};

MATERIALX_NAMESPACE_END

#endif
