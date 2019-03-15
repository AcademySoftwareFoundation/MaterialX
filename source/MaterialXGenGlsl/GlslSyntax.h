//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_GLSL_SYNTAX_H
#define MATERIALX_GLSL_SYNTAX_H

/// @file
/// GLSL syntax class

#include <MaterialXGenShader/Syntax.h>

namespace MaterialX
{

/// Syntax class for GLSL (OpenGL Shading Language)
class GlslSyntax : public Syntax
{
  public:
    GlslSyntax();

    static SyntaxPtr create() { return std::make_shared<GlslSyntax>(); }

    const string& getInputQualifier() const override { return INPUT_QUALIFIER; }
    const string& getOutputQualifier() const override { return OUTPUT_QUALIFIER; }
    const string& getConstantQualifier() const override { return CONSTANT_QUALIFIER; };
    const string& getUniformQualifier() const override { return UNIFORM_QUALIFIER; };

    bool typeSupported(const TypeDesc* type) const override;

    static const string INPUT_QUALIFIER;
    static const string OUTPUT_QUALIFIER;
    static const string UNIFORM_QUALIFIER;
    static const string CONSTANT_QUALIFIER;

    static const StringVec VEC2_MEMBERS;
    static const StringVec VEC3_MEMBERS;
    static const StringVec VEC4_MEMBERS;
};

} // namespace MaterialX

#endif
