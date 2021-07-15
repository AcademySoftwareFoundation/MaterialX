//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_CLOSUREMIXNODEOSL_H
#define MATERIALX_CLOSUREMIXNODEOSL_H

#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

/// Closure mix node implementation for OSL.
class ClosureMixNodeOsl : public ShaderNodeImpl
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    /// String constants
    static const string FG;
    static const string BG;
    static const string MIX;
};

} // namespace MaterialX

#endif
