//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_CLOSUREADDNODEGLSL_H
#define MATERIALX_CLOSUREADDNODEGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Closure add node implementation for GLSL.
class MX_GENGLSL_API ClosureAddNodeGlsl : public GlslImplementation
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    /// String constants
    static const string IN1;
    static const string IN2;
};

} // namespace MaterialX

#endif
