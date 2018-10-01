#ifndef MATERIALX_MAYAGLSLPUGINSHADERGENERATOR_H
#define MATERIALX_MAYAGLSLPUGINSHADERGENERATOR_H

#include <MaterialXGenOgsFx/OgsFxShaderGenerator.h>
#include <MaterialXGenShader/HwShader.h>

namespace MaterialX
{

/// An OgsFx shader targeting the glslShader plugin in Maya
/// This shader is identical to the OgsFxShader class except the
/// semantic for u_viewPosition is named differently.
class MayaGlslPluginShader : public OgsFxShader
{
    using ParentClass = OgsFxShader;

public:
    MayaGlslPluginShader(const string& name);

    void createUniform(size_t stage, const string& block, const TypeDesc* type, const string& name, const string& semantic = EMPTY_STRING, ValuePtr value = nullptr) override;
    void createAppData(const TypeDesc* type, const string& name, const string& semantic = EMPTY_STRING) override;
    void createVertexData(const TypeDesc* type, const string& name, const string& semantic = EMPTY_STRING) override;
};

/// An OgsFx shader generator targeting the glslShader plugin in Maya
/// This generator shader is identical to the OgsFxShaderGenerator class except
/// transparency handling is declared differently in the technique section.
class MayaGlslPluginShaderGenerator : public OgsFxShaderGenerator
{
    using ParentClass = OgsFxShaderGenerator;

public:
    static ShaderGeneratorPtr create() { return std::make_shared<MayaGlslPluginShaderGenerator>(); }

    /// Return a unique identifyer for the target this generator is for
    const string& getTarget() const override { return TARGET; }

    /// Unique identifyer for this generator target
    static const string TARGET;

protected:
    /// Create a new shader instance
    OgsFxShaderPtr createShader(const string& name) override;

    /// Get parameters for the technique block
    void getTechniqueParams(const Shader& shader, string& params) override;
};

}

#endif
