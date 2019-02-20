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
    ShaderPtr generate(const string& name, ElementPtr element, GenContext& context) override;

    /// Add all function calls for a graph.
    void emitFunctionCalls(ShaderStage& stage, const ShaderGraph& graph, GenContext& context) override;

    /// Unique identifyer for the osl language
    static const string LANGUAGE;

protected:
    /// Protected constructor.
    OslShaderGenerator();

    /// Create and initialize a new OSL shader for shader generation.
    virtual ShaderPtr createShader(const string& name, ElementPtr element, GenContext& context);

    /// Emit include headers needed by the generated shader code.
    virtual void emitIncludes(ShaderStage& stage);
};

namespace OSL
{
    /// Identifiers for OSL stage and variable blocks
    extern const string STAGE;
    extern const string UNIFORMS;
    extern const string INPUTS;
    extern const string OUTPUTS;
}

} // namespace MaterialX

#endif
