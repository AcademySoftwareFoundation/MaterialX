#ifndef MATERIALX_SURFACE_H
#define MATERIALX_SURFACE_H

#include <MaterialXShaderGen/SgImplementation.h>

namespace MaterialX
{

/// Implementation of 'surface' node for OgsFx
class SurfaceOgsFx : public SgImplementation
{
public:
    static SgImplementationPtr creator();
    const string& getLanguage() const override;
    const string& getTarget() const override;

    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader, int numArgs = 0, ...) override;

    bool isTransparent(const SgNode& node) const override;
};

} // namespace MaterialX

#endif
