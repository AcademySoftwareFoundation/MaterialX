//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_ESSLSYNTAX_H
#define MATERIALX_ESSLSYNTAX_H

/// @file
/// ESSL syntax class

#include <MaterialXGenGlsl/GlslSyntax.h>

namespace MaterialX
{

/// Syntax class for ESSL (OpenGL ES Shading Language)
class EsslSyntax : public GlslSyntax
{
public:
    EsslSyntax();

    static SyntaxPtr create() { return std::make_shared<EsslSyntax>(); }
};

} // namespace MaterialX

#endif
