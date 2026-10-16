//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_WGSLGEOMPROPVALUENODE_H
#define MATERIALX_WGSLGEOMPROPVALUENODE_H

#include <MaterialXGenWgsl/Export.h>

#include <MaterialXGenHw/Nodes/HwGeomPropValueNode.h>

MATERIALX_NAMESPACE_BEGIN

/// Boolean GeomPropValue node for WGSL. The private uniform is stored as u32, so the
/// assignment to the bool output is cast.
class MX_GENWGSL_API WgslGeomPropValueNodeBoolean : public HwGeomPropValueNodeAsUniform
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
