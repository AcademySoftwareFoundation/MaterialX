//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_CLOSUREADDNODE_H
#define MATERIALX_CLOSUREADDNODE_H

#include <MaterialXGenShader/ShaderNodeImpl.h>

MATERIALX_NAMESPACE_BEGIN

/// Closure add node implementation.
class MX_GENSHADER_API ClosureAddNode : public ShaderNodeImpl
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    /// String constants
    static const string IN1;
    static const string IN2;
};

MATERIALX_NAMESPACE_END

#endif
