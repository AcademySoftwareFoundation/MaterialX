//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_OGSFXSYNTAX_H
#define MATERIALX_OGSFXSYNTAX_H

/// @file
/// OGSFX syntax class

#include <MaterialXGenGlsl/GlslSyntax.h>

MATERIALX_NAMESPACE_BEGIN


/// Syntax class for OgsFx
class OgsFxSyntax : public GlslSyntax
{
    using ParentClass = GlslSyntax;

public:
    OgsFxSyntax();

    static SyntaxPtr create() { return std::make_shared<OgsFxSyntax>(); }
};

MATERIALX_NAMESPACE_END

#endif
