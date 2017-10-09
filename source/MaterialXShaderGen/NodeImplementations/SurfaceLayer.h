#ifndef MATERIALX_SURFACELAYER_H
#define MATERIALX_SURFACELAYER_H

#include <MaterialXShaderGen/NodeImplementation.h>

namespace MaterialX
{

/// Implementation of surfacelayer node for glsl
class SurfaceLayerGlsl : public NodeImplementation
{
    DECLARE_NODE_IMPLEMENTATION(SurfaceLayerGlsl)
public:
    void emitFunction(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
