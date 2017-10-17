#ifndef MATERIALX_GLSL_CODEGENERATOR_H
#define MATERIALX_GLSL_CODEGENERATOR_H

#include <MaterialXShaderGen/ShaderGenerator.h>

namespace MaterialX
{

/// Base class for GLSL (OpenGL Shading Language) code generation.
/// A generator for a specific GLSL target should be derived from this class.
class GlslShaderGenerator : public ShaderGenerator
{
public:
    /// Emit code for all texturing nodes.
    virtual void emitTextureNodes(Shader& shader);

    /// Emit code for calculating the BSDF response given the incident and outgoing light directions.
    /// The output bsdf will hold the variable name keeping the result.
    virtual void emitBsdf(const string& incident, const string& outgoing, Shader& shader, string& bsdf);

    /// Emit code for calculating the emission
    /// The output emission will hold the variable keeping the result.
    virtual void emitSurfaceEmission(Shader& shader, string& emission);

protected:
    /// Protected constructor.
    GlslShaderGenerator();
};

} // namespace MaterialX

#endif
