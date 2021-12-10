//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_CLOSURELAYERNODEOSL_H
#define MATERIALX_CLOSURELAYERNODEOSL_H

#include <MaterialXGenOsl/Export.h>

#include <MaterialXGenShader/Nodes/ClosureLayerNode.h>

MATERIALX_NAMESPACE_BEGIN

/// Closure layer node OSL implementation.
class MX_GENOSL_API ClosureLayerNodeOsl : public ClosureLayerNode
{
  public:
    static ShaderNodeImplPtr create();
    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
