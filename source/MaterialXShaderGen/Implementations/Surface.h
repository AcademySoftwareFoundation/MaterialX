#ifndef MATERIALX_SURFACE_H
#define MATERIALX_SURFACE_H

#include <MaterialXShaderGen/Implementations/OgsFxImplementation.h>

namespace MaterialX
{

/// Implementation of 'surface' node for OgsFx
class SurfaceOgsFx : public SgImplementation
{
  public:
    static SgImplementationPtr creator();

    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    bool isTransparent(const SgNode& node) const override;
};

} // namespace MaterialX

#endif
