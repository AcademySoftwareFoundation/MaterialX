//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_LAYERNODEGLSL_H
#define MATERIALX_LAYERNODEGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Layer node implementation for GLSL.
class LayerNodeGlsl : public GlslImplementation
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
