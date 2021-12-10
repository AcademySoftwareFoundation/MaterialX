//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_SURFACENODEMAYA_H
#define MATERIALX_SURFACENODEMAYA_H

#include <MaterialXGenGlsl/Nodes/SurfaceNodeGlsl.h>
#include <MaterialXGenGlsl/GlslShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN


/// Surface node implementation for GLSL
class SurfaceNodeMaya : public SurfaceNodeGlsl
{
  public:
    SurfaceNodeMaya();

    static ShaderNodeImplPtr create();

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitLightLoop(const ShaderNode& node, GenContext& context, ShaderStage& stage, const string& outColor) const override;
};

MATERIALX_NAMESPACE_END

#endif
