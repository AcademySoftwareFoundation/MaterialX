#ifndef MATERIALX_MAYAGLSLPUGINSHADERGENERATOR_H
#define MATERIALX_MAYAGLSLPUGINSHADERGENERATOR_H

#include <MaterialXGenOgsFx/OgsFxShaderGenerator.h>

namespace MaterialX
{

/// An OgsFx shader generator targeting the glslShader plugin in Maya
/// This generator shader is identical to the OgsFxShaderGenerator class except
/// transparency handling is declared differently in the technique section.
class MayaGlslPluginShaderGenerator : public OgsFxShaderGenerator
{
public:
    static ShaderGeneratorPtr create();

    /// Return a unique identifyer for the target this generator is for
    const string& getTarget() const override { return TARGET; }

    /// Unique identifyer for this generator target
    static const string TARGET;

protected:
    /// Create a new shader instance
    ShaderPtr createShader(const string& name, ElementPtr element, GenContext& context) const override;

    /// Get parameters for the technique block
    void getTechniqueParams(const Shader& shader, string& params) const override;
};

}

#endif
