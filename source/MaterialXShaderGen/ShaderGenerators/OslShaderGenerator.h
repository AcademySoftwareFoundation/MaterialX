#ifndef MATERIALX_OSLSHADERGENERATOR_H
#define MATERIALX_OSLSHADERGENERATOR_H

#include <MaterialXShaderGen/ShaderGenerator.h>

namespace MaterialX
{

/// Base class for OSL (Open Shading Language) shader generators.
/// A generator for a specific OSL target should be derived from this class.
class OslShaderGenerator : public ShaderGenerator
{
public:
    /// Generate the shader for a graph starting from the given output port.
    ShaderPtr generate(NodePtr node, OutputPtr connectingElement = nullptr) override;

    /// Emit the shader body
    void emitShaderBody(Shader& shader) override;

protected:
    /// Protected constructor.
    OslShaderGenerator();

    /// Emit the shader signature with inputs and outputs
    void emitShaderSignature(Shader &shader);
};

} // namespace MaterialX

#endif
