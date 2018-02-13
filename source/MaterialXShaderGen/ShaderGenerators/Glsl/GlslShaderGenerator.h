#ifndef MATERIALX_GLSLSHADERGENERATOR_H
#define MATERIALX_GLSLSHADERGENERATOR_H

#include <MaterialXShaderGen/ShaderGenerator.h>
#include <MaterialXShaderGen/HwShader.h>

namespace MaterialX
{

/// Base class for GLSL (OpenGL Shading Language) code generation.
/// A generator for a specific GLSL target should be derived from this class.
class GlslShaderGenerator : public ShaderGenerator
{
public:
    enum class BsdfDir
    {
        LIGHT_DIR,
        VIEW_DIR,
        REFL_DIR
    };

public:
    GlslShaderGenerator();

    static ShaderGeneratorPtr creator() { return std::make_shared<GlslShaderGenerator>(); }

    /// Generate a shader starting from the given element, translating 
    /// the element and all dependencies upstream into shader code.
    ShaderPtr generate(const string& shaderName, ElementPtr element) override;

    /// Return a unique identifyer for the language used by this generator
    const string& getLanguage() const override { return LANGUAGE; }

    /// Return a unique identifyer for the target this generator is for
    const string& getTarget() const override { return TARGET; }

    /// Return the version string for the GLSL version this generator is for
    virtual const string& getVersion() const { return VERSION; }

    /// Emit function definitions for all nodes
    void emitFunctionDefinitions(Shader& shader) override;

    /// Emit all functon calls constructing the shader body
    void emitFunctionCalls(Shader &shader) override;

    /// Emit a shader uniform input variable
    void emitUniform(const Shader::Variable& uniform, Shader& shader) override;

    /// Emit the final output expression
    void emitFinalOutput(Shader& shader) const override;

    /// Query the shader generator if it wants to publish a given port as a
    /// shader uniform. Return the publicName to use if it should be published.
    bool shouldPublish(const ValueElement* port, string& publicName) const override;

    /// Return any extra arguments if needed for the given node
    const Arguments* getExtraArguments(const SgNode& node) const override;

    /// Emit code for all texturing nodes.
    virtual void emitTextureNodes(Shader& shader);

    /// Emit code for calculating the BSDF response given the incident and outgoing light directions.
    /// The output bsdf will hold the variable name keeping the result.
    virtual void emitSurfaceBsdf(const SgNode& surfaceShaderNode, BsdfDir wi, BsdfDir wo, Shader& shader, string& bsdf);

    /// Emit code for calculating the emission
    /// The output emission will hold the variable keeping the result.
    virtual void emitSurfaceEmission(const SgNode& surfaceShaderNode, Shader& shader, string& emission);

    /// Unique identifyer for the glsl language
    static const string LANGUAGE;

    /// Unique identifyer for this generator target
    static const string TARGET;

    /// Version string for the generator target
    static const string VERSION;

protected:
    static void toVec4(const string& type, string& variable);

    Arguments _bsdfNodeArguments;
};


/// Base class for common GLSL node implementations
class GlslImplementation : public SgImplementation
{
public:
    const string& getLanguage() const override;
    const string& getTarget() const override;

protected:
    GlslImplementation() {}

    static const string SPACE;
    static const string WORLD;
    static const string OBJECT;
    static const string MODEL;
    static const string INDEX;
};


} // namespace MaterialX

#endif
