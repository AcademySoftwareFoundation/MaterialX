//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_ESSLSYNTAX_H
#define MATERIALX_ESSLSYNTAX_H

/// @file
/// Vulkan GLSL syntax class

#include <MaterialXGenGlsl/GlslSyntax.h>

MATERIALX_NAMESPACE_BEGIN

/// Syntax class for ESSL (OpenGL ES Shading Language)
class MX_GENGLSL_API VkSyntax : public GlslSyntax
{
public:
    VkSyntax();

    static SyntaxPtr create() { return std::make_shared<VkSyntax>(); }

    const string& getInputQualifier() const override { return INPUT_QUALIFIER; }


};

MATERIALX_NAMESPACE_END

#endif
