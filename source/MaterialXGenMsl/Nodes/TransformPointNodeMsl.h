//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_TRANSFORMPOINTNODEMSL_H
#define MATERIALX_TRANSFORMPOINTNODEMSL_H

#include <MaterialXGenMsl/Nodes/TransformVectorNodeMsl.h>

MATERIALX_NAMESPACE_BEGIN

/// TransformPoint node implementation for MSL
class MX_GENMSL_API TransformPointNodeMsl : public TransformVectorNodeMsl
{
  public:
    static ShaderNodeImplPtr create();

  protected:
    virtual string getHomogeneousCoordinate(const ShaderInput* in, GenContext& context) const override;
};

MATERIALX_NAMESPACE_END

#endif
