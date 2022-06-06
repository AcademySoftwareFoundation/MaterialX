//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_OSLSHADERGENERATOR_H
#define MATERIALX_OSLSHADERGENERATOR_H

/// @file
/// OSL shading language generator

#include <MaterialXGenOsl/Export.h>

#include <MaterialXGenShader/ShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

using OslShaderGeneratorPtr = shared_ptr<class OslShaderGenerator>;

/// @class OslShaderGenerator
/// Base class for OSL (Open Shading Language) shader generators.
/// A generator for a specific OSL target should be derived from this class.
class MX_GENOSL_API OslShaderGenerator : public ShaderGenerator
{
  public:
    OslShaderGenerator();

    static ShaderGeneratorPtr create() { return std::make_shared<OslShaderGenerator>(); }

    /// Return a unique identifier for the target this generator is for
    const string& getTarget() const override { return TARGET; }

    /// Generate a shader starting from the given element, translating
    /// the element and all dependencies upstream into shader code.
    ShaderPtr generate(const string& name, ElementPtr element, GenContext& context) const override;

    /// Add all function calls for a graph. If a classification mask is given only functions for
    /// nodes matching this classification will be emitted.
    void emitFunctionCalls(const ShaderGraph& graph, GenContext& context, ShaderStage& stage, uint32_t classification = 0u) const override;

    /// Emit code for starting a new function body.
    void emitFunctionBodyBegin(const ShaderNode& node, GenContext& context, ShaderStage& stage, Syntax::Punctuation punc = Syntax::CURLY_BRACKETS) const override;

    /// Unique identifier for this generator target
    static const string TARGET;

    /// Register metadata that should be exported to the generated shaders.
    void registerShaderMetadata(const DocumentPtr& doc, GenContext& context) const override;

protected:
    // Extra file arguments for texture lookup call
    static const string T_FILE_EXTRA_ARGUMENTS;

    /// Create and initialize a new OSL shader for shader generation.
    virtual ShaderPtr createShader(const string& name, ElementPtr element, GenContext& context) const;

    /// Emit include headers needed by the generated shader code.
    virtual void emitLibraryIncludes(ShaderStage& stage, GenContext& context) const;

    /// Emit a block of shader inputs.
    virtual void emitShaderInputs(const VariableBlock& inputs, ShaderStage& stage) const;

    /// Emit a block of shader outputs.
    virtual void emitShaderOutputs(const VariableBlock& inputs, ShaderStage& stage) const;
};

namespace OSL
{
    /// Identifiers for OSL variable blocks
    extern MX_GENOSL_API const string UNIFORMS;
    extern MX_GENOSL_API const string INPUTS;
    extern MX_GENOSL_API const string OUTPUTS;
}

MATERIALX_NAMESPACE_END

#endif
