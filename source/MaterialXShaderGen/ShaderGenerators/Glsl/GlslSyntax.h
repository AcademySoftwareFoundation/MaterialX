#ifndef MATERIALX_GLSL_SYNTAX_H
#define MATERIALX_GLSL_SYNTAX_H

#include <MaterialXShaderGen/Syntax.h>

namespace MaterialX
{

/// Syntax class for GLSL (OpenGL Shading Language)
class GlslSyntax : public Syntax
{
public:
    GlslSyntax();

    string getValue(const Value& value, bool paramInit = false) const override;
};

} // namespace MaterialX

#endif
