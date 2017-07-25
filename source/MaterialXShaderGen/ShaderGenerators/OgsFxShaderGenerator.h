#ifndef MATERIALX_OGSFX_CODEGENERATOR_H
#define MATERIALX_OGSFX_CODEGENERATOR_H

#include <MaterialXShaderGen/ShaderGenerators/GlslShaderGenerator.h>
#include <MaterialXShaderGen/Shader.h>

namespace MaterialX
{

/// A GLSL shader generator targeting the OgsFX file format
class OgsFxShaderGenerator : public GlslShaderGenerator
{
    DECLARE_SHADER_GENERATOR(OgsFxShaderGenerator)
public:
    OgsFxShaderGenerator() : GlslShaderGenerator() {}

    /// Return the v-direction used by the target system
    Shader::VDirection getTargetVDirection() const override { return Shader::VDirection::DOWN; }

    /// Generate a shader starting from the given element, translating 
    /// the element and all dependencies upstream into shader code.
    ShaderPtr generate(const string& shaderName, ElementPtr element) override;

    /// Emit the final output expression
    void emitFinalOutput(Shader& shader) const override;

    /// Emit a shader uniform input variable
    void emitUniform(const string& name, const string& type, const ValuePtr& value, Shader& shader) override;

    /// Emit the connected variable name for an input port
    /// or constant value if the port is not connected
    void emitInput(const ValueElement& port, Shader& shader) override;

protected:
    void addExtraShaderUniforms(Shader& shader);
    bool useAsShaderUniform(const Parameter& param) const;
};

}

#endif
