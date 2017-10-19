#ifndef MATERIALX_SURFACE_H
#define MATERIALX_SURFACE_H

#include <MaterialXShaderGen/NodeImplementation.h>

namespace MaterialX
{

/// Implementation of 'surface' node for OgsFx
class SurfaceOgsFx : public NodeImplementation
{
    DECLARE_NODE_IMPLEMENTATION(SurfaceOgsFx)
public:
    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
