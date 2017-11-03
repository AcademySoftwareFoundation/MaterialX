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
    /// Generate a shader starting from the given element, translating 
    /// the element and all dependencies upstream into shader code.
    ShaderPtr generate(const string& shaderName, ElementPtr element) override;

    /// Emit function definitions for all nodes
    void emitFunctions(Shader& shader) override;

    /// Emit the shader body
    void emitShaderBody(Shader& shader) override;

protected:
    /// Protected constructor.
    OslShaderGenerator();

    /// Emit include headers needed by the generated shader code.
    void emitIncludes(Shader& shader);

    /// Emit the shader signature with inputs and outputs.
    void emitShaderSignature(Shader &shader);
};

} // namespace MaterialX

#endif
