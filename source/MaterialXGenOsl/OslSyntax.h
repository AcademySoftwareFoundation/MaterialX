//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_OSLSYNTAX_H
#define MATERIALX_OSLSYNTAX_H

/// @file
/// OSL syntax class

#include <MaterialXGenShader/Syntax.h>

namespace MaterialX
{

/// @class OslSyntax
/// Syntax class for OSL (Open Shading Language)
class OslSyntax : public Syntax
{
public:
    OslSyntax();

    static SyntaxPtr create() { return std::make_shared<OslSyntax>(); }

    const string& getOutputQualifier() const override;
    const string& getConstantQualifier() const override { return EMPTY_STRING; };

    static const string OUTPUT_QUALIFIER;
    static const StringVec VECTOR_MEMBERS;
    static const StringVec VECTOR2_MEMBERS;
    static const StringVec VECTOR4_MEMBERS;
    static const StringVec COLOR2_MEMBERS;
    static const StringVec COLOR4_MEMBERS;
};

} // namespace MaterialX

#endif
