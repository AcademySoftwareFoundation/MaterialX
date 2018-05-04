#ifndef MATERIALX_OGSFXSYNTAX_H
#define MATERIALX_OGSFXSYNTAX_H

#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslSyntax.h>

namespace MaterialX
{

/// Syntax class for OgsFx
class OgsFxSyntax : public GlslSyntax
{
    using ParentClass = GlslSyntax;

public:
    OgsFxSyntax();

    static SyntaxPtr create() { return std::make_shared<OgsFxSyntax>(); }

    void makeUnique(string& name, const string& /*type*/, UniqueNameMap& uniqueNames) const override;
};

} // namespace MaterialX

#endif
