//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_MDLSHADERGENERATOR_H
#define MATERIALX_MDLSHADERGENERATOR_H

/// @file
/// MDL shading language generator

#include <MaterialXGenMdl/Export.h>

#include <MaterialXGenShader/ShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

/// Shared pointer to an MdlShaderGenerator
using MdlShaderGeneratorPtr = shared_ptr<class MdlShaderGenerator>;

/// @class MdlShaderGenerator
/// Shader generator for MDL (Material Definition Language).
class MX_GENMDL_API MdlShaderGenerator : public ShaderGenerator
{
  public:
    MdlShaderGenerator();

    static ShaderGeneratorPtr create() { return std::make_shared<MdlShaderGenerator>(); }

    /// Return a unique identifier for the target this generator is for
    const string& getTarget() const override { return TARGET; }

    /// Generate a shader starting from the given element, translating
    /// the element and all dependencies upstream into shader code.
    ShaderPtr generate(const string& name, ElementPtr element, GenContext& context) const override;

    /// Return a registered shader node implementation given an implementation element.
    /// The element must be an Implementation or a NodeGraph acting as implementation.
    ShaderNodeImplPtr getImplementation(const NodeDef& nodedef, GenContext& context) const override;

    /// Return the result of an upstream connection or value for an input.
    string getUpstreamResult(const ShaderInput* input, GenContext& context) const override;

    /// Unique identifier for this generator target
    static const string TARGET;

  protected:
    // Create and initialize a new MDL shader for shader generation.
    ShaderPtr createShader(const string& name, ElementPtr element, GenContext& context) const;

    // Emit a block of shader inputs.
    void emitShaderInputs(const VariableBlock& inputs, ShaderStage& stage) const;
};

namespace MDL
{
    // Identifiers for MDL variable blocks
    extern MX_GENMDL_API const string INPUTS;
    extern MX_GENMDL_API const string OUTPUTS;
}

MATERIALX_NAMESPACE_END

#endif
