#ifndef MATERIALX_OGSFXSHADERGENERATOR_H
#define MATERIALX_OGSFXSHADERGENERATOR_H

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
    ShaderPtr generate(const string& shaderName, ElementPtr element, GenContext& context) const override;

    /// Unique identifyer for this generator target
    static const string TARGET;

protected:
    void emitVertexStage(ShaderStage& stage, const ShaderGraph& graph, GenContext& context) const override;
    void emitPixelStage(ShaderStage& stage, const ShaderGraph& graph, GenContext& context) const override;

    /// Emit a shader input variable
    void emitVariableDeclaration(ShaderStage& stage, const ShaderPort* variable,
                                 const string& qualifier, bool assignValue = true) const override;

    /// Create and initialize a new OGSFX shader for shader generation.
    ShaderPtr createShader(const string& name, ElementPtr element, GenContext& context) const override;

    /// Get parameters for the technique block
    virtual void getTechniqueParams(const Shader& shader, string& params) const;
};

namespace HW
{
    extern const string FX_STAGE;
}

}

#endif
