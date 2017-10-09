#ifndef MATERIALX_GLSL_CODEGENERATOR_H
#define MATERIALX_GLSL_CODEGENERATOR_H

#include <MaterialXShaderGen/ShaderGenerator.h>

namespace MaterialX
{

/// Base class for GLSL (OpenGL Shading Language) code generation
/// A generator for a specific GLSL target should be derived from this class
class GlslShaderGenerator : public ShaderGenerator
{
public:
    /// Emit code for calculating all input values to closures
    virtual void emitClosureInputs(Shader& shader);

    /// Emit code for calculating the BSDF response given the incident and outgoing light directions
    virtual void emitBsdf(const string& incident, const string& outgoing, Shader& shader, string& bsdf);

    /// Emit 
    virtual void emitSurfaceEmissionAndOpacity(Shader& shader, string& emission, string& opacity);

protected:
    /// Protected constructor.
    GlslShaderGenerator();
};

} // namespace MaterialX

#endif
