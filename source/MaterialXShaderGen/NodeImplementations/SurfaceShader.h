#ifndef MATERIALX_SURFACESHADER_H
#define MATERIALX_SURFACESHADER_H

#include <MaterialXShaderGen/NodeImplementation.h>

namespace MaterialX
{

/// Implementation of surfaceshader node for OgsFx
class SurfaceShaderOgsFx : public NodeImplementation
{
    DECLARE_NODE_IMPLEMENTATION(SurfaceShaderOgsFx)
public:
    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
