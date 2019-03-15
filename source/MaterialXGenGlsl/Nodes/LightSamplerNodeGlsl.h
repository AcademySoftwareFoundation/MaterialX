//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_LIGHTSAMPLERNODEGLSL_H
#define MATERIALX_LIGHTSAMPLERNODEGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Utility node for sampling lights for GLSL.
class LightSamplerNodeGlsl : public GlslImplementation
{
public:
    LightSamplerNodeGlsl();

    static ShaderNodeImplPtr create();

    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

} // namespace MaterialX

#endif
