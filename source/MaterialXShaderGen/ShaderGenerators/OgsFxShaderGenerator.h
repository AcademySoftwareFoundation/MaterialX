#ifndef MATERIALX_OGSFX_CODEGENERATOR_H
#define MATERIALX_OGSFX_CODEGENERATOR_H

#include <MaterialXShaderGen/ShaderGenerators/GlslShaderGenerator.h>
#include <MaterialXShaderGen/HwShader.h>

namespace MaterialX
{

using OgsFxShaderPtr = shared_ptr<class OgsFxShader>;

/// Shader class extending HwShader with a new stage 
/// holding the final composited OsgFx shader.
class OgsFxShader : public HwShader
{
public:
    /// Identifier for final effects stage
    static const size_t FINAL_FX_STAGE = HwShader::NUM_STAGES;
    static const size_t NUM_STAGES = HwShader::NUM_STAGES + 1;

public:
    OgsFxShader(const string& name) : HwShader(name) {}

    /// Return the number of shader stages for this shader.
    virtual size_t numStages() const { return NUM_STAGES; }
};

/// A GLSL shader generator targeting the OgsFX file format
class OgsFxShaderGenerator : public GlslShaderGenerator
{
public:
    OgsFxShaderGenerator();

    static ShaderGeneratorPtr creator() { return std::make_shared<OgsFxShaderGenerator>(); }

    /// Return a unique identifyer for the target this generator is for
    const string& getTarget() const override { return TARGET; }

    /// Return the v-direction used by the target system
    Shader::VDirection getTargetVDirection() const override { return Shader::VDirection::DOWN; }

    /// Generate a shader starting from the given element, translating 
    /// the element and all dependencies upstream into shader code.
    ShaderPtr generate(const string& shaderName, ElementPtr element) override;

    /// Emit the final output expression
    void emitFinalOutput(Shader& shader) const override;

    /// Unique identifyer for this generator target
    static const string TARGET;
};

}

#endif
