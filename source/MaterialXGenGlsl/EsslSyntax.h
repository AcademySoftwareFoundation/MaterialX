//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_ESSLSYNTAX_H
#define MATERIALX_ESSLSYNTAX_H

/// @file
/// ESSL syntax class

#include <MaterialXGenGlsl/GlslSyntax.h>

MATERIALX_NAMESPACE_BEGIN

/// Syntax class for ESSL (OpenGL ES Shading Language)
class MX_GENGLSL_API EsslSyntax : public GlslSyntax
{
public:
    EsslSyntax();

    static SyntaxPtr create() { return std::make_shared<EsslSyntax>(); }
};

MATERIALX_NAMESPACE_END

#endif
