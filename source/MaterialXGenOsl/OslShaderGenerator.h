#ifndef MATERIALX_OSLSHADERGENERATOR_H
#define MATERIALX_OSLSHADERGENERATOR_H

#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

using OslShaderGeneratorPtr = shared_ptr<class OslShaderGenerator>;

/// Base class for OSL (Open Shading Language) shader generators.
/// A generator for a specific OSL target should be derived from this class.
class OslShaderGenerator : public ShaderGenerator
{
    using ParentClass = ShaderGenerator;

public:
    /// Return a unique identifyer for the language used by this generator
    const string& getLanguage() const override { return LANGUAGE; }

    /// Generate a shader starting from the given element, translating
    /// the element and all dependencies upstream into shader code.
    ShaderPtr generate(const string& shaderName, ElementPtr element, const GenOptions& options) override;

    /// Emit the shader body
    void emitFunctionCalls(const GenContext& context, Shader& shader) override;

    /// Emit the final output expression
    void emitFinalOutput(Shader& shader) const override;

    /// Unique identifyer for the osl language
    static const string LANGUAGE;

protected:
    /// Protected constructor.
    OslShaderGenerator();

    /// Emit include headers needed by the generated shader code.
    void emitIncludes(Shader& shader);

    /// Emit a shader input variable
    void emitVariable(const Shader::Variable& variable, const string& qualifier, Shader& shader) override;
};

} // namespace MaterialX

#endif
