#ifndef MATERIALX_OSLSYNTAX_H
#define MATERIALX_OSLSYNTAX_H

#include <MaterialXShaderGen/Syntax.h>

namespace MaterialX
{

/// Syntax class for OSL (Open Shading Language)
class OslSyntax : public Syntax
{
public:
    OslSyntax();

    static SyntaxPtr create() { return std::make_shared<OslSyntax>(); }
};

} // namespace MaterialX

#endif
