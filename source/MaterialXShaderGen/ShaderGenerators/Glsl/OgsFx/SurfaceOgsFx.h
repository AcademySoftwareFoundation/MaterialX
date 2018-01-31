#ifndef MATERIALX_SURFACEOGSFX_H
#define MATERIALX_SURFACEOGSFX_H

#include <MaterialXShaderGen/ShaderGenerators/Glsl/OgsFx/OgsFxShaderGenerator.h>

namespace MaterialX
{

/// Implementation of 'surface' node for OgsFx
class SurfaceOgsFx : public OgsFxImplementation
{
  public:
    static SgImplementationPtr creator();

    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    bool isTransparent(const SgNode& node) const override;
};

} // namespace MaterialX

#endif
