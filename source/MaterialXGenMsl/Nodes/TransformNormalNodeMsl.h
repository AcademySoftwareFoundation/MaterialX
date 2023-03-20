//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_TRANSFORMNORMALNODEMSL_H
#define MATERIALX_TRANSFORMNORMALNODEMSL_H

#include <MaterialXGenMsl/Nodes/TransformVectorNodeMsl.h>

MATERIALX_NAMESPACE_BEGIN

/// TransformNormal node implementation for MSL
class MX_GENMSL_API TransformNormalNodeMsl : public TransformVectorNodeMsl
{
  public:
    static ShaderNodeImplPtr create();

  protected:
    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    const string& getMatrix(const string& fromSpace, const string& toSpace) const override;
};

MATERIALX_NAMESPACE_END

#endif
