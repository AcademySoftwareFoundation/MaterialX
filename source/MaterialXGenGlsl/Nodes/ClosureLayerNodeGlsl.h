//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_CLOSURELAYERNODEGLSL_H
#define MATERIALX_CLOSURELAYERNODEGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Closure layer node implementation for GLSL.
class MX_GENGLSL_API ClosureLayerNodeGlsl : public GlslImplementation
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    /// String constants
    static const string TOP;
    static const string BASE;
};

} // namespace MaterialX

#endif
