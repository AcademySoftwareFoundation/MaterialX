#ifndef MATERIALX_OGSFXSHADERGENERATOR_H
#define MATERIALX_OGSFXSHADERGENERATOR_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenShader/HwShader.h>

namespace MaterialX
{

using OgsFxShaderPtr = shared_ptr<class OgsFxShader>;

/// Shader class targeting the OgsFX file format.
/// Extending HwShader with a new stage holding the final 
/// composited OsgFx shader.
class OgsFxShader : public HwShader
{
    using ParentClass = HwShader;

public:
    /// Identifier for final effects stage
    static const string FINAL_FX_STAGE;

public:
    OgsFxShader(const string& name);

    void createUniform(const string& stage, const string& block, const TypeDesc* type, const string& name, const string& path = EMPTY_STRING, const string& semantic = EMPTY_STRING, ValuePtr value = nullptr) override;
    void createAppData(const TypeDesc* type, const string& name, const string& semantic = EMPTY_STRING) override;
    void createVertexData(const TypeDesc* type, const string& name, const string& semantic = EMPTY_STRING) override;
};

using OgsFxShaderGeneratorPtr = shared_ptr<class OgsFxShaderGenerator>;

/// A GLSL shader generator targeting the OgsFX file format
class OgsFxShaderGenerator : public GlslShaderGenerator
{
    using ParentClass = GlslShaderGenerator;

public:
    OgsFxShaderGenerator();

    static ShaderGeneratorPtr create() { return std::make_shared<OgsFxShaderGenerator>(); }

    /// Return a unique identifyer for the target this generator is for
    const string& getTarget() const override { return TARGET; }

    /// Generate a shader starting from the given element, translating 
    /// the element and all dependencies upstream into shader code.
    ShaderPtr generate(const string& shaderName, ElementPtr element, const GenOptions& options) override;

    /// Unique identifyer for this generator target
    static const string TARGET;

protected:
    /// Emit a shader input variable
    void emitVariable(const Shader::Variable& variable, const string& qualifier, Shader& shader) override;

    /// Create a new shader instance
    virtual OgsFxShaderPtr createShader(const string& name);

    /// Get parameters for the technique block
    virtual void getTechniqueParams(const Shader& shader, string& params);
};

}

#endif
