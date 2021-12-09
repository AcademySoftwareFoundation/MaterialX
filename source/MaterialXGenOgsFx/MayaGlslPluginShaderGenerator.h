//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_MAYAGLSLPUGINSHADERGENERATOR_H
#define MATERIALX_MAYAGLSLPUGINSHADERGENERATOR_H

/// @file
/// Maya OGSFX shader generator

#include <MaterialXGenOgsFx/OgsFxShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN


/// An OgsFx shader generator targeting the glslShader plugin in Maya
/// This generator shader is identical to the OgsFxShaderGenerator class except
/// transparency handling is declared differently in the technique section.
class MayaGlslPluginShaderGenerator : public OgsFxShaderGenerator
{
public:
    MayaGlslPluginShaderGenerator();

    static ShaderGeneratorPtr create();

    /// Return a unique identifyer for the target this generator is for
    const string& getTarget() const override { return TARGET; }

    /// Unique identifyer for this generator target
    static const string TARGET;

protected:
    /// Get parameters for the technique block
    void getTechniqueParams(const Shader& shader, string& params) const override;
};

MATERIALX_NAMESPACE_END

#endif
