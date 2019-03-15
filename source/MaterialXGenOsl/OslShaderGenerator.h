//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_OSLSHADERGENERATOR_H
#define MATERIALX_OSLSHADERGENERATOR_H

/// @file
/// OSL shading language generator

#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

using OslShaderGeneratorPtr = shared_ptr<class OslShaderGenerator>;

/// @class OslShaderGenerator
/// Base class for OSL (Open Shading Language) shader generators.
/// A generator for a specific OSL target should be derived from this class.
class OslShaderGenerator : public ShaderGenerator
{
  public:
    OslShaderGenerator();

    static ShaderGeneratorPtr create() { return std::make_shared<OslShaderGenerator>(); }

    /// Return a unique identifyer for the language used by this generator
    const string& getLanguage() const override { return LANGUAGE; }

    /// Return a unique identifyer for the target this generator is for
    const string& getTarget() const override { return TARGET; }

    /// Generate a shader starting from the given element, translating
    /// the element and all dependencies upstream into shader code.
    ShaderPtr generate(const string& name, ElementPtr element, GenContext& context) const override;

    /// Add all function calls for a graph.
    void emitFunctionCalls(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const override;

    /// Unique identifyer for the osl language
    static const string LANGUAGE;

    /// Unique identifyer for this generator target
    static const string TARGET;

protected:

    /// Create and initialize a new OSL shader for shader generation.
    virtual ShaderPtr createShader(const string& name, ElementPtr element, GenContext& context) const;

    /// Emit include headers needed by the generated shader code.
    virtual void emitIncludes(ShaderStage& stage, GenContext& context) const;
};

namespace OSL
{
    /// Identifiers for OSL variable blocks
    extern const string UNIFORMS;
    extern const string INPUTS;
    extern const string OUTPUTS;
}

} // namespace MaterialX

#endif
