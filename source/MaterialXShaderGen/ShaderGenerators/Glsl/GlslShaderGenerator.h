#ifndef MATERIALX_GLSLSHADERGENERATOR_H
#define MATERIALX_GLSLSHADERGENERATOR_H

#include <MaterialXShaderGen/ShaderGenerator.h>

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
    /// Return a unique identifyer for the language used by this generator
    const string& getLanguage() const override { return LANGUAGE; }

    /// Emit function definitions for all nodes
    void emitFunctionDefinitions(Shader& shader) override;

    /// Emit all functon calls constructing the shader body
    void emitFunctionCalls(Shader &shader) override;

    /// Emit a shader uniform input variable
    void emitUniform(const Shader::Variable& uniform, Shader& shader) override;

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

protected:
    /// Protected constructor.
    GlslShaderGenerator();

    static void toVec4(const string& type, string& variable);

    Arguments _bsdfNodeArguments;
};

} // namespace MaterialX

#endif
