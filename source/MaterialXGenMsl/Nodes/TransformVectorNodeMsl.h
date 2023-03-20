//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_TRANSFORMVECTORNODEMSL_H
#define MATERIALX_TRANSFORMVECTORNODEMSL_H

#include <MaterialXGenMsl/MslShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

/// TransformVector node implementation for MSL
class MX_GENMSL_API TransformVectorNodeMsl : public MslImplementation
{
  public:
    static ShaderNodeImplPtr create();

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

  protected:
    virtual const string& getMatrix(const string& fromSpace, const string& toSpace) const;
    virtual string getHomogeneousCoordinate(const ShaderInput* in, GenContext& context) const;
};

MATERIALX_NAMESPACE_END

#endif
