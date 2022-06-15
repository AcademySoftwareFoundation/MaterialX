//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_VKSHADERGENERATOR_H
#define MATERIALX_VKSHADERGENERATOR_H

/// @file
/// ESSL shader generator

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenGlsl/VkResourceBindingContext.h>

MATERIALX_NAMESPACE_BEGIN

using VkShaderGeneratorPtr = shared_ptr<class VkShaderGenerator>;

/// @class VkShaderGenerator 
/// A Vulkan GLSL shader generator 
class MX_GENGLSL_API VkShaderGenerator : public GlslShaderGenerator
{
  public:
    VkShaderGenerator();

    static ShaderGeneratorPtr create() { return std::make_shared<VkShaderGenerator>(); }

    /// Return a unique identifier for the target this generator is for
    const string& getTarget() const override { return TARGET; }

    /// Return the version string for the ESSL version this generator is for
    const string& getVersion() const override { return VERSION; }

    string getVertexDataPrefix(const VariableBlock& vertexData) const override;

    /// Unique identifier for this generator target
    static const string TARGET;
    static const string VERSION;

    // Emit directives for stage
    void emitDirectives(GenContext& context, ShaderStage& stage) const override;
    //void emitVariableDeclarations(const VariableBlock& block, const string& qualifier, const string& separator,
    //                              GenContext& context, ShaderStage& stage,
    //                              bool assignValue) const override;

    void emitInputs(GenContext& context, ShaderStage& stage) const override;
    
    void emitOutputs(GenContext& context, ShaderStage& stage) const override;
    
    /*
    // Emit uniforms with binding information
    void emitResourceBindings(GenContext& context, const VariableBlock& uniforms, ShaderStage& stage) const;

    // Emit structured uniforms with binding information and align members where possible
    void emitStructuredResourceBindings(GenContext& context, const VariableBlock& uniforms,
                                        ShaderStage& stage, const std::string& structInstanceName,
                                        const std::string& arraySuffix);

    // Emit separate binding locations for sampler and uniform table
    void enableSeparateBindingLocations(bool separateBindingLocation) { _separateBindingLocation = separateBindingLocation; };
    */

  protected:

    
    HwResourceBindingContextPtr getResourceBindingContext(GenContext& ) const override;

    // Separate binding locations flag
    // Indicates whether to use a shared binding counter for samplers and uniforms or separate ones.
    // By default a shader counter is used.
    bool _separateBindingLocation = false;

    VkResourceBindingContextPtr _resourceBindingCtx = nullptr;
};

MATERIALX_NAMESPACE_END

#endif
