//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_TRANSFORMPOINTNODEGLSL_H
#define MATERIALX_TRANSFORMPOINTNODEGLSL_H

#include <MaterialXGenGlsl/Nodes/TransformVectorNodeGlsl.h>

MATERIALX_NAMESPACE_BEGIN

/// TransformPoint node implementation for GLSL
class MX_GENGLSL_API TransformPointNodeGlsl : public TransformVectorNodeGlsl
{
  public:
    static ShaderNodeImplPtr create();

  protected:
    virtual string getHomogeneousCoordinate(const ShaderInput* in, GenContext& context) const override;
};

MATERIALX_NAMESPACE_END

#endif
