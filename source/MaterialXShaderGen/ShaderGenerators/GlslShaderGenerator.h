#ifndef MATERIALX_GLSL_CODEGENERATOR_H
#define MATERIALX_GLSL_CODEGENERATOR_H

#include <MaterialXShaderGen/ShaderGenerator.h>

namespace MaterialX
{

/// Base class for GLSL (OpenGL Shading Language) code generation
/// A generator for a specific GLSL target should be derived from this class
class GlslShaderGenerator : public ShaderGenerator
{
protected:
    /// Protected constructor.
    GlslShaderGenerator();
};

} // namespace MaterialX

#endif
