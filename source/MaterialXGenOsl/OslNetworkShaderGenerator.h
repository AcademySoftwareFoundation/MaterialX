//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_OSLNETWORKSHADERGENERATOR_H
#define MATERIALX_OSLNETWORKSHADERGENERATOR_H

/// @file
/// OSL shading language generator

#include <MaterialXGenOsl/Export.h>

#include <MaterialXGenOsl/OslShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

using OslNetworkShaderGeneratorPtr = shared_ptr<class OslNetworkShaderGenerator>;

/// @class OslNetworkShaderGenerator
/// OSL (Open Shading Language) Network shader generator.
/// Generates a command string that OSL can use to build a ShaderGroup.
class MX_GENOSL_API OslNetworkShaderGenerator : public OslShaderGenerator
{
  public:
    /// Constructor.
    OslNetworkShaderGenerator(TypeSystemPtr typeSystem);

    /// Creator function.
    /// If a TypeSystem is not provided it will be created internally.
    /// Optionally pass in an externally created TypeSystem here,
    /// if you want to keep type descriptions alive after the lifetime
    /// of the shader generator.
    static ShaderGeneratorPtr create(TypeSystemPtr typeSystem = nullptr)
    {
        return std::make_shared<OslNetworkShaderGenerator>(typeSystem ? typeSystem : TypeSystem::create());
    }

    ShaderNodeImplPtr createShaderNodeImplForImplementation(const Implementation& implElement) const override;

    /// Return a unique identifier for the target this generator is for
    const string& getTarget() const override { return TARGET; }

    /// Generate a shader starting from the given element, translating
    /// the element and all dependencies upstream into shader code.
    ShaderPtr generate(const string& name, ElementPtr element, GenContext& context) const override;

    /// Unique identifier for this generator target
    static const string TARGET;

  protected:
    /// Create and initialize a new OSL shader for shader generation.
    ShaderPtr createShader(const string& name, ElementPtr element, GenContext& context) const override;
};

namespace OSLNetwork
{

/// Identifiers for OSL variable blocks
extern MX_GENOSL_API const string UNIFORMS;
extern MX_GENOSL_API const string INPUTS;
extern MX_GENOSL_API const string OUTPUTS;

} // namespace OSLNetwork

MATERIALX_NAMESPACE_END

#endif
