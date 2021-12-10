//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_CLOSUREMIXNODE_H
#define MATERIALX_CLOSUREMIXNODE_H

#include <MaterialXGenShader/ShaderNodeImpl.h>

MATERIALX_NAMESPACE_BEGIN

/// Closure mix node implementation.
class MX_GENSHADER_API ClosureMixNode : public ShaderNodeImpl
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    /// String constants
    static const string FG;
    static const string BG;
    static const string MIX;
};

MATERIALX_NAMESPACE_END

#endif
