//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_BLURNODEOSL_H
#define MATERIALX_BLURNODEOSL_H

#include <MaterialXGenShader/Nodes/BlurNode.h>

namespace MaterialX
{

/// Blur node implementation for OSL
class BlurNodeOsl : public BlurNode
{
  public:
    static ShaderNodeImplPtr create();
    void emitSamplingFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

} // namespace MaterialX

#endif
