//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_BLURNODEOSL_H
#define MATERIALX_BLURNODEOSL_H

#include <MaterialXGenOsl/Export.h>

#include <MaterialXGenShader/Nodes/BlurNode.h>

MATERIALX_NAMESPACE_BEGIN

/// Blur node implementation for OSL
class MX_GENOSL_API BlurNodeOsl : public BlurNode
{
  public:
    static ShaderNodeImplPtr create();
    void emitSamplingFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
