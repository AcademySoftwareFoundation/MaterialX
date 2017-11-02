#ifndef MATERIALX_SURFACE_H
#define MATERIALX_SURFACE_H

#include <MaterialXShaderGen/NodeImplementation.h>

namespace MaterialX
{

/// Implementation of 'surface' node for OgsFx
class SurfaceOgsFx : public NodeImplementation
{
public:
    static NodeImplementationPtr creator();
    const string& getLanguage() const override;
    const string& getTarget() const override;

    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader, int numArgs = 0, ...) override;
};

} // namespace MaterialX

#endif
