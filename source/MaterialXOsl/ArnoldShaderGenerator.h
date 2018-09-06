#ifndef MATERIALX_ARNOLDSHADERGENERATOR_H
#define MATERIALX_ARNOLDSHADERGENERATOR_H

#include <MaterialXOsl/OslShaderGenerator.h>

namespace MaterialX
{

/// An OSL shader generator targeting the Arnold renderer
class ArnoldShaderGenerator : public OslShaderGenerator
{
public:
    ArnoldShaderGenerator() : OslShaderGenerator() {}

    static ShaderGeneratorPtr create() { return std::make_shared<ArnoldShaderGenerator>(); }

    /// Return a unique identifyer for the target this generator is for
    const string& getTarget() const override { return TARGET; }

    /// Return the v-direction used by the target system
    Shader::VDirection getTargetVDirection() const override;

    /// Unique identifyer for this generator target
    static const string TARGET;
};

}

#endif
