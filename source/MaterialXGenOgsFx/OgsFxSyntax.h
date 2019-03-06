//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_OGSFXSYNTAX_H
#define MATERIALX_OGSFXSYNTAX_H

#include <MaterialXGenGlsl/GlslSyntax.h>

namespace MaterialX
{

/// Syntax class for OgsFx
class OgsFxSyntax : public GlslSyntax
{
    using ParentClass = GlslSyntax;

public:
    OgsFxSyntax();

    static SyntaxPtr create() { return std::make_shared<OgsFxSyntax>(); }
};

} // namespace MaterialX

#endif
