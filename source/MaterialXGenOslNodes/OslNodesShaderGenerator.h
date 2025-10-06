//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_OSLNODESSHADERGENERATOR_H
#define MATERIALX_OSLNODESSHADERGENERATOR_H

/// @file
/// OSL shading language generator

#include <MaterialXGenOslNodes/Export.h>

#include <MaterialXGenShader/ShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

using OslNodesShaderGeneratorPtr = shared_ptr<class OslNodesShaderGenerator>;

/// @class OslNodesShaderGenerator
/// Base class for OSL (Open Shading Language) shader generators.
/// A generator for a specific OSL target should be derived from this class.
class MX_GENOSLNODES_API OslNodesShaderGenerator : public ShaderGenerator
{
  public:
    /// Constructor.
    OslNodesShaderGenerator(TypeSystemPtr typeSystem);

    /// Creator function.
    /// If a TypeSystem is not provided it will be created internally.
    /// Optionally pass in an externally created TypeSystem here,
    /// if you want to keep type descriptions alive after the lifetime
    /// of the shader generator.
    static ShaderGeneratorPtr create(TypeSystemPtr typeSystem = nullptr)
    {
        return std::make_shared<OslNodesShaderGenerator>(typeSystem ? typeSystem : TypeSystem::create());
    }

    /// Return a unique identifier for the target this generator is for
    const string& getTarget() const override { return TARGET; }

    /// Generate a shader starting from the given element, translating
    /// the element and all dependencies upstream into shader code.
    ShaderPtr generate(const string& name, ElementPtr element, GenContext& context) const override;

    /// Unique identifier for this generator target
    static const string TARGET;

  protected:
    /// Create and initialize a new OSL shader for shader generation.
    virtual ShaderPtr createShader(const string& name, ElementPtr element, GenContext& context) const;

};

namespace OSLNodes
{

/// Identifiers for OSL variable blocks
extern MX_GENOSLNODES_API const string UNIFORMS;
extern MX_GENOSLNODES_API const string INPUTS;
extern MX_GENOSLNODES_API const string OUTPUTS;

} // namespace OSL

MATERIALX_NAMESPACE_END

#endif
