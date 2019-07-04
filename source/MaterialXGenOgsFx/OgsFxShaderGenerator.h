//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_OGSFXSHADERGENERATOR_H
#define MATERIALX_OGSFXSHADERGENERATOR_H

/// @file
/// OGSFX shader generator

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

using OgsFxShaderGeneratorPtr = shared_ptr<class OgsFxShaderGenerator>;

/// A GLSL shader generator targeting the OgsFX file format
class OgsFxShaderGenerator : public GlslShaderGenerator
{
public:
    OgsFxShaderGenerator();

    static ShaderGeneratorPtr create();

    /// Return a unique identifyer for the target this generator is for
    const string& getTarget() const override { return TARGET; }

    /// Generate a shader starting from the given element, translating
    /// the element and all dependencies upstream into shader code.
    ShaderPtr generate(const string& name, ElementPtr element, GenContext& context) const override;

    /// Unique identifyer for this generator target
    static const string TARGET;

protected:
    void emitVertexStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const override;
    void emitPixelStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const override;

    /// Emit a shader input variable
    void emitVariableDeclaration(const ShaderPort* variable, const string& qualifier,
                                 GenContext& context, ShaderStage& stage,
                                 bool assignValue = true) const override;

    /// Create and initialize a new OGSFX shader for shader generation.
    ShaderPtr createShader(const string& name, ElementPtr element, GenContext& context) const override;

    /// Get parameters for the technique block
    virtual void getTechniqueParams(const Shader& shader, string& params) const;

    StringMap _semanticsMap;
};

namespace Stage
{
    /// Identifier for full Effect
    extern const string EFFECT;
}

}

#endif
